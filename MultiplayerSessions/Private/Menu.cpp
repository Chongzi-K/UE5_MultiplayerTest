// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	//指定 Lobby 地图，并开启监听服务器
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	NumPublicConnections = NumberOfPublicConnections;
	//根据传入参数修改匹配类型
	MatchType = TypeOfMatch;
	//将此界面显示并聚焦
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	//切换控制模式
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
		//将 委托 MultiplayerOnCreateSessionComplete 绑定到 回调函数 UMenu::OnCreateSession
        //非 Dynamic 的绑定用 AddUObject
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}

bool UMenu::Initialize()
{
	//初始化，添加按钮的委托绑定
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
	//切换地图时去除 Menu
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UWorld* World = GetWorld();
		if (World)//服务器端执行 Server Travel,调用所有在会话中的客户端进行 Client Travel
		{
			World->ServerTravel(PathToLobby);
		}
	}
	else//创建会话失败，重新启用按钮功能
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

	for (auto Result : SessionResults)//遍历所有结果
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		if (SettingsValue == MatchType)//如果 SettingsValue == 本地MatchType（FreeForAll）
		{
			if(GEngine) { GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Green, FString::Printf(TEXT("Menu::OnFindSessions.SettingsValue == MatchType"))); }
			MultiplayerSessionsSubsystem->JoinSession(Result);//传入搜索结果对该会话触发加入操作
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
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);//客户端处获得决定前往的会话的连接

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();//获取本地控制器，用于传送
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);//客户端依据地址传送到服务端地图
			}
		}
	}
}

void UMenu::OnDestroySession(bool bWasSuccessful)//无需在 Menu 中操作，Menu 也不需要响应
{
}

void UMenu::OnStartSession(bool bWasSuccessful)//无需在 Menu 中操作，Menu 也不需要响应
{
}

void UMenu::HostButtonClicked()//Menu 的 Host 按钮被按下,尝试自己创见会话
{
	//按下按钮后立刻禁用再次点击，防止多次点击，会在回调函数中恢复
	HostButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}

void UMenu::JoinButtonClicked()//Menu 的 Join 按钮被按下,尝试搜寻并加入已存在的会话
{
	//按下按钮后立刻禁用再次点击，防止多次点击，会在回调函数中恢复
	JoinButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		//有子系统，尝试创建会话
		MultiplayerSessionsSubsystem->FindSessions(10000);//搜寻10000个会话
	}
}

void UMenu::MenuTearDown()//在 地图切换 时被调用，移除 HUD 并恢复控制模式
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
