// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "Projectile_Rocket.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERPROJECT_API AProjectile_Rocket : public AProjectile
{
	GENERATED_BODY()

public:
	AProjectile_Rocket();

protected:

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)override;

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMesh;
};
