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
			false// bAutoDestroy ������Ҫ���� actor �Զ����٣���ΪҪʵ�ֱ�ը������켣��Ȼ���ڵ�Ч��
			);
	}

	if (ProjectileFlyingLoop && LoopingSoundAttenuation)
	{
		//�����������Ч+˥��
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
	APawn* FiringPawn = GetInstigator();//��ȡ��ö������ķ�����
	if (FiringPawn)
	{
		AController* FiringController = FiringPawn->GetController();
		if (FiringController && HasAuthority())
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


	//���� OnHit ֻ�� Destroyed
	//Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}


void AProjectile_Rocket::DestroyRocketTrailWhenTimerFinished()
{
	Destroy();
}

void AProjectile_Rocket::Destroyed()
{

}