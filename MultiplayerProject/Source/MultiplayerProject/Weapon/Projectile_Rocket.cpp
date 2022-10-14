// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile_Rocket.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"

AProjectile_Rocket::AProjectile_Rocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AProjectile_Rocket::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile_Rocket::OnHit);
	}

	if (RocketTrailSystem)
	{
		RocketTrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			RocketTrailSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false// bAutoDestroy ，不需要跟随 actor 自动销毁，因为要实现爆炸后烟雾轨迹依然存在的效果
			);
	}

	if (ProjectileFlyingLoop && LoopingSoundAttenuation)
	{
		//火箭弹飞行音效+衰减
		ProjectileFlyingLoopAudioComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileFlyingLoop,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.0f,
			1.0f,
			0.0f,
			LoopingSoundAttenuation,
			(USoundConcurrency*)nullptr,
			false
			);
	}
}

void AProjectile_Rocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	APawn* FiringPawn = GetInstigator();//获取这枚火箭弹的发起者
	if (FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController && HasAuthority())
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

	GetWorldTimerManager().SetTimer(DestroyRocketTrailHandle, this, &AProjectile_Rocket::DestroyRocketTrailWhenTimerFinished, DestroyTime);

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if (RocketMesh)
	{
		RocketMesh->SetVisibility(false);
	}
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (RocketTrailSystemComponent && RocketTrailSystemComponent->GetSystemInstance())
	{
		RocketTrailSystemComponent->GetSystemInstance()->Deactivate();
	}
	if (ProjectileFlyingLoopAudioComponent && ProjectileFlyingLoopAudioComponent->IsPlaying())
	{
		ProjectileFlyingLoopAudioComponent->Stop();
	}


	//父类 OnHit 只有 Destroyed
	//Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}


void AProjectile_Rocket::DestroyRocketTrailWhenTimerFinished()
{
	Destroy();
}

void AProjectile_Rocket::Destroyed()
{

}