// Fill out your copyright notice in the Description page of Project Settings.


#include "TimeComponent.h"


UTimeComponent::UTimeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	Owner = nullptr;
	bInitialized = false;
	bCanTimeReversing = false;
	bOutData = false;
	RecordTimeLength = 0.0f;

}



void UTimeComponent::BeginPlay()
{
	Super::BeginPlay();


	
}



void UTimeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);


}

void UTimeComponent::SetTimeReversing(bool InbCanTimeReversing)
{
}

