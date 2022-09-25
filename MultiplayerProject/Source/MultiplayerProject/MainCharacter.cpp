// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "MultiplayerProject/Weapon/Weapon.h"

AMainCharacter::AMainCharacter()
{
 	
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHeadWidget"));
	OverHeadWidget->SetupAttachment(RootComponent);
	OverHeadWidget->SetDrawSize(FVector2D(500.f,500.f));


}

void AMainCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//�󶨺󣬵������������ֵ�����ı䣬�ŻḴ�Ƶ����пͻ����ϣ�������ÿ֡ÿtickȥ�޸�
	DOREPLIFETIME_CONDITION(AMainCharacter, OverlappingWeapon, COND_OwnerOnly);//COND_OwnerOnly��ֻ���Ƹ�ʵ�����ڵĿͻ���
	//DOREPLIFETIME(AMainCharacter, Health);
	//DOREPLIFETIME(AMainCharacter, Shield);
	//DOREPLIFETIME(AMainCharacter, bDisableGameplay);
} 

void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AMainCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMainCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMainCharacter::Jump);

}

void AMainCharacter::MoveForward(float Value)
{

	if (Controller != nullptr && Value != 0.f)
	{
		//��ȡ���� controller �� rotation �������� character ��
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		//RotationMatrix �ı��������򣬵����ı��С
		//ͨ�� YawRotation ���һ�� ��������ǰ��FVector
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void AMainCharacter::MoveRight(float Value)
{

	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		//ͨ�� YawRotation ���һ�� ���������ҵ�FVector
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void AMainCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AMainCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

//�ú���ֻ���ڷ����������ã���Ϊֻ�з������ſ����˴����ص��¼�
void AMainCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	//�÷�����AWeapon::OnSphereOverlap����

	if (OverlappingWeapon)
	{
		//������Ϊfalse��������ص�������  ����������濪��,�ͻ�����OnRep_OverlappingWeapon��������ʵ�ַ���˽����ص�ʱ�ر�PickupWidget�Ĺ���
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;

	if (IsLocallyControlled())//����Ƿ����
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

//ʹ�� LastWeapon ������ Weapon ����ǰ�����һ��ֵ���Ա��ڽ����ص��¼�ʱ�ر���һ��Weapon�� PickupWidget
//���Ǹú��������ڷ���˱����ã����縴�ƴ����������Է���˹ر�PickupWidget��Ҫ����ʵ��
void AMainCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}