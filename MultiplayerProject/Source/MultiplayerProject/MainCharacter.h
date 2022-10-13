// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MultiplayerProject/Types/TurningInPlace.h"
#include "MultiplayerProject/Interfaces/InteractWithCrosshairsInterface.h"
#include "MultiplayerProject/Types/CombatState.h"
#include "MultiplayerProject/Components_Pawn/CombatComponent.h"
#include "MainCharacter.generated.h"


UCLASS()
class MULTIPLAYERPROJECT_API AMainCharacter : public ACharacter,public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:

	AMainCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	//������ȷ�� ��Щ�����ڷ����仯ʱ���Ƶ��ͻ�����
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents()override;//��ʼ�����
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayElimMontage();
	virtual void OnRep_ReplicatedMovement()override;

	//����̭
	void Elim();
	UFUNCTION(NetMulticast,Reliable)
	void MulticastElim();

	FTimerHandle ElimTimer;

	void ElimTimerFinish();

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.0f;

	UPROPERTY(Replicated)
	bool bDisableGamePlay = false;//�Ծֽ�����������룬ֻ���������ת����ͷ

	void FireButtonPressed();//��Ҫ��

protected:

	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void ReloadButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void AimOffset(float DeltaTime);
	void RotateInPlace(float DeltaTime);

	void CaculateAimOffset_Pitch();

	void SimulateProxiesTurn();
	virtual void Jump()override;

	void FireButtonReleased();
	void PlayHitReactMontage();

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	
	void UpdateHUD_Health();

	//�ж�������Ƿ���Ч����ʼ��HUD
	void PollInitialize();
private:

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverHeadWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* CombatComponent;

	//��ֵͨ�����縴��ʱ�����ú���OnRep_OverlappingWeapon
	//ע�⣺��������Ϊ����ͨ�����縴�Ƹ������ݣ����Բ��ᴥ���������ڴ˴�����Ϊ����˲�����ֿͻ��˴����� ��ʾwidget
	//�������ʾPickupWidget��ʵ��
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server,Reliable)//�ɿ�����֤�յ����
	void ServerEquipButtonPressed();

	float AimOffset_Yaw;
	float AimOffset_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);//��ԭ��ת����90��ʱ����ת����
	float InterpAimOffset_Yaw;

	UPROPERTY(EditAnywhere,Category="Combat")
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* ReloadMontage;

	void HideCamerIfCharacterClose();

	UPROPERTY(EditAnywhere)
	float CameraThresHold = 200.0f;//������������С��ĳֵ����������

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotationThisFrame;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CaculateSpeed();


	/**
	 *  �����Ϣ
	 */
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.0f;

	UPROPERTY(ReplicatedUsing= OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health;

	UFUNCTION()
	void OnRep_Health();

	UPROPERTY()
	class AMainPlayerController* MainPlyerController;

	bool bElimmed = false;

	UPROPERTY()
	class AMainPlayerState* MainPlayerState;

public:

	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();

	FORCEINLINE float GetAimOffset_Yaw() const { return AimOffset_Yaw; }
	FORCEINLINE float GetAimOffset_Pitch() const { return AimOffset_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; };
	FORCEINLINE UCameraComponent* GetFollowCamera()const { return FollowCamera; }
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	FORCEINLINE bool IsElimmed()const { return bElimmed; }
	FORCEINLINE float GetHealth()const { return Health; }
	FORCEINLINE float GetMaxHealth()const { return MaxHealth; }
	FORCEINLINE ECombatState GetCombateState() const { return CombatComponent == nullptr ? ECombatState::ECS_MAX : CombatComponent->CombatState; }
	FORCEINLINE UCombatComponent* GetCombatComponent()const { return CombatComponent; }
	FORCEINLINE bool GetDisableGameplay()const { return bDisableGamePlay; }

	AWeapon* GetEquippedWeapon();

	FVector GetHitTarget()const;

	void Destroy();

};

