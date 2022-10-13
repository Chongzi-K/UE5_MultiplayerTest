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
	//TODO - �� Tick ���Ƴ� PollInitialize ���������
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
	else if (MatchState == MatchState::InProgress) { WarmupTime + MatchTime - GetServerTime() + LevelStartingTime; }

	uint32 SecondLeft = FMath::CeilToInt(TimeLeft);

	if (CountDownInt != SecondLeft)//ת���������룬�����仯�˲��޸� HUD �� ʵ�����޸� HUD ��Ч��
	{
		if (MatchState == MatchState::WaitingToStart)//�ڵȴ���ʼ�ĵ���ʱģʽ
		{
			SetHUDAnnouncementCountDown(SecondLeft);
		}
		if (MatchState == MatchState::InProgress)//��Ϸ��
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
				//OnPossese ���� SetHUDHealth ������ʱHUDδ��ȫ��ʼ����
				//���µ�ʱ��ȡ HUD �ɹ�����������ʧ�ܣ��ʴ��б��������ڴ˴���������
				//�˷�����tick�е���
				SetHUDDefeat(HUD_Defeats);
				SetHUDScore(HUD_Socre);
			}
		}
	}
}

void AMainPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();//��ȡ����������ʱ��
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);//�ͻ��˷��������ʱ�䣬�������յ������ʱ��
}

void AMainPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeOfServerReceivedClientRequest)
{
	//�ڿͻ��˼�������ʱ��

	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;//��Ϣ�ӿͻ��ˡ�����ˡ��ͻ��������ʱ��
	float CurrentServerTime = TimeOfServerReceivedClientRequest + (0.5f * RoundTripTime);//�������յ��ͻ��������ʱ��+���ʱ��=�ͻ��˼�����ķ����ʱ��
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();//˫��ʱ���=������ķ����ʱ��-�ͻ���ʱ��
}

float AMainPlayerController::GetServerTime()
{
	if (HasAuthority()) 
	{ 
		//�����ֱ�ӷ��ر���ʱ��
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
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());//���������û�ȡ����ʱ�䲢�㲥
	}
}

void AMainPlayerController::ServerChenkMatchState_Implementation()
{
	AMainGameMode* GameMode = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this));//ֻ���ڷ���˲��ܻ�ȡ GameMode ,�ͻ��˵���ֻ�᷵�� nullptr
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidGame(MatchState, WarmupTime, MatchTime, LevelStartingTime);

		if (MainHUD && MatchState == MatchState::WaitingToStart)
		{
			MainHUD->AddAnnouncement();
		}
	}
}

void AMainPlayerController::ClientJoinMidGame_Implementation(FName InMatchState,float InWarmupTime,float InMatchTime,float InLevelStartingTime)
{
	//�������ɷ������ͻ��˵��ã����ڴ���ͻ�����;����ʱ����Ϣͬ��
	WarmupTime = InWarmupTime;
	MatchTime = InMatchTime;
	LevelStartingTime = InLevelStartingTime;
	MatchState = InMatchState;
	OnMatchStateSet(MatchState);

	if (MainHUD && MatchState == MatchState::WaitingToStart)
	{
		MainHUD->AddAnnouncement();
	}
}

void AMainPlayerController::CheckTimeSync(float DeltaTime)//����Ƿ���Ҫ����ʱ��
{
	TimeSyncRunningTime += DeltaTime;
	if (TimeSyncRunningTime > TimeSyncFrequency && IsLocalController())
	{
		//��ͬ����ʱ�ﵽ�����Ǳ�����ͬ��һ��ʱ��
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.0f;
	}
}

void AMainPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
	if (MatchState == MatchState::WaitingToStart)
	{

	}

	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
}

void AMainPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
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