// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Engine/Engine.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	//指定 Lobby 地图
	PathToLevel_Lobby = FString::Printf(TEXT("%s?listen"),*LobbyPath);
	//根据传入参数修改可加入的连接
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
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);//鼠标自由移动，不局限于窗口
			PlayerController->SetInputMode(InputModeData);//InputModeUIOnly
			PlayerController->SetShowMouseCursor(true);
		}
	}

	//在游戏实例中获取会话子系统
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionsSubsystem)
	{
		//将 委托 MultiplayerOnCreateSessionComplete 绑定到 回调函数 UMenu::OnCreateSession
		//非 Dynamic 的绑定用 AddUObject
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &UMenu::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &UMenu::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &UMenu::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UMenu::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &UMenu::OnStartSession);
	}
}

bool UMenu::Initialize()
{
	//初始化，添加按钮的委托绑定
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
	//切换地图时去除 Menu
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	// 由子系统的委托触发的回调函数：创建会话成功？
	if (bWasSuccessful)
	{
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, FString::Printf(TEXT("Create Session Success! bWasSuccessful=1"))); }
		
		UWorld* World = GetWorld();
		if (World)
		{
			if (World->ServerTravel(PathToLevel_Lobby))//服务器端执行 Server Travel,调用所有在会话中的客户端进行 Client Travel
			{
				if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("ServerTravel: %s !"), *PathToLevel_Lobby)); }
			}
		}
	}
	else//创建会话失败，重新启用按钮功能
	{
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, FString::Printf(TEXT("Create Session fail! bWasSuccessful=0"))); }
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("reset the button"))); }
		HostButton->bIsEnabled = true;
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResult, bool bWasSeccessful)//传入搜寻会话中找到的多个结果
{
	if (MultiplayerSessionsSubsystem == nullptr) { return; }

	for (auto Result : SessionResult)//遍历所有结果
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);//搜寻所有会话的匹配类型（MatchType），并返回结果 SettingsValue
		if (SettingsValue == MatchType)//如果 SettingsValue == 本地MatchType（FreeForAll）
		{
			if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("JoinSession !"))); }
			MultiplayerSessionsSubsystem->JoinSession(Result);//对该会话触发加入操作，并传出结果
			return;
		}
	}
	if (!bWasSeccessful || SessionResult.Num()==0)//如果搜寻会话失败或者找到的会话为 0 ；
	{
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("Do Not Find Session !"))); }
		JoinButton->SetIsEnabled(true);//恢复按钮作用
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
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);//客户端处获得决定前往的会话的连接

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();//获取本地控制器，用于传送
			if (PlayerController)
			{
				if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("ClientTravel"))); }
				PlayerController->ClientTravel(Address,ETravelType::TRAVEL_Absolute);//客户端依据地址传送到服务端地图
			}
		}
	}
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		//找到会话但加入会话失败，恢复按钮功能
		JoinButton->bIsEnabled = true;
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
	if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Click Host Button"))); }

	//按下按钮后立刻禁用再次点击，防止多次点击，会在回调函数中恢复
	HostButton->SetIsEnabled(false);

	if (MultiplayerSessionsSubsystem)
	{
		//有子系统，尝试创建会话
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Include MultiplayerSessionsSubsystem, Ready to create session"))); }
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}

}

void UMenu::JoinButtonClicked()//Menu 的 Join 按钮被按下,尝试搜寻并加入已存在的会话
{
	if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("Click Join Button"))); }

	JoinButton->SetIsEnabled(false);//禁用再次点击，回调函数中恢复

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSessions(10000);//搜寻10000个会话
	}

}

void UMenu::MenuTearDown()//在 地图切换 时被调用，移除 HUD 并恢复控制模式
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