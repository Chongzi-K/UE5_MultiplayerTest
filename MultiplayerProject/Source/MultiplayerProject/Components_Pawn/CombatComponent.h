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

	UFUNCTION(Server, Reliable)//RPC 可以传入函数，OnRep不可以
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();



	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);
	//FVector_NetQuantize 是FVector的子类，四舍五入为0位小数位的浮点数，大小为20bit，即范围为 +-2^20

	void TraceUnderCrosshairs(FHitResult& TraceHitResult); 

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server,Reliable)
	void SeverReload();

	void HandleReload();//处理客户端服务端上的 Reload
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
	//瞄准与视角拉进拉远
	//
	float DefaultFOV;//不瞄准时的默认视野

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedFOV = 30.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomInterpSpeed = 20.0f;

	float CurrentFOV;

	void InterpFOV(float DeltaTime);

    /**
    *自动连射 
    */
	FTimerHandle FireTimer;

	bool bCanFire;//防止鼠标连点射速过快

	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	//当前装备的武器对应的弹药，而不是所有的弹药
	UPROPERTY(ReplicatedUsing= OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	//Map 不支持复制，因为哈希的结果在服务器和客户端上不一定相同
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
