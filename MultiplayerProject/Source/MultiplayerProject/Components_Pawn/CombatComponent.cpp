// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "MultiplayerProject/Weapon/Weapon.h"
#include "MultiplayerProject/MainCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	BaseWalkSpeed = 600.0f;
	AimWalkSpeed = 450.0f;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (MainCharacter)
	{
		MainCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
	
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (MainCharacter==nullptr||WeaponToEquip==nullptr) { return; }

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* RightHandSocket = MainCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (RightHandSocket)
	{
		RightHandSocket->AttachActor(EquippedWeapon, MainCharacter->GetMesh());
	}
	EquippedWeapon->SetOwner(MainCharacter);
	//SetOwner�Ĳ���Owner�Ѿ������˸��ƣ����һ��к��� OnRep_Owner()
	MainCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	MainCharacter->bUseControllerRotationYaw = true;


}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	//Character ����
	//��ǰ�޸ģ�ʹ�ÿͻ��˿����� bAiming�ڷ������ϱ��޸�֮ǰ�Ϳ��� bAiming ���޸ĵĽ������������
	bAiming = bIsAiming;
	//if (!MainCharacter->HasAuthority())
	//{
	//	//������Ƿ������������ RPC���ڿͻ����ϴ��� �޸ķ������ж�Ӧʵ�� bAiming �ĺ���
	//	ServerSetAiming(bIsAiming);
	//}
	//RPC�����ڷ������϶��Լ����ã����Բ����ж�
	ServerSetAiming(bIsAiming);

	if (MainCharacter)
	{
		MainCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (MainCharacter)
	{
		MainCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon&&MainCharacter)
	{
		//���������ֵ�޷�����ȷ���Ƶ���һ���ͻ��˵�����
		MainCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
		MainCharacter->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed)
	{
		ServerFire();
	}

}

void UCombatComponent::MulticastFire_Implementation()
{
	if (EquippedWeapon == nullptr) { return; }

	if (MainCharacter && bFireButtonPressed)
	{
		MainCharacter->PlayFireMontage(bAiming);
		EquippedWeapon->Fire();
	}
}

void UCombatComponent::ServerFire_Implementation()
{
	MulticastFire();
}
