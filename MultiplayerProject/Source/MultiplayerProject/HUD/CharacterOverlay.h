// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERPROJECT_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta=(BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HealthTextBlock;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmountTextBlock;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatAmountTextBlock;
	
};
