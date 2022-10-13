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
	APawn* FiringPawn = GetInstigator();//获取这枚火箭弹的发起者
	if (FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			//有衰减的范围伤害
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,
				Damage,
				10.0f,//衰减后的最小值
				GetActorLocation(),//原点
				200.0f,//范围内不衰减
				500.0f,//衰减到0的半径
				1.0f,//DamageFalloff  衰减函数，指数的
				UDamageType::StaticClass(),
				TArray<AActor*>(),//要忽略的Actor
				this,//DamageCauser
				FiringController
			);
		}
	}

	//父类 OnHit 只有 Destroyed
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}