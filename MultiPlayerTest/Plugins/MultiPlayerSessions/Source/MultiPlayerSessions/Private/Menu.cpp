// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Engine/Engine.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	//ָ�� Lobby ��ͼ
	PathToLevel_Lobby = FString::Printf(TEXT("%s?listen"),*LobbyPath);
	//���ݴ�������޸Ŀɼ��������
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
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);//��������ƶ����������ڴ���
			PlayerController->SetInputMode(InputModeData);//InputModeUIOnly
			PlayerController->SetShowMouseCursor(true);
		}
	}

	//����Ϸʵ���л�ȡ�Ự��ϵͳ
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionsSubsystem)
	{
		//�� ί�� MultiplayerOnCreateSessionComplete �󶨵� �ص����� UMenu::OnCreateSession
		//�� Dynamic �İ��� AddUObject
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &UMenu::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &UMenu::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &UMenu::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UMenu::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &UMenu::OnStartSession);
	}
}

bool UMenu::Initialize()
{
	//��ʼ������Ӱ�ť��ί�а�
	if (!Super::Initialize()) { return false; }
	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &UMenu::HostButtonClicked);
	}
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &UMenu::JoinButtonClicked);
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
	// ����ϵͳ��ί�д����Ļص������������Ự�ɹ���
	if (bWasSuccessful)
	{
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, FString::Printf(TEXT("Create Session Success! bWasSuccessful=1"))); }
		
		UWorld* World = GetWorld();
		if (World)
		{
			if (World->ServerTravel(PathToLevel_Lobby))//��������ִ�� Server Travel,���������ڻỰ�еĿͻ��˽��� Client Travel
			{
				if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("ServerTravel: %s !"), *PathToLevel_Lobby)); }
			}
		}
	}
	else//�����Ựʧ�ܣ��������ð�ť����
	{
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, FString::Printf(TEXT("Create Session fail! bWasSuccessful=0"))); }
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("reset the button"))); }
		HostButton->bIsEnabled = true;
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResult, bool bWasSeccessful)//������Ѱ�Ự���ҵ��Ķ�����
{
	if (MultiplayerSessionsSubsystem == nullptr) { return; }

	for (auto Result : SessionResult)//�������н��
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);//��Ѱ���лỰ��ƥ�����ͣ�MatchType���������ؽ�� SettingsValue
		if (SettingsValue == MatchType)//��� SettingsValue == ����MatchType��FreeForAll��
		{
			if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("JoinSession !"))); }
			MultiplayerSessionsSubsystem->JoinSession(Result);//�ԸûỰ����������������������
			return;
		}
	}
	if (!bWasSeccessful || SessionResult.Num()==0)//�����Ѱ�Ựʧ�ܻ����ҵ��ĻỰΪ 0 ��
	{
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Do Not Find Session !"))); }
		JoinButton->SetIsEnabled(true);//�ָ���ť����
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("UMenu::OnJoinSession"))); }
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
				if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("ClientTravel"))); }
				PlayerController->ClientTravel(Address,ETravelType::TRAVEL_Absolute);//�ͻ������ݵ�ַ���͵�����˵�ͼ
			}
		}
	}
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		//�ҵ��Ự������Ựʧ�ܣ��ָ���ť����
		JoinButton->bIsEnabled = true;
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
	if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Click Host Button"))); }

	//���°�ť�����̽����ٴε������ֹ��ε�������ڻص������лָ�
	HostButton->SetIsEnabled(false);

	if (MultiplayerSessionsSubsystem)
	{
		//����ϵͳ�����Դ����Ự
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Include MultiplayerSessionsSubsystem, Ready to create session"))); }
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}

}

void UMenu::JoinButtonClicked()//Menu �� Join ��ť������,������Ѱ�������Ѵ��ڵĻỰ
{
	if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Click Join Button"))); }

	JoinButton->SetIsEnabled(false);//�����ٴε�����ص������лָ�

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSessions(10000);//��Ѱ10000���Ự
	}

}

void UMenu::MenuTearDown()//�� ��ͼ�л� ʱ�����ã��Ƴ� HUD ���ָ�����ģʽ
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerContorller = World->GetFirstPlayerController();
		if (PlayerContorller)
		{
			FInputModeGameOnly InputModeData;
			PlayerContorller->SetInputMode(InputModeData);
			PlayerContorller->SetShowMouseCursor(false);
		}
	}
}