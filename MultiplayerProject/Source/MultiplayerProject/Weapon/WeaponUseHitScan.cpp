// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponUseHitScan.h"
#include "Engine/SkeletalMeshSocket.h"
#include "MultiplayerProject/MainCharacter.h"
#include "Kismet/GameplayStatics.h"

void AWeaponUseHitScan::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) { return; }//��ȡ����������ʱֱ����Ч
	AController* InstigatorController = OwnerPawn->GetController();/*���� Cast ����ʡ����*/

	const USkeletalMeshSocket* MuzzleFlashScoket = GetWeaponMesh()->GetSocketByName("MuzzleFlashSocket");
	if (MuzzleFlashScoket && InstigatorController)
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
			if (FireHit.bBlockingHit)
			{
				AMainCharacter* MainCharacter = Cast<AMainCharacter>(FireHit.GetActor());
				if (MainCharacter)
				{
					if (HasAuthority())//����˲ű����������˺�
					{
						UGameplayStatics::ApplyDamage(
							MainCharacter,
							Damage,
							InstigatorController,
							this,
							UDamageType::StaticClass()
						);
                    }
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
		}
	}
}