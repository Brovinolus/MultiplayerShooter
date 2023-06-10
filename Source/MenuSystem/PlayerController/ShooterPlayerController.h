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

	virtual float GetServerTime(); // Synced with server world clock
	virtual void ReceivedPlayer() override; // Sync with server clock as soon as possible
protected:
	virtual void BeginPlay() override;
	void CheckTimeSync(float DeltaSeconds);
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnPossess(APawn* InPawn) override;
	void PingValue();
	void UpdateHUDValues();
	void SetHUDTime();

	// Sync time between client and server

	// Request the current server time, passing in the client's time when the request was sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	// Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	// difference between client and server time
	float ClientServerDelta = 0.f;
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;

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
