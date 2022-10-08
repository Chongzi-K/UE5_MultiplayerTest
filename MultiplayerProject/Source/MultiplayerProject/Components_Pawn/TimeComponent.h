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

	bool bInitialized;//�Ƿ��ѱ���ʼ��
	bool bCanTimeReversing;//�ܷ����ʱ�����
	bool bOutData;//ʱ������Ƿ�ִ�����
	float RecordTimeLength;//�ɻ��ݵ�ʱ�䳤��

	TDoubleLinkedList<TimeInfo> TimeFrames;//˫�����洢ÿһ֡��¼����Ϣ

	UFUNCTION()
		void SetTimeReversing(bool InbCanTimeReversing);//���л���
		
};
