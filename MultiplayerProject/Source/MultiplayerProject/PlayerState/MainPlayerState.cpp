// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerState.h"
#include "MultiplayerProject/MainCharacter.h"
#include "MultiplayerProject/MainPlayerController/MainPlayerController.h"
#include "Net/UnrealNetwork.h"

void AMainPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps)const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMainPlayerState, Defeats);
}

void AMainPlayerState::OnRep_Score()//客户端修改score
{
	Super::OnRep_Score();

	MainCharacter = MainCharacter == nullptr ? Cast<AMainCharacter>(GetPawn()) : MainCharacter;
	if (MainCharacter)
	{
		MainPlayerController = MainPlayerController == nullptr ? Cast<AMainPlayerController>(MainCharacter->Controller) : MainPlayerController;
		if (MainPlayerController)
		{
			MainPlayerController->SetHUDScore(GetScore());
		}
	}

}

void AMainPlayerState::AddToScore(float ScoreAmount)//服务端修改score
{
	SetScore(GetScore() + ScoreAmount);
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

void AMainPlayerState::AddToDefeats(int32 DefeatAmount)//服务器更新 Defeats 并发送复制，更新本地HUD
{
	Defeats += DefeatAmount;
	MainCharacter = MainCharacter == nullptr ? Cast<AMainCharacter>(GetPawn()) : MainCharacter;
	if (MainCharacter)
	{
		MainPlayerController = MainPlayerController == nullptr ? Cast<AMainPlayerController>(MainCharacter->Controller) : MainPlayerController;
		if (MainPlayerController)
		{
			MainPlayerController->SetHUDDefeat(Defeats);
		}
	}
}
void AMainPlayerState::OnRep_Defeats()//客户端收到复制后更新HUD
{
	MainCharacter = MainCharacter == nullptr ? Cast<AMainCharacter>(GetPawn()) : MainCharacter;
	if (MainCharacter)
	{
		MainPlayerController = MainPlayerController == nullptr ? Cast<AMainPlayerController>(MainCharacter->Controller) : MainPlayerController;
		if (MainPlayerController)
		{
			MainPlayerController->SetHUDDefeat(Defeats);
		}
	}
}
