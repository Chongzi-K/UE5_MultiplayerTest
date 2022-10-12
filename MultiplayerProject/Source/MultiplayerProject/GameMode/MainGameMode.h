// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MainGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERPROJECT_API AMainGameMode : public AGameMode
{
	GENERATED_BODY()

public:

	AMainGameMode();

	virtual void Tick(float DeltaTime);

	virtual void PlayerEliminated(class AMainCharacter* EliminatedCharacter,class AMainPlayerController* VictimController, AMainPlayerController* AttackerController);
	
	virtual void RequestRespawn(ACharacter* ElimmedCharacter,AController* ElimmedController);

	UPROPERTY(EditDefaultsOnly)
		float WarmupTime = 10.0f;//等待模式的持续时间

	float LevelStartingTime = 0.0f;//进入游戏地图开始计时

protected:

	virtual void BeginPlay()override;
	virtual void OnMatchStateSet()override;

private:

	float CountDownTime = 0.f;//等待模式结束后的开始倒计时




};
