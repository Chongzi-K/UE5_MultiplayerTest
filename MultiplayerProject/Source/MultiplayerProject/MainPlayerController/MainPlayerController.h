// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MultiplayerProject/HUD/CharacterOverlay.h"
#include "MainPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERPROJECT_API AMainPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	virtual void Tick(float DeltaTime)override;

	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDefeat(int32 Defeat);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDMatchCountDown(float CountDownTime);
	virtual void OnPossess(APawn* InPawn)override;

	virtual float GetServerTime();//同步服务器时间
	virtual void ReceivedPlayer() override;//服务器接收玩家瞬间触发，在这里可以最快速进行时间同步减少误差

	void OnMatchStateSet(FName State);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:

	virtual void BeginPlay()override;

	void SetHUDTime();

	void PollInitialize();

	/**
	 * 同步时间
	 * 因为用的是BeginPlay开始计时，所以服务端时间>客户端时间
	 */
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);//RPC,客户端调用，让服务端发送时间信息

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeOfServerReceivedClientRequest);//客户端执行，用来报告收到的服务端时间

	float ClientServerDelta = 0.0f;//客户端服务端之间的时间差



private:
	UPROPERTY()
	class AMainHUD* MainHUD;

	float MatchTime = 120.0f;
	uint32 CountDownInt = 0;

	UPROPERTY(EditAnywhere)
	float TimeSyncFrequency = 5.0f;//同步时间的频率/秒；

	float TimeSyncRunningTime;//距离上一次同步经过的时间

	void CheckTimeSync(float DeltaTime);

	UPROPERTY(ReplicatedUsing= OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	bool bInitializeChrarcterOverlay= false ;

	float HUD_Health;
	float HUD_MaxHealth;
	float HUD_Socre;
	int32 HUD_Defeats;


};
