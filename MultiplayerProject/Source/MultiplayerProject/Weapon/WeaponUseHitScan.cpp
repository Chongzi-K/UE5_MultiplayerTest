// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponUseHitScan.h"
#include "Engine/SkeletalMeshSocket.h"
#include "MultiplayerProject/MainCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"


void AWeaponUseHitScan::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

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
		FVector ScaleEndPoint = ScaleStartPoint + (HitTarget - ScaleStartPoint) * 1.25f;//向前伸出一段，防止因为Mesh的原因无法返回结果

		FHitResult FireHit;
		UWorld* World = GetWorld();
		if (World)
		{
			World->LineTraceSingleByChannel(
				FireHit,
				ScaleStartPoint,
				ScaleEndPoint,
				ECollisionChannel::ECC_Visibility
			);

			FVector BeamEnd = ScaleEndPoint;//预先设置子弹尾迹终点为射箭检测终点

			//命中玩家
			if (FireHit.bBlockingHit)
			{
				BeamEnd = FireHit.ImpactPoint;//如果检测成功，则子弹尾迹终点为检测点
				AMainCharacter* MainCharacter = Cast<AMainCharacter>(FireHit.GetActor());
				if (MainCharacter && HasAuthority() &&InstigatorController)
				{
					UGameplayStatics::ApplyDamage(
						MainCharacter,
						Damage,
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}
				if (ImpactParticleSystem)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						World,
						ImpactParticleSystem,
						FireHit.ImpactPoint,
						FireHit.ImpactNormal.Rotation()
					);
				}
			}

			//绘制子弹尾迹
			if (BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
					World,
					BeamParticles,
					MuzzleFlashSocketFransform
				);
				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}

			//播放命中声音
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
			}
		}

		//枪口闪光
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(World, MuzzleFlash, MuzzleFlashSocketFransform);
		}
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}

	}
}