// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERPROJECT_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	virtual void Tick(float DeltaTime)override;

	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeat(int32 Defeat);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountDown(float CountDownTime);
	virtual void OnPossess(APawn* InPawn)override;

protected:

	virtual void BeginPlay()override;

	void SetHUDTime();

private:
	UPROPERTY()
	class AMainHUD* MainHUD;

	float MatchTime = 120.0f;
	uint32 CountDownInt = 0;

	
};
