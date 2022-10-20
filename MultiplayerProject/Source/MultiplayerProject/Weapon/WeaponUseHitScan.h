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
	UParticleSystem* BeamParticles;//射线检测产生的粒子效果，用作子弹尾迹

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;


	/**
	 * 用于进行射击散点的计算
	 */
	UPROPERTY(EditAnywhere, Category = "Scatter")
	float DistanceToSphere = 800.0f;//用一个固定距离的圆来取点

	UPROPERTY(EditAnywhere, Category = "Scatter")
	float RadiusOfSphere = 75.0f;//用一个固定距离的圆来取点

	UPROPERTY(EditAnywhere, Category = "Scatter")
	bool bUseScatter = false;


};
