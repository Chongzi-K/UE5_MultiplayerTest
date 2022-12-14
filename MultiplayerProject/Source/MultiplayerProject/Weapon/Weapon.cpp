// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "MultiplayerProject/MainCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Casing.h"
#include "Engine/SkeletalMeshSocket.h"
#include "MultiplayerProject/MainPlayerController/MainPlayerController.h"

// Sets default values
AWeapon::AWeapon()
{ 
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;//开启复制
	SetReplicateMovement(true);//运动也复制，修复玩家在空中死亡时武器掉落位置各端显示不统一的问题

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);

	//WeaponMesh->SetSimulatePhysics(true);
	//WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);//对所有通道碰撞
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);//忽略pawn通道
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//禁用Mesh的碰撞



	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	//设置服务端/客户端 拾取检测球体 的碰撞响应为忽略，在beginplay中再分情况判断是否开启响应，目的是只在服务端上产生响应
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//在服务端/客户端上禁用 拾取检测球体 的碰撞

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	//if(HasAuthority()) 效果相同，都是在服务端上才true
	if (GetLocalRole() == ENetRole::ROLE_Authority)
	{
		//该pawn在服务器上，则开启响应
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		//在服务器上才绑定重叠委托
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
	
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME(AWeapon, CurrentAmmo);
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AMainCharacter* MainCharacter = Cast<AMainCharacter>(OtherActor);

	if (MainCharacter)
	{
		//只在服务器上产生的响应事件，所以此时只有服务器才能看到，需要复制
		//复制通过 AMainCharacter::GetLifetimeReplicatedProps 中 DOREPLIFETIME_CONDITION (条件触发复制) + 	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon) (客户端接受复制时触发函数) 实现
		MainCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AMainCharacter* MainCharacter = Cast<AMainCharacter>(OtherActor);
	if (MainCharacter)
	{
	//	if (WeaponType == EWeaponType::EWT_Flag && BlasterCharacter->GetTeam() == Team) return;
	//	if (BlasterCharacter->IsHoldingTheFlag()) return;
	
		MainCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::OnRep_CurrentAmmo()
{
	SetHUDAmmo();
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr)//与 Dropped 双保险
	{
		MainOwnerCharacter = nullptr;
		MainOwnerController = nullptr;
	}
	else
	{
		SetHUDAmmo();
	}

}

void AWeapon::SetHUDAmmo()
{
	MainOwnerCharacter = MainOwnerCharacter == nullptr ? Cast<AMainCharacter>(GetOwner()) : MainOwnerCharacter;
	if (MainOwnerCharacter)
	{
		MainOwnerController = MainOwnerController == nullptr ? Cast<AMainPlayerController>(MainOwnerCharacter->Controller) : MainOwnerController;
		if (MainOwnerController)
		{
			MainOwnerController->SetHUDWeaponAmmo(CurrentAmmo);
		}
	}
}

void AWeapon::AddAmmo(int32 AmmoAmountToAdd)
{
	CurrentAmmo = FMath::Clamp(CurrentAmmo - AmmoAmountToAdd, 0, MagCapacity);
	SetHUDAmmo();
}

void AWeapon::SpendRound()
{
	CurrentAmmo = FMath::Clamp(CurrentAmmo - 1, 0, MagCapacity);//触发OnRep_OnRep_CurrentAmmo
	SetHUDAmmo();
}

void AWeapon::SetWeaponState(EWeaponState StateToSet)
{
	WeaponState = StateToSet;//客户端调用AWeapon::OnRep_WeaponState()
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (WeaponType == EWeaponType::EWT_SubmachineGun)
		{
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			WeaponMesh->SetEnableGravity(true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		}
		break;

	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		break;
	}
}

void AWeapon::OnRep_WeaponState()
//客户端接受WeaponState复制更新时调用
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;

	case EWeaponState::EWS_Dropped:
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation,false);
    }
	if (CasingClass)
	{
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<ACasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
					);
			}

		}
	}
	SpendRound();//触发弹药减少
}

void AWeapon::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);//調用對應的回調函數
	FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, true);
	
	//玩家死亡時分離武器
	WeaponMesh->DetachFromComponent(DetachmentRules);
	SetOwner(nullptr);
	MainOwnerCharacter = nullptr;
	MainOwnerController = nullptr;
}