// Fill out your copyright notice in the Description page of Project Settings.


#include "MainPlayerController.h"
#include "MultiplayerProject/HUD/MainHUD.h"
#include "MultiplayerProject/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "MultiplayerProject/MainCharacter.h"
#include "Net/UnrealNetwork.h"
#include "MultiplayerProject/GameMode/MainGameMode.h"
#include "MultiplayerProject/HUD/Announcement.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerProject/Components_Pawn/CombatComponent.h"


void AMainPlayerController::BeginPlay()
{
	Super::BeginPlay();

	MainHUD = Cast<AMainHUD>(GetHUD());

	ServerChenkMatchState();
}

void AMainPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMainPlayerController, MatchState);

}

void AMainPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInitialize();
	//TODO - 从 Tick 中移除 PollInitialize ，提高性能
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
	else
	{
		bInitializeChrarcterOverlay = true;
		HUD_Health = Health;
		HUD_MaxHealth = MaxHealth;
	}
}

void AMainPlayerController::SetHUDScore(float InScore)
{
	MainHUD = MainHUD == nullptr ? Cast<AMainHUD>(GetHUD()) : MainHUD;
	if (MainHUD && MainHUD->CharacterOverlay)
	{
		if (MainHUD->CharacterOverlay->ScoreAmountTextBlock)
		{
			FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(InScore));
			MainHUD->CharacterOverlay->ScoreAmountTextBlock->SetText(FText::FromString(ScoreText));
		}
	}
	else
	{
		bInitializeChrarcterOverlay = true;
		HUD_Socre = InScore;
	}
}

void AMainPlayerController::SetHUDDefeat(int32 InDefeats)
{
	MainHUD = MainHUD == nullptr ? Cast<AMainHUD>(GetHUD()) : MainHUD;
	if (MainHUD && MainHUD->CharacterOverlay)
	{
		if (MainHUD->CharacterOverlay->DefeatAmountTextBlock)
		{
			FString DefeatText = FString::Printf(TEXT("%d"), InDefeats);
			MainHUD->CharacterOverlay->DefeatAmountTextBlock->SetText(FText::FromString(DefeatText));
		}
	}
	else
	{
		bInitializeChrarcterOverlay = true;
		HUD_Defeats = InDefeats;
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
			if (CountDownTime < 0.0f)
			{
				MainHUD->CharacterOverlay->MatchCountDownText->SetText(FText());
				return;
			}
			int32 Minutes = FMath::FloorToInt(CountDownTime / 60.0f);
			int32 Seconds = CountDownTime - Minutes * 60;
			FString TimeText = FString::Printf(TEXT("%02d : %02d"), Minutes, Seconds);
			MainHUD->CharacterOverlay->MatchCountDownText->SetText(FText::FromString(TimeText));
		}
	}
}

void AMainPlayerController::SetHUDAnnouncementCountDown(float CountDownTime)
{
	MainHUD = MainHUD == nullptr ? Cast<AMainHUD>(GetHUD()) : MainHUD;
	if (MainHUD && MainHUD->CharacterOverlay)
	{
		if (MainHUD->Announcement->WarmupTime)
		{
			if (CountDownTime < 0.0f)
			{
				MainHUD->Announcement->WarmupTime->SetText(FText());
				return;
			}
			int32 Minutes = FMath::FloorToInt(CountDownTime / 60.0f);
			int32 Seconds = CountDownTime - Minutes * 60;
			FString TimeText = FString::Printf(TEXT("%02d : %02d"), Minutes, Seconds);
			MainHUD->Announcement->WarmupTime->SetText(FText::FromString(TimeText));
		}
	}
}

void AMainPlayerController::SetHUDTime()
{
	float TimeLeft = 0.0f;
	if (MatchState == MatchState::WaitingToStart) { TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime; }
	else if (MatchState == MatchState::InProgress) { TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime; }
	else if (MatchState == MatchState::Cooldown) { TimeLeft =CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime; }

	uint32 SecondLeft = FMath::CeilToInt(TimeLeft);

	if (HasAuthority())
	{
		//服务器上执行
		MainGameMode = MainGameMode == nullptr ? Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this)) : MainGameMode;
		if (MainGameMode)
		{
			SecondLeft = FMath::CeilToInt(MainGameMode->GetCountdownTime() + LevelStartingTime);
	    }
	}


	if (CountDownInt != SecondLeft)//转化成整数秒，秒数变化了才修改 HUD ， 实现秒修改 HUD 的效果
	{
		if (MatchState == MatchState::WaitingToStart || MatchState==MatchState::Cooldown)//在等待开始的倒计时模式
		{
			SetHUDAnnouncementCountDown(SecondLeft);
		}
		if (MatchState == MatchState::InProgress)//游戏中
		{
			SetHUDMatchCountDown(SecondLeft);
		}
	}
	CountDownInt = SecondLeft;
}

void AMainPlayerController::PollInitialize()
{
	if (CharacterOverlay == nullptr)
	{
		if (MainHUD && MainHUD->CharacterOverlay)
		{
			CharacterOverlay = MainHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				SetHUDHealth(HUD_Health, HUD_MaxHealth);
				//OnPossese 调用 SetHUDHealth ，但此时HUD未完全初始化，
				//导致当时获取 HUD 成功但属性设置失败，故丛中保存数据在此处重新设置
				//此方法在tick中调用
				SetHUDDefeat(HUD_Defeats);
				SetHUDScore(HUD_Socre);
			}
		}
	}
}

void AMainPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();//获取服务器绝对时间
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);//客户端发送请求的时间，服务器收到请求的时间
}

void AMainPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeOfServerReceivedClientRequest)
{
	//在客户端计算服务端时间

	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;//信息从客户端→服务端→客户端所需的时间
	float CurrentServerTime = TimeOfServerReceivedClientRequest + (0.5f * RoundTripTime);//服务器收到客户端请求的时间+半程时间=客户端计算出的服务端时间
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();//双端时间差=计算出的服务端时间-客户端时间
}

float AMainPlayerController::GetServerTime()
{
	if (HasAuthority()) 
	{ 
		//服务端直接返回本地时间
		return GetWorld()->GetTimeSeconds(); 
	}
	else
	{
		return GetWorld()->GetTimeSeconds() + ClientServerDelta;
	}

}

void AMainPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (HasAuthority())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());//服务器调用获取本地时间并广播
	}
}

void AMainPlayerController::ServerChenkMatchState_Implementation()
{
	AMainGameMode* GameMode = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this));//只有在服务端才能获取 GameMode ,客户端调用只会返回 nullptr
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		CooldownTime = GameMode->CooldownTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);//服务端发送给客户端ClientJoinMidGame_Implementation

		if (MainHUD && MatchState == MatchState::WaitingToStart)
		{
			MainHUD->AddAnnouncement();
		}
	}
}

void AMainPlayerController::ClientJoinMidGame_Implementation(FName InMatchState,float InWarmupTime,float InMatchTime, float InCooldownTime, float InLevelStartingTime)
{
	//本方法由服务端向客户端调用，用于处理客户端中途加入时的信息同步
	WarmupTime = InWarmupTime;
	MatchTime = InMatchTime;
	LevelStartingTime = InLevelStartingTime;
	CooldownTime = InCooldownTime;
	MatchState = InMatchState;
	OnMatchStateSet(MatchState);

	if (MainHUD && MatchState == MatchState::WaitingToStart)
	{
		MainHUD->AddAnnouncement();
	}
}

void AMainPlayerController::CheckTimeSync(float DeltaTime)//检查是否需要更新时间
{
	TimeSyncRunningTime += DeltaTime;
	if (TimeSyncRunningTime > TimeSyncFrequency && IsLocalController())
	{
		//当同步计时达到并且是本机则同步一次时间
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.0f;
	}
}

void AMainPlayerController::OnMatchStateSet(FName State)//
{
	//MatchState 变化后服务器处理
	MatchState = State;
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AMainPlayerController::OnRep_MatchState()
{
	//MatchState 变化后客户端处理
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void AMainPlayerController::HandleMatchHasStarted()
{
	MainHUD = MainHUD == nullptr ? Cast <AMainHUD>(GetHUD()) : MainHUD;
	if (MainHUD)
	{
		MainHUD->AddCharacterOverlay();
		if (MainHUD->Announcement)
		{
			MainHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void AMainPlayerController::HandleCooldown()
{
	MainHUD = MainHUD == nullptr ? Cast <AMainHUD>(GetHUD()) : MainHUD;
	if (MainHUD)
	{
		MainHUD->CharacterOverlay->RemoveFromParent();
		if (MainHUD->Announcement && MainHUD->Announcement->AnnouncementText && MainHUD->Announcement->InfoText)
		{
			//对局结束后，重新显示 Announcement
			MainHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("新对局即将开始：");
			MainHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));
			MainHUD->Announcement->InfoText->SetText(FText());

		}
	}
	AMainCharacter* MainCharacter = Cast<AMainCharacter>(GetPawn());
	if (MainCharacter&&MainCharacter->GetCombatComponent())
	{
		MainCharacter->bDisableGamePlay = true;
		MainCharacter->GetCombatComponent()->FireButtonPressed(false);
	}
}