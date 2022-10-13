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
	bDelayedStart = true;//游戏开始的时候会进入等待模式，会生成一个可自由飞行的 pawn 给玩家，直到服务器调用 start
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
			//倒计时模式+倒计时结束=重启对局
			RestartGame();//不会在客户端生效
		}
	}
}

void AMainGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		//使用迭代器，遍历存在的所有 controller
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

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)//排除自杀
	{
		AttackerPlayerState->AddToScore(1.0f);
	}

	if (EliminatedCharacter)//淘汰玩家
	{
		EliminatedCharacter->Elim();
	}

	if (VictimPlayerState)//受害者 PlayerState 中被击败数+1
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
		//隨機選取玩家出生點
		TArray<AActor*> PlayerStartPoints;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStartPoints);
		int32 Selection = FMath::RandRange(0, PlayerStartPoints.Num() - 1);
		//spawn 的時候會檢查重生點周圍的碰撞，默認有碰撞則不 spawn ,需要在Character中更改 spawn collision handling method
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStartPoints[Selection]);
	}
}