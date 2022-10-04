// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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

protected:

	virtual void BeginPlay() override;

	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)//RPC 可以传入函数，OnRep不可以
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void FireButtonPressed(bool bPressed);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);
	//FVector_NetQuantize 是FVector的子类，四舍五入为0位小数位的浮点数，大小为20bit，即范围为 +-2^20

	void TraceUnderCrosshairs(FHitResult& TraceHitResult); 

	void SetHUDCrosshairs(float DeltaTime);


private:

	UPROPERTY()
	class AMainCharacter* MainCharacter;

	class AMainPlayerController* Controller;
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

};
