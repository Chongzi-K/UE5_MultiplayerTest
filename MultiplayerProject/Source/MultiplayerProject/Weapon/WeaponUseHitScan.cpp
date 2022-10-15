// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponUseHitScan.h"
#include "Engine/SkeletalMeshSocket.h"
#include "MultiplayerProject/MainCharacter.h"
#include "Kismet/GameplayStatics.h"

void AWeaponUseHitScan::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) { return; }//获取不到所有者时直接无效
	AController* InstigatorController = OwnerPawn->GetController();/*不用 Cast ，节省性能*/

	const USkeletalMeshSocket* MuzzleFlashScoket = GetWeaponMesh()->GetSocketByName("MuzzleFlashSocket");
	if (MuzzleFlashScoket && InstigatorController)
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
			if (FireHit.bBlockingHit)
			{
				AMainCharacter* MainCharacter = Cast<AMainCharacter>(FireHit.GetActor());
				if (MainCharacter)
				{
					if (HasAuthority())//服务端才被允许申请伤害
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