// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
	//������˱����еĻص�����
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Green, FString::Printf(TEXT("Subsystem.GetSubsystem.Successful"))); }
		//SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface = Subsystem->GetSessionInterface())//��ȡ�Ự�ӿڣ�����Steam���������
		{
			if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Green, FString::Printf(TEXT("Subsystem::GetSessionInterface.Success"))); }
		}
		else
		{
			if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Red, FString::Printf(TEXT("Subsystem::GetSessionInterface.Fail"))); }
		}
	}
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
	DesiredNumPublicConnections = NumPublicConnections;
	DesiredMatchType = MatchType;
	if (!SessionInterface.IsValid())
	{
		//δ��ȡ���ӿ�
		return;
	}
	//���Ի�ȡ�Ѵ��ڵĻỰ
	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		//�Ự��Ϊ�գ������٣����������´���һ��
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Green, FString::Printf(TEXT("ExistingSession != nullptr,Destroying"))); }
		bCreateSessionOnDestroy = true; // ������ʧ��ʱһ�����ٲ����´����Ự�Ļ��ᣬ��Ҫ�����Ҫint��
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;

		DestroySession();
	}

	//��ί�м���ί���б��Ա��Ժ���Խ����ί���б���ɾ��
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
	//ʹ����OnlineSubsystem��������lan��ƥ�䣬δʹ��������
	LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	LastSessionSettings->NumPublicConnections = NumPublicConnections;//�����������
	LastSessionSettings->bAllowJoinInProgress = true;//�����ڻỰ�ѱ�����ʱ����
	LastSessionSettings->bAllowJoinViaPresence = true;//����ͨ��steam����
	LastSessionSettings->bShouldAdvertise = true;//ͨ��steam����
	LastSessionSettings->bUsesPresence = true;//ʹ��steam�˻����ڵ���
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->BuildUniqueId = 1;
	LastSessionSettings->bUseLobbiesIfAvailable = true;

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();

	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
	{
		//�����Ựʧ��
		//���վ����ɾ��ί��
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Red, FString::Printf(TEXT("Subsystem::CreateSession.successful=false"))); }
		//�㲥�Զ���ί�У���Menu�н��ܲ�ִ�ж�Ӧ�Ļص�����
		MultiplayerOnCreateSessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)//������Ҫ��Ѱ�ĻỰ����
{
	if (!SessionInterface.IsValid()) { return; }

	//��ί�м���ί���б�
	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());//��������ָ��
	LastSessionSearch->MaxSearchResults = MaxSearchResults;//��Ѱ�ĻỰ��������
	
	//ͨ���ж��Ƿ����������ϵͳ�������Ƿ�ʹ�� Lan �ϲ�ѯ
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
		//δ�ҵ��Ự
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Green, FString::Printf(TEXT("Subsystem::FindSessions=fasle"))); }
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		//����Ҫ�����ﴦ��if true ��������Ѿ�ͨ��ί��ȥ������������
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (!SessionInterface.IsValid())
	{
		//���������������ϵͳ�ӿڣ��򷵻�δ֪�������ͣ���Ϊ�޷��������˻Ự
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Red, FString::Printf(TEXT("Subsystem::JoinSession.SessionInterface.notValid"))); }
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	//����ί���б�
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult))
	{
		//�������Ự�ɹ�����ִ�� SessionInterface->JoinSession �е�ί�й㲥������Ҫ�ڴ˴����� 
        //����Ựʧ�ܣ��㲥δ֪����
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Red, FString::Printf(TEXT("SessionInterface->JoinSession=fail"))); }
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UMultiplayerSessionsSubsystem::DestroySession()
{
	//�����Լ������ĻỰ
	if (!SessionInterface.IsValid())
	{
		//����ӿ���Ч��ʹ��ί�а󶨵Ļص�����������
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Red, FString::Printf(TEXT("Subsystem::DestroySession.SessionInterface.notValid"))); }
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		//����޷����ٻỰ
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Red, FString::Printf(TEXT("DestroySession: !SessionInterface->DestroySession(NAME_GameSession)"))); }
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::StartSession()
{
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	//�Ƴ�ί��
	if (SessionInterface)
	{
		//����ʹ�ø�ί�У���� OnCreateSessionCompleteDelegate_Handle ���Ƴ� CreateSessionCompleteDelegateHandle
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Red, FString::Printf(TEXT("Subsystem::OnCreateSessionComplete.SessionInterface.notValid"))); }
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	if (LastSessionSearch->SearchResults.Num() <= 0)
	{
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Red, FString::Printf(TEXT("Subsystem::OnFindSessionsComplete.SearchResults.Num() <= 0"))); }
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}
	if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 60.0f, FColor::Green, FString::Printf(TEXT("Subsystem::OnFindSessionsComplete0SearchResults.Num() > 0"))); }
	MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	if (bWasSuccessful && bCreateSessionOnDestroy)
	{
		//���ٻỰʱ���bCreateSessionOnDestroy������棬������ٴδ����Ự
		//��Ϊֻ���ڴ����Ựʧ��ʱ�Ż���� Destroy ,��һ�γ������´����Ự
		bCreateSessionOnDestroy = false;
		CreateSession(LastNumPublicConnections, LastMatchType);
	}
	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}
