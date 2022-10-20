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

protected:

	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);

	void WeaponTraceHit(const FVector& ScaleStartPoint, const FVector& HitTarget, FHitResult& OutHit);


	UPROPERTY(EditAnywhere)
		USoundCue* HitSound;

	UPROPERTY(EditAnywhere)
		class UParticleSystem* ImpactParticleSystem;

	UPROPERTY(EditAnywhere)
		float Damage = 20.0f;
private:

	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;//���߼�����������Ч���������ӵ�β��

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;


	/**
	 * ���ڽ������ɢ��ļ���
	 */
	UPROPERTY(EditAnywhere, Category = "Scatter")
	float DistanceToSphere = 800.0f;//��һ���̶������Բ��ȡ��

	UPROPERTY(EditAnywhere, Category = "Scatter")
	float RadiusOfSphere = 75.0f;//��һ���̶������Բ��ȡ��

	UPROPERTY(EditAnywhere, Category = "Scatter")
	bool bUseScatter = false;


};
