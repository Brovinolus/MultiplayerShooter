// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f;

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

	// Send data from a client to a server
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void FireButtonPressed(bool bPressed);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);
	
	void TraceToShoot(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

private:
	TObjectPtr<AShootingCharacter> Character;
	TObjectPtr<class AShooterPlayerController> Controller;
	TObjectPtr<class AShooterHUD> HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	TObjectPtr<AWeapon> EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;
	
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootFactor;

	UPROPERTY(EditAnywhere)
	float CrosshairBaseValue = 0.5f;
	UPROPERTY(EditAnywhere)
	float CrosshairInAirFactorTarget = 2.25f;
	UPROPERTY(EditAnywhere)
	float CrosshairAimFactorTarget = -0.6f;
	UPROPERTY(EditAnywhere)
	float CrosshairShootFactorTarget = 0.75f;
	
	FVector HitTarget;

	/**
	 * Aiming and FOV
	 */

	// Field of view when not aiming
	float DefaultFOV;
	FVector DefaultCameraLocation;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedFOV = 30.f;

	float CurrentFOV;
	FVector CurrentCameraLocation;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);
};
