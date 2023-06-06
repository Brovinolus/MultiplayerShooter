// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

enum class EWeaponType : uint8;
/**
 * 
 */
UCLASS()
class MENUSYSTEM_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDKillsCount(float KillsCount);
	void SetHUDDeaths(int32 Deaths);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDWeaponMaxAmmo(int32 Ammo);
	void SetHUDWeaponType(EWeaponType WeaponType);
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnPossess(APawn* InPawn) override;
	void PingValue();
	void UpdateHUDValues();
	
private:
	UPROPERTY()
	TObjectPtr<class AShooterHUD> ShooterHUD;
	UPROPERTY()
	TObjectPtr<class AShooterPlayerState> ShooterPlayerState;

	TObjectPtr<class UCharacterOverlay> CharacterOverlay;

	float HUDMaxAmmo;
	bool bInitializeMaxAmmo = false;
	float HUDWeaponAmmo;
	bool bInitializeWeaponAmmo = false;
};
