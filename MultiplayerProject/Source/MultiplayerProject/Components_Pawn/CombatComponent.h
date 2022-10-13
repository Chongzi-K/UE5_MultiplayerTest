// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MultiplayerProject/HUD/MainHUD.h"
#include "MultiplayerProject/Weapon/WeaponTypes.h"
#include "MultiplayerProject/Types/CombatState.h"
#include "CombatComponent.generated.h"




UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERPROJECT_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()
	

public:	

	UCombatComponent();

	friend class AMainCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	void EquipWeapon(class AWeapon* WeaponToEquip);

	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	void FireButtonPressed(bool bPressed);

protected:

	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)//RPC ���Դ��뺯����OnRep������
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();



	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);
	//FVector_NetQuantize ��FVector�����࣬��������Ϊ0λС��λ�ĸ���������СΪ20bit������ΧΪ +-2^20

	void TraceUnderCrosshairs(FHitResult& TraceHitResult); 

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server,Reliable)
	void SeverReload();

	void HandleReload();//����ͻ��˷�����ϵ� Reload
	int32 AmountToReload();

private:

	UPROPERTY()
	class AMainCharacter* MainCharacter;


	UPROPERTY()
	class AMainPlayerController* Controller;

	UPROPERTY()
	class AMainHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;

	FVector HitTarget;

	UPROPERTY()
	FHUDPackage HUDPackage;

	//
	//��׼���ӽ�������Զ
	//
	float DefaultFOV;//����׼ʱ��Ĭ����Ұ

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedFOV = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomInterpSpeed = 20.0f;

	float CurrentFOV;

	void InterpFOV(float DeltaTime);

    /**
    *�Զ����� 
    */
	FTimerHandle FireTimer;

	bool bCanFire;//��ֹ����������ٹ���

	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	//��ǰװ����������Ӧ�ĵ�ҩ�����������еĵ�ҩ
	UPROPERTY(ReplicatedUsing= OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	//Map ��֧�ָ��ƣ���Ϊ��ϣ�Ľ���ڷ������Ϳͻ����ϲ�һ����ͬ
	TMap<EWeaponType, int32>CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 4;

	void InitializeCarriedAmmo();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	void UpdateAmmoAmount();

};
