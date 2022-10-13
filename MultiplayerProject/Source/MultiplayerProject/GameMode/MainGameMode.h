// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MainGameMode.generated.h"


namespace MatchState
{
	//自定义MatchState，需要在cpp中定义
	extern MULTIPLAYERPROJECT_API const FName Cooldown;//当对局结束时显示胜方，并准备下一场对局

}

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

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.0f;//一场比赛的时间

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.0f;//MatchState::Cooldown 持续的时间，结束后重新开始对局

	float LevelStartingTime = 0.0f;//进入游戏地图开始计时

protected:

	virtual void BeginPlay()override;
	virtual void OnMatchStateSet()override;

private:

	float CountDownTime = 0.f;//等待模式结束后的开始倒计时


public:

	FORCEINLINE float GetCountdownTime()const { return CountDownTime; }


};
