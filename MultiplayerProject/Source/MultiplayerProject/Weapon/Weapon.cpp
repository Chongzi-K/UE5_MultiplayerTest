// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "MultiplayerProject/MainCharacter.h"

// Sets default values
AWeapon::AWeapon()
{ 
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;//��������

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);//������ͨ����ײ
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);//����pawnͨ��
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);//����Mesh����ײ

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	//���÷����/�ͻ��� ʰȡ������� ����ײ��ӦΪ���ԣ���beginplay���ٷ�����ж��Ƿ�����Ӧ��Ŀ����ֻ�ڷ�����ϲ�����Ӧ
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);//�ڷ����/�ͻ����Ͻ��� ʰȡ������� ����ײ

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	//if(HasAuthority()) Ч����ͬ�������ڷ�����ϲ�true
	if (GetLocalRole() == ENetRole::ROLE_Authority)
	{
		//��pawn�ڷ������ϣ�������Ӧ
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		//�ڷ������ϲŰ��ص�ί��
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
		//ֻ�ڷ������ϲ�������Ӧ�¼������Դ�ʱֻ�з��������ܿ�������Ҫ����
		//����ͨ�� AMainCharacter::GetLifetimeReplicatedProps �� DOREPLIFETIME_CONDITION (������������) + 	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon) (�ͻ��˽��ܸ���ʱ��������) ʵ��
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

