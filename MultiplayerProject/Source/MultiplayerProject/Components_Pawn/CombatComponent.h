// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MultiplayerProject/HUD/MainHUD.h"
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

	UFUNCTION(Server, Reliable)//RPC ���Դ��뺯����OnRep������
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void FireButtonPressed(bool bPressed);

	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);
	//FVector_NetQuantize ��FVector�����࣬��������Ϊ0λС��λ�ĸ���������СΪ20bit������ΧΪ +-2^20

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

};
