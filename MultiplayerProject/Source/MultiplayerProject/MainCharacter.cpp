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

	//绑定后，当服务器上这个值发生改变，才会复制到所有客户端上；而不是每帧每tick去修改
	DOREPLIFETIME_CONDITION(AMainCharacter, OverlappingWeapon, COND_OwnerOnly);//COND_OwnerOnly：只复制给实例所在的客户端
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
		//获取的是 controller 的 rotation ，而不是 character 的
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		//RotationMatrix 改变向量方向，但不改变大小
		//通过 YawRotation 获得一个 控制器向前的FVector
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void AMainCharacter::MoveRight(float Value)
{

	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		//通过 YawRotation 获得一个 控制器向右的FVector
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

//该函数只会在服务器被调用，因为只有服务器才开启了触发重叠事件
void AMainCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	//该方法在AWeapon::OnSphereOverlap调用

	if (OverlappingWeapon)
	{
		//先设置为false，如果有重叠武器则  服务端在下面开启,客户端在OnRep_OverlappingWeapon开启，以实现服务端结束重叠时关闭PickupWidget的功能
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;

	if (IsLocallyControlled())//如果是服务端
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

//使用 LastWeapon 保存在 Weapon 更新前的最后一次值，以便在结束重叠事件时关闭上一个Weapon的 PickupWidget
//但是该函数不会在服务端被调用（网络复制触发），所以服务端关闭PickupWidget需要另外实现
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