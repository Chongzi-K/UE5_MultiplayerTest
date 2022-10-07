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

void AMainPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();
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

void AMainPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	MainHUD = MainHUD == nullptr ? Cast<AMainHUD>(GetHUD()) : MainHUD;
	if (MainHUD && MainHUD->CharacterOverlay)
	{
		if (MainHUD->CharacterOverlay->CarriedAmmoAmountTextBlock)
		{
			FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
			MainHUD->CharacterOverlay->CarriedAmmoAmountTextBlock->SetText(FText::FromString(AmmoText));
		}
	}
}

void AMainPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	AMainCharacter* MainCharacter = Cast<AMainCharacter>(InPawn);
	if (MainCharacter)
	{
		SetHUDHealth(MainCharacter->GetHealth(), MainCharacter->GetMaxHealth());
	}
}

void AMainPlayerController::SetHUDMatchCountDown(float CountDownTime)
{
	MainHUD = MainHUD == nullptr ? Cast<AMainHUD>(GetHUD()) : MainHUD;
	if (MainHUD && MainHUD->CharacterOverlay)
	{
		if (MainHUD->CharacterOverlay->MatchCountDownText)
		{
			int32 Minutes = FMath::FloorToInt(CountDownTime / 60.0f);
			int32 Seconds = CountDownTime - Minutes * 60;
			FString TimeText = FString::Printf(TEXT("%02d : %02d"), Minutes, Seconds);
			MainHUD->CharacterOverlay->MatchCountDownText->SetText(FText::FromString(TimeText));
		}
	}
}

void AMainPlayerController::SetHUDTime()
{
	uint32 SecondLeft = FMath::CeilToInt(MatchTime-GetWorld()->GetTimeSeconds());
	if (CountDownInt != SecondLeft)//转化成整数秒，秒数变化了才修改 HUD ， 实现秒修改 HUD 的效果
	{
		SetHUDMatchCountDown(MatchTime - GetWorld()->GetTimeSeconds());
	}
	CountDownInt = SecondLeft;
}