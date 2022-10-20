// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotGun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "MultiplayerProject/MainCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

void AShotGun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) { return; }//获取不到所有者时直接无效

	AController* InstigatorController = OwnerPawn->GetController();
	//不用 Cast ，节省性能
	//Controller 在 Simulate 实例 上都为 nullptr


	const USkeletalMeshSocket* MuzzleFlashScoket = GetWeaponMesh()->GetSocketByName("MuzzleFlashSocket");
	if (MuzzleFlashScoket)
	{
		FTransform MuzzleFlashSocketFransform = MuzzleFlashScoket->GetSocketTransform(GetWeaponMesh());
		FVector ScaleStartPoint = MuzzleFlashSocketFransform.GetLocation();
		
		//uint32 Hits = 0;//用于计数命中了几次 //直接++处理
		TMap<AMainCharacter*, uint32>HitMap;//被命中玩家-命中的子弹个数

		//循环处理，每一发子弹都单独计算
		for (uint32 i = 0; i < NumOfProjectilesPerShot; i++)
		{
			//FVector ScaleEndPoint = TraceEndWithScatter(ScaleStartPoint, HitTarget);
			FHitResult FireHit;
			WeaponTraceHit(ScaleStartPoint, HitTarget, FireHit);

			//BeamEnd = FireHit.ImpactPoint;//如果检测成功，则子弹尾迹终点为检测点
			AMainCharacter* MainCharacter = Cast<AMainCharacter>(FireHit.GetActor());

			//命中后在服务器上更新命中图
			if (MainCharacter && HasAuthority() && InstigatorController)
			{
				if (HitMap.Contains(MainCharacter))
				{
					HitMap[MainCharacter]++;
				}
				else
				{
					HitMap.Emplace(MainCharacter, 1);
				}
			}

			//在子类中绘制粒子和声音，而不是在父类中，是因为父类会统一处理所有，而散射子弹到命中点的时间有早有晚
			// 让每个子弹都可以得到单独的粒子和声音绘制
			//绘制命中粒子
			if (ImpactParticleSystem)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticleSystem,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}
			//播放子弹声音
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint, 0.5f, FMath::FRandRange(-0.5f, 0.5f));
			}
		}

		//循环处理，为在命中图内的玩家申请伤害
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key && HasAuthority() && InstigatorController)
			{
				UGameplayStatics::ApplyDamage(
					HitPair.Key,
					Damage * HitPair.Value,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
		}
	}
}

