// Fill out your copyright notice in the Description page of Project Settings.


#include "MainAnimInstance.h"
#include "MainCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

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
	bIsCrouched = MainCharacter->bIsCrouched;
	bAiming = MainCharacter->IsAiming();


}