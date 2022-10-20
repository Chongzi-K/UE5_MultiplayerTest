// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")),FString LobbyPath=FString(TEXT("Game/Map/Lobby ")));

protected:

	virtual bool Initialize() override;

	//��ǰ�ؿ����л�ʱ�Զ����ã������ǵ�ǰ�ؿ��͵�ǰ����
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

	//
	//���� MultiplayerSessionSubsysten ���Զ���ί�еĻص�����
	// ��̬������Ҫ��UFUNCTION()�꣬���Ƕ�̬�Ŀ��Բ���Ҫ
	//
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);

	UFUNCTION()
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResult, bool bWasSeccessful);
    
	UFUNCTION()
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

private:

	//ʹ��meta�󶨵�c++����Ҫ����ͼ����һ�£�����ᵼ�±���
	UPROPERTY(meta=(BindWidget))
	class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	void MenuTearDown();

	//��������������߻Ự����ϵͳ
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	int32 NumPublicConnections{4};
	FString MatchType{TEXT("FreeForAll")};
	FString PathToLevel_Lobby{ TEXT("") };
	
};
