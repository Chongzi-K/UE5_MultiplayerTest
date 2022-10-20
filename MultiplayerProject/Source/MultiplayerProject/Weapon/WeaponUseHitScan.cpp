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
	if (OwnerPawn == nullptr) { return; }//��ȡ����������ʱֱ����Ч
	
	AController* InstigatorController = OwnerPawn->GetController();
	//���� Cast ����ʡ����
	//Controller �� Simulate ʵ�� �϶�Ϊ nullptr


	const USkeletalMeshSocket* MuzzleFlashScoket = GetWeaponMesh()->GetSocketByName("MuzzleFlashSocket");
	if (MuzzleFlashScoket)
	{
		FTransform MuzzleFlashSocketFransform = MuzzleFlashScoket->GetSocketTransform(GetWeaponMesh());
		FVector ScaleStartPoint = MuzzleFlashSocketFransform.GetLocation();
		//FVector ScaleEndPoint = ScaleStartPoint + (HitTarget - ScaleStartPoint) * 1.25;

		FHitResult FireHit;
		WeaponTraceHit(ScaleStartPoint, HitTarget, FireHit);//���������ӵ�+��������

		//�������
		if (FireHit.bBlockingHit)
		{
			//BeamEnd = FireHit.ImpactPoint;//������ɹ������ӵ�β���յ�Ϊ����
			AMainCharacter* MainCharacter = Cast<AMainCharacter>(FireHit.GetActor());

			//���к��ڷ�����ִ�������˺�
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

			//������������
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

//		//�����ӵ�β��
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


		//ǹ������
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, MuzzleFlashSocketFransform);
		}
		//����ǹ��
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, FireSound, GetActorLocation());
		}
		//�����ӵ�����
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
		//��ǰ���һ�Σ���ֹ��ΪMesh��ԭ���޷����ؽ��
		//ʹ����������ɢ���߲�ʹ��
		
		World->LineTraceSingleByChannel(
			OutHit,
			ScaleStartPoint,
			ScaleEndPoint,
			ECollisionChannel::ECC_Visibility
		);

		//����β��,Ĭ��β���յ�Ϊ������յ�
		FVector BeamEnd = ScaleEndPoint;
		if (OutHit.bBlockingHit)
		{
			//��������У���β���յ�Ϊ���е�
			BeamEnd = OutHit.ImpactPoint;
		}
		if (BeamParticles)
		{
			//����β��
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
	FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();//׷����㵽HitTarget��Vector
	FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	
	//���һ����λ���� * Rand(0������뾶)
	FVector RandomVectorInSphere = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.0f, RadiusOfSphere);
	//�����㣬�ó�һ�������ĳ���������������
	FVector EndLocation = SphereCenter + RandomVectorInSphere;
	//�����㵽������
	FVector ToEndLocation = EndLocation - TraceStart;

	DrawDebugSphere(GetWorld(), SphereCenter, RadiusOfSphere, 12, FColor::Green, true);
	DrawDebugSphere(GetWorld(), EndLocation, 4.0f, 4, FColor::Red, true);
	DrawDebugLine(GetWorld(), TraceStart, FVector(TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size()), FColor::Orange, true);

	return FVector(TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size());
}