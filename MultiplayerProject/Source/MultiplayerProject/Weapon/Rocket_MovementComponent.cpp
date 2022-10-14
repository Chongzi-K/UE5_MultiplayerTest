// Fill out your copyright notice in the Description page of Project Settings.


#include "Rocket_MovementComponent.h"


URocket_MovementComponent::EHandleBlockingHitResult URocket_MovementComponent::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	//用于处理发射时火箭弹碰到自己
    //阻塞后忽略这次结果，继续运动
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
	return EHandleBlockingHitResult::AdvanceNextSubstep;
}

void URocket_MovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	// Rockets should not stop; only explode when their CollisionBox detects a hit
}
