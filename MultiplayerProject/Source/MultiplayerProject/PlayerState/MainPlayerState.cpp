// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerState.h"
#include "MultiplayerProject/MainCharacter.h"
#include "MultiplayerProject/MainPlayerController/MainPlayerController.h"

void AMainPlayerState::OnRep_Score()//客户端修改score
{
	Super::OnRep_Score();

	MainCharacter = MainCharacter == nullptr ? Cast<AMainCharacter>(GetPawn()) : MainCharacter;
	if (MainCharacter)
	{
		MainPlayerController = MainPlayerController == nullptr ? Cast<AMainPlayerController>(MainCharacter->Controller) : MainPlayerController;
		if (MainPlayerController)
		{
			MainPlayerController->SetHUDScore(Score);
		}
	}

}

void AMainPlayerState::AddToScore(float ScoreAmount)//服务端修改score
{
	Score += ScoreAmount;
	MainCharacter = MainCharacter == nullptr ? Cast<AMainCharacter>(GetPawn()) : MainCharacter;
	if (MainCharacter)
	{
		MainPlayerController = MainPlayerController == nullptr ? Cast<AMainPlayerController>(MainCharacter->Controller) : MainPlayerController;
		if (MainPlayerController)
		{
			MainPlayerController->SetHUDScore(Score);
		}
	}
}
