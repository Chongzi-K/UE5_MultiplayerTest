// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GameMode_Lobby.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERTEST_API AGameMode_Lobby : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer)override;
	virtual void Logout(AController* Exiting) override;
	
};
