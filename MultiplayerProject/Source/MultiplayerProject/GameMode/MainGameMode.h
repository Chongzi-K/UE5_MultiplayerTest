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
		float WarmupTime = 10.0f;//�ȴ�ģʽ�ĳ���ʱ��

	float LevelStartingTime = 0.0f;//������Ϸ��ͼ��ʼ��ʱ

protected:

	virtual void BeginPlay()override;
	virtual void OnMatchStateSet()override;

private:

	float CountDownTime = 0.f;//�ȴ�ģʽ������Ŀ�ʼ����ʱ




};
