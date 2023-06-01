// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerController.h"
#include "MenuSystem/HUD/CharacterOverlay.h"
#include "MenuSystem/HUD/ShooterHUD.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"	
#include "MenuSystem/Character/ShooterCharacter.h"
#include "MenuSystem/ShooterState/ShooterPlayerState.h"

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//ShooterPlayerState = GetPlayerState<AShooterPlayerState>();

	ShooterHUD = Cast<AShooterHUD>(GetHUD());
}

void AShooterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	PingValue();
}

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(InPawn);
	if (ShooterCharacter)
	{
		ShooterCharacter->UpdateHUDHealth();
	}
}

void AShooterPlayerController::PingValue()
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;

	bool bHUDValid = ShooterHUD && ShooterHUD->CharacterOverlay && ShooterHUD
		->CharacterOverlay->PingValue;
	if(bHUDValid)
	{
		ShooterPlayerState = ShooterPlayerState == nullptr ? GetPlayerState<AShooterPlayerState>() : ShooterPlayerState;
		if (ShooterPlayerState)
		{
			FString PingValue = FString::Printf(TEXT("%d"), FMath::FloorToInt(ShooterPlayerState->GetPingInMilliseconds()));
			ShooterHUD->CharacterOverlay->PingValue->SetText(FText::FromString(PingValue));
		}
	}
}

void AShooterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;

	bool bHUDValid = ShooterHUD && ShooterHUD->CharacterOverlay && ShooterHUD->CharacterOverlay->HealthBar && ShooterHUD
		->CharacterOverlay->HealthText;
	if(bHUDValid)
	{
		const float HealthPercent = Health / MaxHealth;
		ShooterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		ShooterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void AShooterPlayerController::SetHUDKillsCount(float KillsCount)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD && ShooterHUD->CharacterOverlay && ShooterHUD->CharacterOverlay->HealthBar && ShooterHUD
		->CharacterOverlay->KillCount;
	if(bHUDValid)
	{
		FString KillsText = FString::Printf(TEXT("%d"), FMath::FloorToInt(KillsCount));
		ShooterHUD->CharacterOverlay->KillCount->SetText(FText::FromString(KillsText));
	}
}

void AShooterPlayerController::SetHUDDeaths(int32 Deaths)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD && ShooterHUD->CharacterOverlay && ShooterHUD->CharacterOverlay->HealthBar && ShooterHUD
		->CharacterOverlay->DeathCount;
	if(bHUDValid)
	{
		FString DeathsText = FString::Printf(TEXT("%d"), Deaths);
		ShooterHUD->CharacterOverlay->DeathCount->SetText(FText::FromString(DeathsText));
	}
}
