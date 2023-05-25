// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

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
protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
private:
	UPROPERTY()
	TObjectPtr<class AShooterHUD> ShooterHUD;
};
