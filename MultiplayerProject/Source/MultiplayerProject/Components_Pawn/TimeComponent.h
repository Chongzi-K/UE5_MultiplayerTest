// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Math/Vector.h"
#include "Math/Rotator.h"
#include "TimeComponent.generated.h"


struct TimeInfo
{
	FVector Location;
	FRotator Rotation;
	FVector LinearVelocity;
	FVector AngularVelocity;
	float DeltaTime;

	TimeInfo(FVector InLocation, FRotator InRotation, FVector InLinearVelocity, FVector InAngularVelocity, float InDeltaTime):
	Location(InLocation), Rotation(InRotation), LinearVelocity(InLinearVelocity), AngularVelocity(InAngularVelocity), DeltaTime(InDeltaTime)
	{

	}
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UTimeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTimeComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	AActor* Owner;

	bool bInitialized;//是否已被初始化
	bool bCanTimeReversing;//能否进行时间回溯
	bool bOutData;//时间回溯是否执行完成
	float RecordTimeLength;//可回溯的时间长度

	TDoubleLinkedList<TimeInfo> TimeFrames;//双链表，存储每一帧记录的信息

	UFUNCTION()
		void SetTimeReversing(bool InbCanTimeReversing);//进行回溯
		
};
