// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode_Lobby.h"
#include "GameFramework/GameStateBase.h"


void AGameMode_Lobby::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		if (NumberOfPlayers == 2)
		{
			UWorld* World = GetWorld();
			if (World)
			{
				bUseSeamlessTravel = true;
				World->ServerTravel(FString("/Game/Map/GameMap_TEMP?listen"));
			}
		}
	}


}