// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	//ָ�� Lobby ��ͼ������������������
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	NumPublicConnections = NumberOfPublicConnections;
	//���ݴ�������޸�ƥ������
	MatchType = TypeOfMatch;
	//���˽�����ʾ���۽�
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	//�л�����ģʽ
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionsSubsystem)
	{
		//�� ί�� MultiplayerOnCreateSessionComplete �󶨵� �ص����� UMenu::OnCreateSession
        //�� Dynamic �İ��� AddUObject
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}

bool UMenu::Initialize()
{
	//��ʼ������Ӱ�ť��ί�а�
	if (!Super::Initialize()) { return false; }

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}

	return true;
}

void UMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	//�л���ͼʱȥ�� Menu
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UWorld* World = GetWorld();
		if (World)//��������ִ�� Server Travel,���������ڻỰ�еĿͻ��˽��� Client Travel
		{
			World->ServerTravel(PathToLobby);
		}
	}
	else//�����Ựʧ�ܣ��������ð�ť����
	{
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Red, FString::Printf(TEXT("Menu::OnCreateSession.bWasSuccessful=false"))); }

		HostButton->SetIsEnabled(true);
	}
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (MultiplayerSessionsSubsystem == nullptr)
	{
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Red, FString::Printf(TEXT("Menu::OnFindSessions.MultiplayerSessionsSubsystem=nullptr"))); }
		return;
	}

	for (auto Result : SessionResults)//�������н��
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		if (SettingsValue == MatchType)//��� SettingsValue == ����MatchType��FreeForAll��
		{
			if(GEngine) { GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Green, FString::Printf(TEXT("Menu::OnFindSessions.SettingsValue == MatchType"))); }
			MultiplayerSessionsSubsystem->JoinSession(Result);//������������ԸûỰ�����������
			return;
		}
	}
	if (!bWasSuccessful || SessionResults.Num() == 0)
	{
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Red, FString::Printf(TEXT("Menu::OnFindSessions.!bWasSuccessful || SessionResults.Num() == 0"))); }
		JoinButton->SetIsEnabled(true);
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);//�ͻ��˴���þ���ǰ���ĻỰ������

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();//��ȡ���ؿ����������ڴ���
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);//�ͻ������ݵ�ַ���͵�����˵�ͼ
			}
		}
	}
}

void UMenu::OnDestroySession(bool bWasSuccessful)//������ Menu �в�����Menu Ҳ����Ҫ��Ӧ
{
}

void UMenu::OnStartSession(bool bWasSuccessful)//������ Menu �в�����Menu Ҳ����Ҫ��Ӧ
{
}

void UMenu::HostButtonClicked()//Menu �� Host ��ť������,�����Լ������Ự
{
	//���°�ť�����̽����ٴε������ֹ��ε�������ڻص������лָ�
	HostButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}

void UMenu::JoinButtonClicked()//Menu �� Join ��ť������,������Ѱ�������Ѵ��ڵĻỰ
{
	//���°�ť�����̽����ٴε������ֹ��ε�������ڻص������лָ�
	JoinButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		//����ϵͳ�����Դ����Ự
		MultiplayerSessionsSubsystem->FindSessions(10000);//��Ѱ10000���Ự
	}
}

void UMenu::MenuTearDown()//�� ��ͼ�л� ʱ�����ã��Ƴ� HUD ���ָ�����ģʽ
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}
