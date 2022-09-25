// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameMode_Lobby.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERPROJECT_API AGameMode_Lobby : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
};
