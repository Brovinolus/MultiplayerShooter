// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class MENUSYSTEM_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UProgressBar> HealthBar;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> HealthText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> KillCount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DeathCount;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> PingValue;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WeaponAmmo;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WeaponMaxAmmo;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> WeaponType;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> MatchTime;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SSR_State;
};
