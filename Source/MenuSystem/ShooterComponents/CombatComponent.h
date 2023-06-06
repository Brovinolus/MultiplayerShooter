// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MenuSystem/Weapon/WeaponTypes.h"
#include "MenuSystem/ShooterTypes/CombatState.h"
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
	void ReloadWeapon();
	void UpdateAmmoValues();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();
	bool bLocallyReloading = false;
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

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	int32 AmountToReload();

private:
	UPROPERTY()
	TObjectPtr<AShooterCharacter> Character;
	UPROPERTY()
	TObjectPtr<class AShooterPlayerController> Controller;
	UPROPERTY()
	TObjectPtr<class AShooterHUD> HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	TObjectPtr<AWeapon> EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;

	bool bAimButtonPressed = false;

	UFUNCTION()
	void OnRep_Aiming();

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

	// Max Ammo for the equipped weapon
	UPROPERTY(ReplicatedUsing = OnRep_MaxWeaponAmmo)
	int32 MaxWeaponAmmo;

	void InitializeMaxAmmo();
	
	UFUNCTION()
	void OnRep_MaxWeaponAmmo();

	// cannot be replicated, uses hash algorithm
	UPROPERTY(EditAnywhere)
	TMap<EWeaponType, int32> MaxAmmoMap;

	int32 MaxAmmoRifle = 120;
	int32 MaxAmmoPistol = 60;

	/**
	* Default weapon
	*/
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();
};
