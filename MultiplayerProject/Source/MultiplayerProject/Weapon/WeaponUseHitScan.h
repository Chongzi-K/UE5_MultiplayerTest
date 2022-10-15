// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Sound/SoundCue.h"
#include "WeaponUseHitScan.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERPROJECT_API AWeaponUseHitScan : public AWeapon
{
	GENERATED_BODY()

public:

	virtual void Fire(const FVector& HitTarget)override;

private:

	UPROPERTY(EditAnywhere)
	float Damage = 20.0f;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticleSystem;

	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;//���߼�����������Ч���������ӵ�β��

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
		USoundCue* FireSound;

	UPROPERTY(EditAnywhere)
		USoundCue* HitSound;

};
