// Fill out your copyright notice in the Description page of Project Settings.


#include "MainGameMode.h"
#include "MultiplayerProject/MainCharacter.h"
#include "MultiplayerProject/MainPlayerController/MainPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "MultiplayerProject/PlayerState/MainPlayerState.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

AMainGameMode::AMainGameMode()
{
	bDelayedStart = true;//��Ϸ��ʼ��ʱ������ȴ�ģʽ��������һ�������ɷ��е� pawn ����ң�ֱ������������ start
}

void AMainGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void AMainGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountDownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.0f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountDownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.0f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountDownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime < 0.0f)
		{
			//����ʱģʽ+����ʱ����=�����Ծ�
			RestartGame();//�����ڿͻ�����Ч
		}
	}
}

void AMainGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		//ʹ�õ��������������ڵ����� controller
		AMainPlayerController* MainPlayer = Cast<AMainPlayerController>(*It);
		if (MainPlayer)
		{
			MainPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void AMainGameMode::PlayerEliminated(AMainCharacter* EliminatedCharacter, AMainPlayerController* VictimController, AMainPlayerController* AttackerController)
{
	AMainPlayerState* AttackerPlayerState = AttackerController ? Cast<AMainPlayerState>(AttackerController->PlayerState) : nullptr;
	AMainPlayerState* VictimPlayerState = VictimController ? Cast<AMainPlayerState>(VictimController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)//�ų���ɱ
	{
		AttackerPlayerState->AddToScore(1.0f);
	}

	if (EliminatedCharacter)//��̭���
	{
		EliminatedCharacter->Elim();
	}

	if (VictimPlayerState)//�ܺ��� PlayerState �б�������+1
	{
		VictimPlayerState->AddToDefeats(1);
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