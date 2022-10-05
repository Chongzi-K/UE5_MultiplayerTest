// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "MultiplayerProject/Weapon/Weapon.h"
#include "MultiplayerProject/Components_Pawn/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MainAnimInstance.h"
#include "MultiplayerProject/MultiplayerProject.h"




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

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);//开启复制

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 850.0f);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera,ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	NetUpdateFrequency = 60.f;
	MinNetUpdateFrequency = 30.f;


}

void AMainCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CombatComponent)
	{
		CombatComponent->MainCharacter = this;
	}
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

	AimOffset(DeltaTime);

	HideCamerIfCharacterClose();


}

void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AMainCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AMainCharacter::LookUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMainCharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AMainCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMainCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AMainCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AMainCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMainCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AMainCharacter::FireButtonReleased);

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

void AMainCharacter::EquipButtonPressed()
{
	if (CombatComponent)
	{
		if (HasAuthority())
		{
			CombatComponent->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			//客户端需要通过RPC，在客户端上调用服务器函数实现
			ServerEquipButtonPressed();
		}

	}
}

void AMainCharacter::CrouchButtonPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		//需要在charactermovement中设置蹲伏半高60，移动速度100；
		//CharacterMovement->SetCrouchedHalfHeight(60.0f);
		Crouch();//继承自character类的bIsCrouch在这个函数里会被改变并复制
	}
}

void AMainCharacter::AimButtonPressed()
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(true);
	}
}

void AMainCharacter::AimButtonReleased()
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(false);
	}
}

void AMainCharacter::ServerEquipButtonPressed_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}

void AMainCharacter::MulticastHit_Implementation()
{
	PlayHitReactMontage();
}

void AMainCharacter::HideCamerIfCharacterClose()
{
	if (!IsLocallyControlled()) { return; }
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThresHold)
	{
		GetMesh()->SetVisibility(false);
		if (CombatComponent && CombatComponent->EquippedWeapon&&CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			//仅 owner 不可见
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			//仅 owner 不可见
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
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

bool AMainCharacter::IsWeaponEquipped()
{
	return(CombatComponent && CombatComponent->EquippedWeapon);
}

bool AMainCharacter::IsAiming()
{
	return(CombatComponent && CombatComponent->bAiming);
}

void AMainCharacter::AimOffset(float DeltaTime)
{
	if (CombatComponent && CombatComponent->EquippedWeapon == nullptr) { return; }
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.0f && !bIsInAir)//站立，不跳
	{
		FRotator CurrentAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AimOffset_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAimOffset_Yaw = AimOffset_Yaw;
		}
		bUseControllerRotationYaw = false;
		TurnInPlace(DeltaTime);//解决转身角度突变问题
	}
	if (Speed > 0.0f || bIsInAir)//跑或跳
	{
		StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw,0.0f);
		AimOffset_Yaw = 0.0f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	AimOffset_Pitch = GetBaseAimRotation().Pitch;
	//CharacterMovementComponent->GetPackedAngles()中 把 Rotation 压缩到 5 bytes
    // CompressAxisToShort(float Angle){return FMath::RoundToInt(Angle*65536.f/360.f)&0xFFFf;}
    //Rotation 发送时从 float 压缩成了 int ，接收时恢复为 float
    //负值在压缩解压后会变为正值
	if (AimOffset_Pitch > 90.0f && !IsLocallyControlled())
	{
		//把 Pitch 从 [270，360）映射到[-90，0）
		FVector2D InRange(270.0f, 360.0f);
		FVector2D OutRange(-90.0f, 0.0f);
		AimOffset_Yaw = FMath::GetMappedRangeValueClamped(InRange, OutRange,AimOffset_Pitch);
	}
}

void AMainCharacter::TurnInPlace(float DeltaTime)
{
	if (AimOffset_Yaw>90.0f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if(AimOffset_Yaw<-90.0f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAimOffset_Yaw = FMath::FInterpTo(InterpAimOffset_Yaw, 0.0f, DeltaTime, 4.0f);
		AimOffset_Yaw = InterpAimOffset_Yaw;
		if (FMath::Abs(AimOffset_Yaw) < 15.0f)
		{
			//转动 root bone 到一定角度就停止旋转
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		}
	}
}

AWeapon* AMainCharacter::GetEquippedWeapon()
{
	if (CombatComponent == nullptr) { return nullptr; }
	return CombatComponent->EquippedWeapon;
}

void AMainCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void AMainCharacter::FireButtonPressed()
{
	if (CombatComponent)
	{
		CombatComponent->FireButtonPressed(true);
	}
}

void AMainCharacter::FireButtonReleased()
{
	if (CombatComponent)
	{
		CombatComponent->FireButtonPressed(false);
	}
}

void AMainCharacter::PlayFireMontage(bool bAiming)
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) { return; }

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AMainCharacter::PlayHitReactMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) { return; }

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

FVector AMainCharacter::GetHitTarget()const
{
	if (CombatComponent == nullptr) { return FVector(); }
	return CombatComponent->HitTarget;
}
