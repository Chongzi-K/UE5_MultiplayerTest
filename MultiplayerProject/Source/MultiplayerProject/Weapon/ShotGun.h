// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponUseHitScan.h"
#include "ShotGun.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERPROJECT_API AShotGun : public AWeaponUseHitScan
{
	GENERATED_BODY()

public:

	virtual void Fire(const FVector& HitTarget)override;//��Ҫ�����ö��ҩ
	

private:

	UPROPERTY(EditAnywhere, Category = "Scatter")
	uint32 NumOfProjectilesPerShot = 10;
};
