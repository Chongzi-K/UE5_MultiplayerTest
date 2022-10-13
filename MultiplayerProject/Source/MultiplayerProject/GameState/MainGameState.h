// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MainGameState.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERPROJECT_API AMainGameState : public AGameState
{
	GENERATED_BODY()

public:

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(Replicated)
	TArray<class AMainPlayerState*> PlayerWithTopScore;

	void UpdateTopScore(AMainPlayerState* ScoringPlayer);

private:

	float  TopScore = 0.0f;
	
};
