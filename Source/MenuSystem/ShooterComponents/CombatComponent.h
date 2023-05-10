// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AWeapon;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MENUSYSTEM_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class AShootingCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

	void EquipWeapon(AWeapon* WeaponToEquip);
protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);
	void SetAimingYawRotation(float AimingYaw);
	void SetAimingPitchRotation(float AimingPitch);
	//void SetWalking(bool IsWalking);

	// Send data from a client to a server
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	//UFUNCTION(Server, Reliable)
	//void ServerSetWalking(bool IsWalking);

	UFUNCTION(Server, Reliable)
	void ServerSetAimingYawRotation(float AimingYaw);

	UFUNCTION(Server, Reliable)
	void ServerSetAimingPitchRotation(float AimingPitch);

private:
	TObjectPtr<AShootingCharacter> Character;

	UPROPERTY(Replicated)
	TObjectPtr<AWeapon> EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	//UPROPERTY(Replicated)
	//bool bWalking;

	UPROPERTY(Replicated)
	float AimingYawRotation;

	UPROPERTY(Replicated)
	float AimingPitchRotation;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	//UPROPERTY(EditAnywhere)
	//float BaseWalkSpeed;

	//UPROPERTY(EditAnywhere)
	//float AimWalkSpeed;

};
