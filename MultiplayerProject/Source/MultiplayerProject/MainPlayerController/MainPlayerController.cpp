// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "MultiplayerProject/HUD/MainHUD.h"
#include "MultiplayerProject/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	MainHUD = Cast<AMainHUD>(GetHUD());
}

void AMainPlayerController::SetHUDHealth(float Health, float MaxHealth) 
{
	MainHUD = MainHUD == nullptr ? Cast<AMainHUD>(GetHUD()) : MainHUD;
	if (MainHUD && MainHUD->CharacterOverlay)
	{
		if (MainHUD->CharacterOverlay->HealthBar && MainHUD->CharacterOverlay->HealthTextBlock)
		{
			const float HealthPercent = Health / MaxHealth;
			MainHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
			FString HealthText = FString::Printf(TEXT("%d / %d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
			MainHUD->CharacterOverlay->HealthTextBlock->SetText(FText::FromString(HealthText));
		}
	}
}