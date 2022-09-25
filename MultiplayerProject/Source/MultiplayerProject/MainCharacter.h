// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MainCharacter.generated.h"


UCLASS()
class MULTIPLAYERPROJECT_API AMainCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	AMainCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:

	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);

private:

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverHeadWidget;

	//��ֵͨ�����縴��ʱ�����ú���OnRep_OverlappingWeapon
	//ע�⣺��������Ϊ����ͨ�����縴�Ƹ������ݣ����Բ��ᴥ���������ڴ˴�����Ϊ����˲�����ֿͻ��˴����� ��ʾwidget
	//�������ʾPickupWidget��ʵ��
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

public:

	void SetOverlappingWeapon(AWeapon* Weapon);

};
