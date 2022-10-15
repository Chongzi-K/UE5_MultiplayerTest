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
	if (OwnerPawn == nullptr) { return; }//��ȡ����������ʱֱ����Ч
	
	AController* InstigatorController = OwnerPawn->GetController();
	//���� Cast ����ʡ����
	//Controller �� Simulate ʵ�� �϶�Ϊ nullptr


	const USkeletalMeshSocket* MuzzleFlashScoket = GetWeaponMesh()->GetSocketByName("MuzzleFlashSocket");
	if (MuzzleFlashScoket)
	{
		FTransform MuzzleFlashSocketFransform = MuzzleFlashScoket->GetSocketTransform(GetWeaponMesh());
		FVector ScaleStartPoint = MuzzleFlashSocketFransform.GetLocation();
		FVector ScaleEndPoint = ScaleStartPoint + (HitTarget - ScaleStartPoint) * 1.25f;//��ǰ���һ�Σ���ֹ��ΪMesh��ԭ���޷����ؽ��

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

			FVector BeamEnd = ScaleEndPoint;//Ԥ�������ӵ�β���յ�Ϊ�������յ�

			//�������
			if (FireHit.bBlockingHit)
			{
				BeamEnd = FireHit.ImpactPoint;//������ɹ������ӵ�β���յ�Ϊ����
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

			//�����ӵ�β��
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

			//������������
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint);
			}
		}

		//ǹ������
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