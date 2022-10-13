// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MainGameMode.generated.h"


namespace MatchState
{
	//�Զ���MatchState����Ҫ��cpp�ж���
	extern MULTIPLAYERPROJECT_API const FName Cooldown;//���Ծֽ���ʱ��ʾʤ������׼����һ���Ծ�

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
	float WarmupTime = 10.0f;//�ȴ�ģʽ�ĳ���ʱ��

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.0f;//һ��������ʱ��

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.0f;//MatchState::Cooldown ������ʱ�䣬���������¿�ʼ�Ծ�

	float LevelStartingTime = 0.0f;//������Ϸ��ͼ��ʼ��ʱ

protected:

	virtual void BeginPlay()override;
	virtual void OnMatchStateSet()override;

private:

	float CountDownTime = 0.f;//�ȴ�ģʽ������Ŀ�ʼ����ʱ


public:

	FORCEINLINE float GetCountdownTime()const { return CountDownTime; }


};
