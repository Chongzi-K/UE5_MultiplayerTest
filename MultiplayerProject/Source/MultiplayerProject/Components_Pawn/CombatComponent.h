// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"




UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERPROJECT_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()
	

public:	

	UCombatComponent();

	friend class AMainCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	void EquipWeapon(class AWeapon* WeaponToEquip);

protected:

	virtual void BeginPlay() override;

private:

	UPROPERTY()
	class AMainCharacter* MainCharacter;

	UPROPERTY(Replicated)
	AWeapon* EquippedWeapon;


		
};
