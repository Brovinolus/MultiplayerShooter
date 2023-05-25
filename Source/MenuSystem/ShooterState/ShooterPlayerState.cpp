// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterPlayerState.h"
#include "MenuSystem/Character/ShooterCharacter.h"
#include "MenuSystem/PlayerController/ShooterPlayerController.h"
#include "Net/UnrealNetwork.h"

void AShooterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AShooterPlayerState, Deaths);
}

void AShooterPlayerState::AddToDeaths(int32 DeathsAmount)
{
	Deaths += DeathsAmount;

	Character = Character == nullptr ? Cast<AShooterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		PlayerController = PlayerController == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDDeaths(Deaths);
		}
	}
}

void AShooterPlayerState::OnRep_Deaths()
{
	Character = Character == nullptr ? Cast<AShooterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		PlayerController = PlayerController == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDDeaths(Deaths);
		}
	}
}

// client
void AShooterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<AShooterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		PlayerController = PlayerController == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDKillsCount(GetScore());
		}
	}
}

// server
void AShooterPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);

	Character = Character == nullptr ? Cast<AShooterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		PlayerController = PlayerController == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetHUDKillsCount(GetScore());
		}
	}
}
