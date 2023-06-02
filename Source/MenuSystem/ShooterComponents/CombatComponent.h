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
	friend class AShooterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

	void EquipWeapon(AWeapon* WeaponToEquip);
	void SpawnDefaultWeapon();
protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	// Send data from a client to a server
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();
	void Fire();
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);

	void FireButtonPressed(bool bPressed);

	void SprintButtonPressed(bool bPressed);

	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bIsSprinting);

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);
	
	void TraceToShoot(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

private:
	UPROPERTY()
	TObjectPtr<AShooterCharacter> Character;
	UPROPERTY()
	TObjectPtr<class AShooterPlayerController> Controller;
	UPROPERTY()
	TObjectPtr<class AShooterHUD> HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	TObjectPtr<AWeapon> EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	UPROPERTY(EditAnywhere)
	float SprintSpeed;

	bool bFireButtonPressed;
	bool bSprintButtonPressed;
	
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
	float CurrentFOV;
	FVector DefaultCameraLocation;
	FVector CurrentCameraLocation;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedFOV = 30.f;
	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomInterpSpeed = 20.f;
	
	void InterpFOV(float DeltaTime);

	/**
	* Automatic fire
	*/
	
	FTimerHandle FireTimer;
	bool bCanFire = true;
	void StartFireTimer();
	void FireTimerFinished();

	bool CanFire();

	/**
	* Default weapon
	*/
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeapon;
};
