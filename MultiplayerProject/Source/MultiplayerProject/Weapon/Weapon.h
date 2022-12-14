// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"


UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")//常用于检查枚举值的数量
};


UCLASS()
class MULTIPLAYERPROJECT_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();

	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void ShowPickupWidget(bool bShowWidget);

	virtual void Fire(const FVector& HitTarget);

	void Dropped();

	virtual void OnRep_Owner()override;

	void SetHUDAmmo();

	void AddAmmo(int32 AmmoAmountToAdd);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex,
			bool bFromSweep,
			const FHitResult& SweepResult
		);

	UFUNCTION()
	void OnSphereEndOverlap(
			UPrimitiveComponent* OverlappedComponent,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			int32 OtherBodyIndex
		);

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	UPROPERTY(ReplicatedUsing= OnRep_WeaponState,VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon ProPerties")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_CurrentAmmo)
	int32 CurrentAmmo = 15;//当前子弹数

	UFUNCTION()
	void OnRep_CurrentAmmo();

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;//弹匣容量

	void SpendRound();//开枪后更新子弹数和使用者的HUD

	UPROPERTY()
	class AMainCharacter* MainOwnerCharacter;

	UPROPERTY()
	class AMainPlayerController* MainOwnerController;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

public:

	void SetWeaponState(EWeaponState  StateToSet);
	FORCEINLINE USphereComponent* GetAreaSphere()const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV()const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed()const { return ZoomedInterpSpeed; }
	FORCEINLINE bool IsAmmoExhausted()const { return CurrentAmmo <= 0; }
	FORCEINLINE EWeaponType GetWeaponType()const { return WeaponType; }
	FORCEINLINE int32 GetCurrentAmmo()const { return CurrentAmmo; }
	FORCEINLINE int32 GetMagCapacity()const { return MagCapacity; }

public:
	//
	//武器准星材质，提供给MainHUD
	//
	UPROPERTY(EditAnywhere,Category="Crosshairs")
	class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D* CrosshairsBottom;

	/**
	*Aiming 时拉进拉远视角
	*/
	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.0f;

	UPROPERTY(EditAnywhere)
	float ZoomedInterpSpeed = 20.0f;

	/**
	 * 武器连射
	 */

	UPROPERTY(EditAnywhere, Category = "Combat")
	float FireDelay = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bAutomaticWeapon = true;


};
