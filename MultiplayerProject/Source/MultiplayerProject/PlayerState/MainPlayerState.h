// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "MainPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERPROJECT_API AMainPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void OnRep_Score()override;
	UFUNCTION()
	virtual void OnRep_Defeats();
	void AddToScore(float ScoreAmount);
	void AddToDefeats(int32 DefeatAmount);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const override;

private:

	UPROPERTY()
	class AMainCharacter* MainCharacter;

	UPROPERTY()
	class AMainPlayerController* MainPlayerController;

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;


	
	
};
