// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	if (!HasAuthority()) { return; }

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector MuzzleSocketToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator MuzzleSocketToTargetRotation = MuzzleSocketToTarget.Rotation();
		if (ProjectileClass&&InstigatorPawn)
		{
			FActorSpawnParameters SpawnParms;
			SpawnParms.Owner = GetOwner();
			SpawnParms.Instigator = InstigatorPawn;
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<AProjectile>(
					ProjectileClass,
					SocketTransform.GetLocation(),
					MuzzleSocketToTargetRotation,
					SpawnParms
					);
			}
	    }

    }
}