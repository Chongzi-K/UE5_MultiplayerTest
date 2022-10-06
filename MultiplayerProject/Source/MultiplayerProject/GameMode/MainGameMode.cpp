// Fill out your copyright notice in the Description page of Project Settings.


#include "MainGameMode.h"
#include "MultiplayerProject/MainCharacter.h"
#include "MultiplayerProject/MainPlayerController/MainPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "MultiplayerProject/PlayerState/MainPlayerState.h"

void AMainGameMode::PlayerEliminated(AMainCharacter* EliminatedCharacter, AMainPlayerController* VictimController, AMainPlayerController* AttackerController)
{
	AMainPlayerState* AttackerPlayerState = AttackerController ? Cast<AMainPlayerState>(AttackerController->PlayerState) : nullptr;
	AMainPlayerState* VictimPlayerState = VictimController ? Cast<AMainPlayerState>(VictimController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)//排除自杀
	{
		AttackerPlayerState->AddToScore(1.0f);
	}

	if (EliminatedCharacter)
	{
		EliminatedCharacter->Elim();
	}
}

void AMainGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		//隨機選取玩家出生點
		TArray<AActor*> PlayerStartPoints;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStartPoints);
		int32 Selection = FMath::RandRange(0, PlayerStartPoints.Num() - 1);
		//spawn 的時候會檢查重生點周圍的碰撞，默認有碰撞則不 spawn ,需要在Character中更改 spawn collision handling method
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStartPoints[Selection]);
	}
}