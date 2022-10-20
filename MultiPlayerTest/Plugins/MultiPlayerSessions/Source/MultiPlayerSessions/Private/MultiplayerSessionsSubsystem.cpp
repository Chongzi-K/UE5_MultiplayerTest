// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h" 
#include "OnlineSessionSettings.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this,&UMultiplayerSessionsSubsystem::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &UMultiplayerSessionsSubsystem::OnStartSessionComplete))
	//������˱����еĻص�����
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		SessionInterface = Subsystem->GetSessionInterface();//��ȡ�Ự�ӿڣ�����Steam���������
	}
}

void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType)
{
	if (!SessionInterface.IsValid()) { return; }//δ��ȡ���Ự�ӿ�
	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);//���Ի�ȡ�Ѵ��ڵĻỰ
	if (ExistingSession != nullptr)
	{
		//�Ự��Ϊ�գ������٣����������´���һ��
		bCreateSessionOnDestroy = true;//������ʧ��ʱһ�����ٲ����´����Ự�Ļ��ᣬ��Ҫ�����Ҫint��
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;
		DestroySession();
	}
	//��ί�м���ί���б��Ա��Ժ���Խ����ί���б���ɾ��
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());
    
	//�����趨ʹ����OnlineSubsystem��������lan��ƥ�䣬δʹ��������
	LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : true;
	LastSessionSettings->NumPublicConnections = NumPublicConnections;//�����������
	LastSessionSettings->bAllowJoinInProgress = true;//�����ڻỰ�ѱ�����ʱ����
	LastSessionSettings->bAllowJoinViaPresence = true;//����ͨ��steam����
	LastSessionSettings->bShouldAdvertise = true;//ͨ��steam����
	LastSessionSettings->bUsesPresence = true;//ʹ��steam�˻����ڵ���
	LastSessionSettings->bUseLobbiesIfAvailable = true;//����ʹ�� Lobby
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->BuildUniqueId = 1;

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();//��ȡ���ؿ�����
	
	//���Դ����Ự
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
	{
		//��������Ựʧ��

		//���վ����ɾ��ί��
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		//�㲥ί�У��� Menu �� ���� �н��ܲ�ִ�ж�Ӧ�Ļص�����
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
	//ͨ���ж��Ƿ����������ϵͳ�������Ƿ�ʹ�� Lan ����
	LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : true;
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), LastSessionSearch.ToSharedRef()))
	{
		//δ�ҵ��Ự
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(),false);
	    //����Ҫ�����ﴦ��if true ��������Ѿ�ͨ��ί��ȥ����������ĺ�����
	}


}

void UMultiplayerSessionsSubsystem::JoinSession(const FOnlineSessionSearchResult& SessionResult)
{
	if (!SessionInterface.IsValid()) 
	{ 
		//���������������ϵͳ���򷵻�δ֪�������ͣ���Ϊ�޷��������˻Ự
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return; 
	}

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult))
	{
		//�������Ự�ɹ�����ִ�� SessionInterface->JoinSession �е�ί�й㲥������Ҫ�ڴ˴����� 
		//����Ựʧ�ܣ��㲥δ֪����
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
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		//����޷����ٻỰ
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionComplete.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::StartSession()
{

}


//���ܹ㲥
void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	//�����Ự�ɹ�
	if (SessionInterface)
	{
		//����ʹ�ø�ί�У���� OnCreateSessionCompleteDelegate_Handle ���Ƴ� CreateSessionCompleteDelegateHandle
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}
	if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FString::Printf(TEXT("MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful)"))); }
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
		MultiplayerOnFindSessionsComplete.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}
	MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	
	MultiplayerOnJoinSessionComplete.Broadcast(Result);
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		SessionInterface->ClearOnEndSessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	if (bWasSuccessful && bCreateSessionOnDestroy)//���ٻỰʱ���bCreateSessionOnDestroy������棬������ٴδ����Ự
	{
		//��Ϊֻ���ڴ����Ựʧ��ʱ�Ż���� Destroy ,��һ�γ������´����Ự
		bCreateSessionOnDestroy = false;
		CreateSession(LastNumPublicConnections, LastMatchType);
	}
	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{

}