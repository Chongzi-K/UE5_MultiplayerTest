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

	virtual void Destroyed()override;

protected:

	
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)override;

	virtual void BeginPlay()override;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* RocketTrailSystem;

	UPROPERTY()
	class UNiagaraComponent* RocketTrailSystemComponent;

	void DestroyRocketTrailWhenTimerFinished();

	UPROPERTY(EditAnywhere)
	USoundCue* ProjectileFlyingLoop;

	UPROPERTY()
	UAudioComponent* ProjectileFlyingLoopAudioComponent;

	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;//“Ù¡øÀ•ºı

	UPROPERTY(VisibleAnywhere)
	class URocket_MovementComponent* RocketMovementComponent;

private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMesh;

	FTimerHandle DestroyRocketTrailHandle;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.0f;
};
