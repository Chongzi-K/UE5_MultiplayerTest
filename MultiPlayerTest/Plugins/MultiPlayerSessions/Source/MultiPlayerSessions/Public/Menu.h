// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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
	void MenuSetup();

protected:

	virtual bool Initialize() override;
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

private:

	//使用meta绑定的c++名称要与蓝图名称一致，否则会导致崩溃
	UPROPERTY(meta=(BindWidget))
	class UButton* HostButton;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	void MenuTearDown();

	//subsystem to handle all online session  functionality
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
	
};
