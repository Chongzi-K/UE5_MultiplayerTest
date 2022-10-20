// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotGun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "MultiplayerProject/MainCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

void AShotGun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) { return; }//��ȡ����������ʱֱ����Ч

	AController* InstigatorController = OwnerPawn->GetController();
	//���� Cast ����ʡ����
	//Controller �� Simulate ʵ�� �϶�Ϊ nullptr


	const USkeletalMeshSocket* MuzzleFlashScoket = GetWeaponMesh()->GetSocketByName("MuzzleFlashSocket");
	if (MuzzleFlashScoket)
	{
		FTransform MuzzleFlashSocketFransform = MuzzleFlashScoket->GetSocketTransform(GetWeaponMesh());
		FVector ScaleStartPoint = MuzzleFlashSocketFransform.GetLocation();
		
		//uint32 Hits = 0;//���ڼ��������˼��� //ֱ��++����
		TMap<AMainCharacter*, uint32>HitMap;//���������-���е��ӵ�����

		//ѭ������ÿһ���ӵ�����������
		for (uint32 i = 0; i < NumOfProjectilesPerShot; i++)
		{
			//FVector ScaleEndPoint = TraceEndWithScatter(ScaleStartPoint, HitTarget);
			FHitResult FireHit;
			WeaponTraceHit(ScaleStartPoint, HitTarget, FireHit);

			//BeamEnd = FireHit.ImpactPoint;//������ɹ������ӵ�β���յ�Ϊ����
			AMainCharacter* MainCharacter = Cast<AMainCharacter>(FireHit.GetActor());

			//���к��ڷ������ϸ�������ͼ
			if (MainCharacter && HasAuthority() && InstigatorController)
			{
				if (HitMap.Contains(MainCharacter))
				{
					HitMap[MainCharacter]++;
				}
				else
				{
					HitMap.Emplace(MainCharacter, 1);
				}
			}

			//�������л������Ӻ��������������ڸ����У�����Ϊ�����ͳһ�������У���ɢ���ӵ������е��ʱ����������
			// ��ÿ���ӵ������Եõ����������Ӻ���������
			//������������
			if (ImpactParticleSystem)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticleSystem,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}
			//�����ӵ�����
			if (HitSound)
			{
				UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint, 0.5f, FMath::FRandRange(-0.5f, 0.5f));
			}
		}

		//ѭ������Ϊ������ͼ�ڵ���������˺�
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key && HasAuthority() && InstigatorController)
			{
				UGameplayStatics::ApplyDamage(
					HitPair.Key,
					Damage * HitPair.Value,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
		}
	}
}

