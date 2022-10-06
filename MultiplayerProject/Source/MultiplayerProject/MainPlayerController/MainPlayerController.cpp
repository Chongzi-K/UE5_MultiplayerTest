// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "MultiplayerProject/HUD/MainHUD.h"
#include "MultiplayerProject/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "MultiplayerProject/MainCharacter.h"

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

void AMainPlayerController::SetHUDScore(float Score)
{
	MainHUD = MainHUD == nullptr ? Cast<AMainHUD>(GetHUD()) : MainHUD;
	if (MainHUD && MainHUD->CharacterOverlay)
	{
		if (MainHUD->CharacterOverlay->ScoreAmountTextBlock)
		{
			FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
			MainHUD->CharacterOverlay->ScoreAmountTextBlock->SetText(FText::FromString(ScoreText));
		}
	}
}

void AMainPlayerController::SetHUDDefeat(int32 Defeat)
{
	MainHUD = MainHUD == nullptr ? Cast<AMainHUD>(GetHUD()) : MainHUD;
	if (MainHUD && MainHUD->CharacterOverlay)
	{
		if (MainHUD->CharacterOverlay->DefeatAmountTextBlock)
		{
			FString DefeatText = FString::Printf(TEXT("%d"), Defeat);
			MainHUD->CharacterOverlay->DefeatAmountTextBlock->SetText(FText::FromString(DefeatText));
		}
	}
}

void AMainPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	MainHUD = MainHUD == nullptr ? Cast<AMainHUD>(GetHUD()) : MainHUD;
	if (MainHUD && MainHUD->CharacterOverlay)
	{
		if (MainHUD->CharacterOverlay->WeaponAmmoAmountTextBlock)
		{
			FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
			MainHUD->CharacterOverlay->DefeatAmountTextBlock->SetText(FText::FromString(AmmoText));
		}
	}
}

void  AMainPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AMainCharacter* MainCharacter = Cast<AMainCharacter>(InPawn);
	if (MainCharacter)
	{
		SetHUDHealth(MainCharacter->GetHealth(), MainCharacter->GetMaxHealth());
	}
}