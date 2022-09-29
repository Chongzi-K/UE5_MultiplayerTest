// Fill out your copyright notice in the Description page of Project Settings.


#include "MainAnimInstance.h"
#include "MainCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MultiplayerProject/Weapon/Weapon.h"

void UMainAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	MainCharacter = Cast<AMainCharacter>(TryGetPawnOwner());
}

void UMainAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (MainCharacter == nullptr)
	{
		MainCharacter = Cast<AMainCharacter>(TryGetPawnOwner());
	}
	if (MainCharacter == nullptr) return;

	FVector Velocity = MainCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = MainCharacter->GetCharacterMovement()->IsFalling();
	//正在加速？
	bIsAccelerating = MainCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = MainCharacter->IsWeaponEquipped();
	EquippedWeapon = MainCharacter->GetEquippedWeapon();
	bIsCrouched = MainCharacter->bIsCrouched;
	bAiming = MainCharacter->IsAiming();


	//OffSet,Yaw 
	FRotator AimRotation = MainCharacter->GetBaseAimRotation();//Velocity 和 BaseAimRotation 在客户端上也会实时更新
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(MainCharacter->GetVelocity());
	//UE_LOG(LogTemp, Warning, TEXT("AimRotaion.Yaw :"),AimRotation.Yaw);
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotaion = FMath::RInterpTo(DeltaRotaion, DeltaRot, DeltaTime, 6.0f);
	YawOffset = DeltaRotaion.Yaw;

	CharacterRotationLastFrame = CharacterRotationThisFrame;
	CharacterRotationThisFrame = MainCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotationThisFrame, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean,Target,DeltaTime,6.0f);
	Lean = FMath::Clamp(Interp, -90.0f, 90.0f);

	AimOffset_Yaw = MainCharacter->GetAimOffset_Yaw();
	AimOffset_Pitch = MainCharacter->GetAimOffset_Pitch();

	if (bWeaponEquipped&&EquippedWeapon&&EquippedWeapon->GetWeaponMesh()&&MainCharacter->GetMesh())
	{
		//将左手骨骼附着到武器骨骼的插槽中
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"),ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		MainCharacter->GetMesh()->TransformFromBoneSpace(FName("hand_r"),LeftHandTransform.GetLocation(),FRotator::ZeroRotator,OutPosition,OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
	}
}