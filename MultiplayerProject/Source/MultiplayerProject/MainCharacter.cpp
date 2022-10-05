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
#include "MultiplayerProject/MainPlayerController/MainPlayerController.h"
#include "MultiplayerProject/GameMode/MainGameMode.h"
#include "TimerManager.h"


AMainCharacter::AMainCharacter()
{
 	
	PrimaryActorTick.bCanEverTick = true;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

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
	CombatComponent->SetIsReplicated(true);//��������

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

	//�󶨺󣬵������������ֵ�����ı䣬�ŻḴ�Ƶ����пͻ����ϣ�������ÿ֡ÿtickȥ�޸�
	DOREPLIFETIME_CONDITION(AMainCharacter, OverlappingWeapon, COND_OwnerOnly);//COND_OwnerOnly��ֻ���Ƹ�ʵ�����ڵĿͻ���
	DOREPLIFETIME(AMainCharacter, Health);
	//DOREPLIFETIME(AMainCharacter, Shield);
	//DOREPLIFETIME(AMainCharacter, bDisableGameplay);
} 

void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateHUD_Health();

	//ֻ�ڷ����������˺���Ӧ
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AMainCharacter::ReceiveDamage);
	}
	
}

void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy&&IsLocallyControlled())//ö��ֵ���ԱȽϣ�����Ĵ�
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)//0.25sδ�����˶�������һ�θ�����Ӧ
		{
			OnRep_ReplicatedMovement();
		}
		CaculateAimOffset_Pitch();
	}
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
			//�ͻ�����Ҫͨ��RPC���ڿͻ����ϵ��÷���������ʵ��
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
		//��Ҫ��charactermovement�����ö׷����60���ƶ��ٶ�100��
		//CharacterMovement->SetCrouchedHalfHeight(60.0f);
		Crouch();//�̳���character���bIsCrouch�����������ᱻ�ı䲢����
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

void AMainCharacter::HideCamerIfCharacterClose()
{
	if (!IsLocallyControlled()) { return; }
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThresHold)
	{
		GetMesh()->SetVisibility(false);
		if (CombatComponent && CombatComponent->EquippedWeapon&&CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			//�� owner ���ɼ�
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			//�� owner ���ɼ�
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void AMainCharacter::OnRep_Health()
{
	PlayHitReactMontage();
	UpdateHUD_Health();
}

void AMainCharacter::UpdateHUD_Health()
{
	//������ʹ��cast���裬Ҫ��ε���ʱʹ����Ԫ���������cast����
//MainPlyerController = Cast<AMainPlayerController>(Controller);
	MainPlyerController = MainPlyerController == nullptr ? Cast<AMainPlayerController>(Controller) : MainPlyerController;
	if (MainPlyerController)
	{
		MainPlyerController->SetHUDHealth(Health, MaxHealth);
	}
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

bool AMainCharacter::IsWeaponEquipped()
{
	return(CombatComponent && CombatComponent->EquippedWeapon);
}

bool AMainCharacter::IsAiming()
{
	return(CombatComponent && CombatComponent->bAiming);
}

float AMainCharacter::CaculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void AMainCharacter::AimOffset(float DeltaTime)
{
	if (CombatComponent && CombatComponent->EquippedWeapon == nullptr) { return; }
	bool bIsInAir = GetCharacterMovement()->IsFalling();
	float Speed = CaculateSpeed();
	if (Speed == 0.0f && !bIsInAir)//վ��������
	{
		bRotateRootBone = true;
		FRotator CurrentAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw, 0.0f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AimOffset_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAimOffset_Yaw = AimOffset_Yaw;
		}
		bUseControllerRotationYaw = false;
		TurnInPlace(DeltaTime);//���ת��Ƕ�ͻ������
	}
	if (Speed > 0.0f || bIsInAir)//�ܻ���
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.0f, GetBaseAimRotation().Yaw,0.0f);
		AimOffset_Yaw = 0.0f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CaculateAimOffset_Pitch();

}

void AMainCharacter::SimulateProxiesTurn()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) { return; }

	bRotateRootBone = false;

	float Speed = CaculateSpeed();	
	if (Speed > 0.0f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotationThisFrame;
	ProxyRotationThisFrame = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotationThisFrame,ProxyRotationLastFrame).Yaw;
	if (FMath::Abs(ProxyYaw)>TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	else
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
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
			//ת�� root bone ��һ���ǶȾ�ֹͣ��ת
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

void AMainCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void AMainCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();
	SimulateProxiesTurn();
	TimeSinceLastMovementReplication = 0.0f;
}

void AMainCharacter::Elim()
{
	//�� GameMode ����
	MulticastElim();
	GetWorldTimerManager().SetTimer(ElimTimer, this, &AMainCharacter::ElimTimerFinish, ElimDelay);
}

void AMainCharacter::ElimTimerFinish()
{
	AMainGameMode* MainGameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();
	if (MainGameMode)
	{
		MainGameMode->RequestRespawn(this, Controller);
	}
}

void AMainCharacter::MulticastElim_Implementation()
{
	bElimmed = true;
	PlayElimMontage();
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

void AMainCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController,AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);
	UpdateHUD_Health();
	//�������ˣ��ͻ�����Health�ĸ�����Ӧ��������
	PlayHitReactMontage();
	if (Health == 0.0f)
	{
		//�ڷ�����ϲ��ܻ�ȡ��GameMode,�ͻ��˻᷵�ؿ�ָ��
		AMainGameMode* MainGameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();
		if (MainGameMode)
		{
			MainPlyerController = MainPlyerController == nullptr ? Cast<AMainPlayerController>(Controller) : MainPlyerController;
			AMainPlayerController* AttackerController = Cast<AMainPlayerController>(InstigatorController);
			MainGameMode->PlayerEliminated(this,MainPlyerController, AttackerController);
		}
	}

}

FVector AMainCharacter::GetHitTarget()const
{
	if (CombatComponent == nullptr) { return FVector(); }
	return CombatComponent->HitTarget;
}

void AMainCharacter::CaculateAimOffset_Pitch()
{
		AimOffset_Pitch = GetBaseAimRotation().Pitch;
		//CharacterMovementComponent->GetPackedAngles()�� �� Rotation ѹ���� 5 bytes
			// CompressAxisToShort(float Angle){return FMath::RoundToInt(Angle*65536.f/360.f)&0xFFFf;}
			//Rotation ����ʱ�� float ѹ������ int ������ʱ�ָ�Ϊ float
			//��ֵ��ѹ����ѹ����Ϊ��ֵ
		if (AimOffset_Pitch > 90.0f && !IsLocallyControlled())
		{
			//�� Pitch �� [270��360��ӳ�䵽[-90��0��
			FVector2D InRange(270.0f, 360.0f);
			FVector2D OutRange(-90.0f, 0.0f);
			AimOffset_Yaw = FMath::GetMappedRangeValueClamped(InRange, OutRange, AimOffset_Pitch);
		}
}