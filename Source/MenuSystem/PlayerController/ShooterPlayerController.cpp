// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerController.h"
#include "MenuSystem/HUD/CharacterOverlay.h"
#include "MenuSystem/HUD/ShooterHUD.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"	
#include "MenuSystem/Character/ShooterCharacter.h"
#include "MenuSystem/ShooterState/ShooterPlayerState.h"
#include "MenuSystem/Weapon/WeaponTypes.h"

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	//ShooterPlayerState = GetPlayerState<AShooterPlayerState>();

	ShooterHUD = Cast<AShooterHUD>(GetHUD());
}

void AShooterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	
	PingValue(DeltaSeconds);

	UpdateHUDValues();

	CheckTimeSync(DeltaSeconds);

	SetHUDTime();
}

void AShooterPlayerController::CheckTimeSync(float DeltaSeconds)
{
	TimeSyncRunningTime += DeltaSeconds;
	if(IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
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

void AShooterPlayerController::PingValue(float DeltaSeconds)
{
	HighPingRunningTime += DeltaSeconds;

	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;

	bool bHUDValid = ShooterHUD && ShooterHUD->CharacterOverlay && ShooterHUD
	                                                               ->CharacterOverlay->PingValue;
	if (bHUDValid)
	{
		ShooterPlayerState = ShooterPlayerState == nullptr ? GetPlayerState<AShooterPlayerState>() : ShooterPlayerState;
		if (ShooterPlayerState)
		{
			FString PingValue = FString::Printf(
				TEXT("%d"), FMath::FloorToInt(ShooterPlayerState->GetPingInMilliseconds()));
			ShooterHUD->CharacterOverlay->PingValue->SetText(FText::FromString(PingValue));

			// if the picked up weapon has SSR disabled, then don't update
			if (bSSR_State)
			{
				if (HighPingRunningTime > CheckPingFrequency)
				{
					if (ShooterPlayerState->GetPingInMilliseconds() > HighPingThreshold)
					{
						ServerReportPingStatus(true);
					
						ShooterHUD->CharacterOverlay->SSR_State->SetText(FText::FromString("SSR Disabled"));
					}
					else
					{
						ServerReportPingStatus(false);

						ShooterHUD->CharacterOverlay->SSR_State->SetText(FText::FromString("SSR Enabled"));
					}
				
					HighPingRunningTime = 0.f;
				}
			}
		}
	}
}

void AShooterPlayerController::SetHUDWeaponSSR(bool SSR_State)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD && ShooterHUD->CharacterOverlay && ShooterHUD->CharacterOverlay->WeaponType;
	if(bHUDValid)
	{
		if (SSR_State)
		{
			bSSR_State = true;
			ShooterHUD->CharacterOverlay->SSR_State->SetText(FText::FromString("SSR Enabled"));
		}
		else
		{
			bSSR_State = false;
			ShooterHUD->CharacterOverlay->SSR_State->SetText(FText::FromString("SSR Disabled"));
		}
	}
}

void AShooterPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

void AShooterPlayerController::UpdateHUDValues()
{
	if (CharacterOverlay == nullptr)
	{
		bool bHUDValid = ShooterHUD && ShooterHUD->CharacterOverlay;
		if (bHUDValid)
		{
			CharacterOverlay = ShooterHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				if (bInitializeMaxAmmo) SetHUDWeaponMaxAmmo(HUDMaxAmmo);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);
			}
		}
	}
}

void AShooterPlayerController::SetHUDTime()
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD && ShooterHUD->CharacterOverlay && ShooterHUD->CharacterOverlay->HealthBar && ShooterHUD
		->CharacterOverlay->MatchTime;
	if(bHUDValid)
	{
		uint32 MatchTime = FMath::CeilToInt(GetServerTime());
		FString MatchTimeText = FString::Printf(TEXT("%d"), MatchTime);
		ShooterHUD->CharacterOverlay->MatchTime->SetText(FText::FromString(MatchTimeText));
	}
}

void AShooterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void AShooterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest,
	float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = (0.5f * RoundTripTime);
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
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

void AShooterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD && ShooterHUD->CharacterOverlay && ShooterHUD->CharacterOverlay->WeaponAmmo;
	if(bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		ShooterHUD->CharacterOverlay->WeaponAmmo->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

void AShooterPlayerController::SetHUDWeaponMaxAmmo(int32 Ammo)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD && ShooterHUD->CharacterOverlay && ShooterHUD->CharacterOverlay->WeaponMaxAmmo;
	if(bHUDValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		ShooterHUD->CharacterOverlay->WeaponMaxAmmo->SetText(FText::FromString(AmmoText));
	}
	else
	{
		bInitializeMaxAmmo = true;
		HUDMaxAmmo = Ammo;
	}
}

void AShooterPlayerController::SetHUDWeaponType(EWeaponType WeaponType)
{
	ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(GetHUD()) : ShooterHUD;
	bool bHUDValid = ShooterHUD && ShooterHUD->CharacterOverlay && ShooterHUD->CharacterOverlay->WeaponType;
	if(bHUDValid)
	{
		FString WeaponTypeText = "";
		
		UE_LOG(LogTemp, Warning, TEXT("%s"), *UEnum::GetValueAsString(WeaponType));
		if (WeaponType == EWeaponType::EWT_Rifle)
		{
			WeaponTypeText = "Rifle";
		}
		
		if (WeaponType == EWeaponType::EWT_Pistol)
		{
			WeaponTypeText = "Pistol";
		}

		ShooterHUD->CharacterOverlay->WeaponType->SetText(FText::FromString(WeaponTypeText));
	}
}

float AShooterPlayerController::GetServerTime()
{
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void AShooterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}
