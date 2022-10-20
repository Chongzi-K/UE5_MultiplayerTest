// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponUseHitScan.h"
#include "Engine/SkeletalMeshSocket.h"
#include "MultiplayerProject/MainCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "WeaponTypes.h"

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
		//FVector ScaleEndPoint = ScaleStartPoint + (HitTarget - ScaleStartPoint) * 1.25;

		FHitResult FireHit;
		WeaponTraceHit(ScaleStartPoint, HitTarget, FireHit);//负责处理发射子弹+返回命中

		//命中玩家
		if (FireHit.bBlockingHit)
		{
			//BeamEnd = FireHit.ImpactPoint;//如果检测成功，则子弹尾迹终点为检测点
			AMainCharacter* MainCharacter = Cast<AMainCharacter>(FireHit.GetActor());

			//命中后在服务器执行申请伤害
			if (MainCharacter && HasAuthority() && InstigatorController)
			{
				UGameplayStatics::ApplyDamage(
					MainCharacter,
					Damage,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}

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
		}

//		//绘制子弹尾迹
// 		if (BeamParticles)
// 		{
// 			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
// 				GetWorld(),
// 				BeamParticles,
// 				MuzzleFlashSocketFransform
// 			);
// 			if (Beam)
// 			{
// 				Beam->SetVectorParameter(FName("Target"), BeamEnd);
// 			}
// 		}


		//枪口闪光
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, MuzzleFlashSocketFransform);
		}
		//播放枪声
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}
		//播放子弹声音
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
		}

	}
}

void AWeaponUseHitScan::WeaponTraceHit(const FVector& ScaleStartPoint, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{
		FVector ScaleEndPoint = bUseScatter ? TraceEndWithScatter(ScaleStartPoint, HitTarget) : ScaleStartPoint + (HitTarget - ScaleStartPoint) * 1.25f;
		//向前伸出一段，防止因为Mesh的原因无法返回结果
		//使用射击随机分散或者不使用
		
		World->LineTraceSingleByChannel(
			OutHit,
			ScaleStartPoint,
			ScaleEndPoint,
			ECollisionChannel::ECC_Visibility
		);

		//绘制尾迹,默认尾迹终点为检测线终点
		FVector BeamEnd = ScaleEndPoint;
		if (OutHit.bBlockingHit)
		{
			//如果有命中，则尾迹终点为命中点
			BeamEnd = OutHit.ImpactPoint;
		}
		if (BeamParticles)
		{
			//绘制尾迹
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				ScaleStartPoint,
				FRotator::ZeroRotator,
				true
			);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}

FVector AWeaponUseHitScan::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget)
{
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();//追踪起点到HitTarget的Vector
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	
	//随机一个单位向量 * Rand(0，球体半径)
	FVector RandomVectorInSphere = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.0f, RadiusOfSphere);
	//结束点，得出一个从球心出发向外的随机向量
	FVector EndLocation = SphereCenter + RandomVectorInSphere;
	//出发点到结束点
	FVector ToEndLocation = EndLocation - TraceStart;

	DrawDebugSphere(GetWorld(), SphereCenter, RadiusOfSphere, 12, FColor::Green, true);
	DrawDebugSphere(GetWorld(), EndLocation, 4.0f, 4, FColor::Red, true);
	DrawDebugLine(GetWorld(), TraceStart, FVector(TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size()), FColor::Orange, true);

	return FVector(TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size());
}