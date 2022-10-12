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

	virtual float GetServerTime();//ͬ��������ʱ��
	virtual void ReceivedPlayer() override;//�������������˲�䴥�����������������ٽ���ʱ��ͬ���������

	void OnMatchStateSet(FName State);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:

	virtual void BeginPlay()override;

	void SetHUDTime();

	void PollInitialize();

	/**
	 * ͬ��ʱ��
	 * ��Ϊ�õ���BeginPlay��ʼ��ʱ�����Է����ʱ��>�ͻ���ʱ��
	 */
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);//RPC,�ͻ��˵��ã��÷���˷���ʱ����Ϣ

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeOfServerReceivedClientRequest);//�ͻ���ִ�У����������յ��ķ����ʱ��

	float ClientServerDelta = 0.0f;//�ͻ��˷����֮���ʱ���



private:
	UPROPERTY()
	class AMainHUD* MainHUD;

	float MatchTime = 120.0f;
	uint32 CountDownInt = 0;

	UPROPERTY(EditAnywhere)
	float TimeSyncFrequency = 5.0f;//ͬ��ʱ���Ƶ��/�룻

	float TimeSyncRunningTime;//������һ��ͬ��������ʱ��

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
