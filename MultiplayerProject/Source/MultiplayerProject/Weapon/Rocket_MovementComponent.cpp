// Fill out your copyright notice in the Description page of Project Settings.


#include "Rocket_MovementComponent.h"

URocket_MovementComponent::EHandleBlockingHitResult URocket_MovementComponent::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
	return EHandleBlockingHitResult::AdvanceNextSubstep;
	//���ڴ�����ʱ����������Լ�
	//�����������ν���������˶�
}

void URocket_MovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice = 0.f, const FVector& MoveDelta = FVector::ZeroVector)
{

}
