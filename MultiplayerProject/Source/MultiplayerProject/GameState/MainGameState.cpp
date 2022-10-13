// Fill out your copyright notice in the Description page of Project Settings.


#include "MainGameState.h"
#include "Net/UnrealNetwork.h"
#include "MultiplayerProject/MainPlayerController/MainPlayerController.h"
#include "MultiplayerProject/PlayerState/MainPlayerState.h"


void AMainGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMainGameState, PlayerWithTopScore);
}

void AMainGameState::UpdateTopScore(AMainPlayerState* ScoringPlayer)
{
	if (PlayerWithTopScore.Num() == 0)
	{
		PlayerWithTopScore.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		PlayerWithTopScore.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		PlayerWithTopScore.Empty();
		PlayerWithTopScore.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}
