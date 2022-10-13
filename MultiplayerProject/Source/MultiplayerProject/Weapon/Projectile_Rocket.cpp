// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile_Rocket.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"

AProjectile_Rocket::AProjectile_Rocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectile_Rocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	APawn* FiringPawn = GetInstigator();//��ȡ��ö������ķ�����
	if (FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			//��˥���ķ�Χ�˺�
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,
				Damage,
				10.0f,//˥�������Сֵ
				GetActorLocation(),//ԭ��
				200.0f,//��Χ�ڲ�˥��
				500.0f,//˥����0�İ뾶
				1.0f,//DamageFalloff  ˥��������ָ����
				UDamageType::StaticClass(),
				TArray<AActor*>(),//Ҫ���Ե�Actor
				this,//DamageCauser
				FiringController
			);
		}
	}

	//���� OnHit ֻ�� Destroyed
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}