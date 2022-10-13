// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "MultiplayerProject/Weapon/Weapon.h"
#include "MultiplayerProject/MainCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "MultiplayerProject/MainPlayerController/MainPlayerController.h"
#include "Camera/CameraComponent.h"
#include "MultiplayerProject/MainCharacter.h"
#include "TimerManager.h"




UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.0f;
	AimWalkSpeed = 450.0f;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (MainCharacter)
	{
		MainCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		if (MainCharacter->GetFollowCamera())
		{
			DefaultFOV = MainCharacter->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
		if (MainCharacter->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}
	
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MainCharacter && MainCharacter->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}


}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo,COND_OwnerOnly);//�������ͻ�����˵û���壬����ֻ���Ƶ�ʵ����Ӧ�Ŀͻ���
	DOREPLIFETIME(UCombatComponent, CombatState);

}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (MainCharacter==nullptr||WeaponToEquip==nullptr) { return; }

	if (EquippedWeapon)//����װ������
	{
		EquippedWeapon->Dropped();
	}

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* RightHandSocket = MainCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (RightHandSocket)
	{
		RightHandSocket->AttachActor(EquippedWeapon, MainCharacter->GetMesh());
	}
	EquippedWeapon->SetOwner(MainCharacter);
	EquippedWeapon->SetHUDAmmo();

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))//Key=�������ͣ���ѯint32 �ӵ�����
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<AMainPlayerController>(MainCharacter->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
    }

	if (EquippedWeapon->IsAmmoExhausted())//��������ʱ����ҩ�þ��Զ� R
	{
		Reload();
	}

	//SetOwner�Ĳ���Owner�Ѿ������˸��ƣ����һ��к��� OnRep_Owner()
	MainCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
	MainCharacter->bUseControllerRotationYaw = true;


}

void UCombatComponent::Reload()
{
	if (CarriedAmmo > 0&&CombatState!=ECombatState::ECS_Reloading)//AmmoMap�ڿͻ��ˣ��пͻ����ж��ӵ��Ƿ��㹻����
	{
		SeverReload();
	}
}

void UCombatComponent::FinishReloading()
{
	if (MainCharacter == nullptr) { return; }
	if (MainCharacter->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoAmount();
	}
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::UpdateAmmoAmount()
{
	if (MainCharacter == nullptr || EquippedWeapon == nullptr) { return; }
	int32 ReloadAmount = AmountToReload();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))//�����Լ�Я���ĵ�ҩ
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	Controller = Controller == nullptr ? Cast<AMainPlayerController>(MainCharacter->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(-ReloadAmount);
}

void UCombatComponent::SeverReload_Implementation()//���ͻ��˵��õķ������Ӧ����-����˴�����
{
	if (MainCharacter == nullptr || EquippedWeapon == nullptr) { return; }

	UpdateAmmoAmount();

	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
}

void UCombatComponent::OnRep_CombatState()//�ͻ����յ����ƵĴ�����
{
	switch (CombatState)
	{
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_ThrowingGrenade:
		break;
	case ECombatState::ECS_SwappingWeapons:
		break;
	case ECombatState::ECS_MAX:
		break;
	default:
		break;
	}
}

void UCombatComponent::HandleReload()
{
	MainCharacter->PlayReloadMontage();
}

int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) { return 0; }
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetCurrentAmmo();
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmmoCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmmoCarried);
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	
	return 0;
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
		Fire();
	}
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || MainCharacter == nullptr) { return; }
	MainCharacter->GetWorldTimerManager().SetTimer(FireTimer, this, &UCombatComponent::FireTimerFinished, EquippedWeapon->FireDelay);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) { return; }
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomaticWeapon)
	{
		Fire();
	} 
	if (EquippedWeapon->IsAmmoExhausted())//��ҩ�þ��Զ� R
	{
		Reload();
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	FVector2D CrosshairLocation = FVector2D(ViewportSize.X / 2.0f, ViewportSize.Y / 2.0f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	bool bScreenToWorldSuccess = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorldSuccess)
	{
		FVector Start = CrosshairWorldPosition;

		if (MainCharacter)
		{
			float DistanceToCharacter = (MainCharacter->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.0f);
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End, ECollisionChannel::ECC_Visibility
		);
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White;
		}

	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (MainCharacter == nullptr && MainCharacter->Controller == nullptr) { return; }
	Controller = Controller == nullptr ? Cast<AMainPlayerController>(MainCharacter->Controller) : Controller;
	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<AMainHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			//FHUDPackage HUDPackage;
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
			}
			//����׼��ɢ���̶�
			FVector2D WalkSpeedRange(0.0f, MainCharacter->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultipierRange(0.0f, 1.0f);
			FVector Velocity = MainCharacter->GetVelocity();
			Velocity.Z = 0.0f;
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultipierRange,Velocity.Size());
			
			if (MainCharacter->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.0f, DeltaTime, 30.0f);
			}
			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.6f, DeltaTime, 30.0f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.0f, DeltaTime, 30.0f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.0f, DeltaTime, 40.0f);
			
			HUDPackage.CrosshairSpread = 
				0.5f +
				CrosshairVelocityFactor +
				CrosshairInAirFactor - 
				CrosshairAimFactor + 
				CrosshairShootingFactor;
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::Fire()
{
	if (CanFire())
	{
		bCanFire = false;
		ServerFire(HitTarget);
		if (EquippedWeapon)
		{
			CrosshairShootingFactor = 0.8f;
		}
		StartFireTimer();
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) { return; }

	if (MainCharacter && bFireButtonPressed&&CombatState==ECombatState::ECS_Unoccupied)
	{
		MainCharacter->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) { return; }
	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	if (MainCharacter && MainCharacter->GetFollowCamera())
	{
		MainCharacter->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) { return false; }
	return !EquippedWeapon->IsAmmoExhausted() && !bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	//�ͻ���ҲҪ��һ��ͼ�������ǿ�����
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))//Key=�������ͣ���ѯint32 �ӵ�����
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
}



