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
	//在其中确定 哪些变量在发生变化时复制到客户端上
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents()override;//初始化组件
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayElimMontage();
	virtual void OnRep_ReplicatedMovement()override;

	//被淘汰
	void Elim();
	UFUNCTION(NetMulticast,Reliable)
	void MulticastElim();

	FTimerHandle ElimTimer;

	void ElimTimerFinish();

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.0f;

	UPROPERTY(Replicated)
	bool bDisableGamePlay = false;//对局结束后禁用输入，只允许鼠标旋转摄像头

	void FireButtonPressed();//需要在

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

	//判断相关类是否有效并初始化HUD
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

	//当值通过网络复制时，调用函數OnRep_OverlappingWeapon
	//注意：服务器因为不是通过网络复制更新数据，所以不会触发函数，在此处体现为服务端不会出现客户端触发的 显示widget
	//服务端显示PickupWidget的实现
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UFUNCTION(Server,Reliable)//可靠，保证收到结果
	void ServerEquipButtonPressed();

	float AimOffset_Yaw;
	float AimOffset_Pitch;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);//在原地转身超过90度时触发转身动画
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
	float CameraThresHold = 200.0f;//相机与人物距离小于某值则隐藏人物

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotationThisFrame;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	float CaculateSpeed();


	/**
	 *  玩家信息
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

