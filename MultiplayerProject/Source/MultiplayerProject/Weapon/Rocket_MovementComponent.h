// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Rocket_MovementComponent.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERPROJECT_API URocket_MovementComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()

protected:
	EHandleBlockingHitResult HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)override;

	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta = FVector::ZeroVector) override;
	
};
