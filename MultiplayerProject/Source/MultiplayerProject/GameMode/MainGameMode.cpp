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

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)//�ų���ɱ
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
		//�S�C�xȡ��ҳ����c
		TArray<AActor*> PlayerStartPoints;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStartPoints);
		int32 Selection = FMath::RandRange(0, PlayerStartPoints.Num() - 1);
		//spawn �ĕr����z�������c�܇�����ײ��Ĭ�J����ײ�t�� spawn ,��Ҫ��Character�и��� spawn collision handling method
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStartPoints[Selection]);
	}
}