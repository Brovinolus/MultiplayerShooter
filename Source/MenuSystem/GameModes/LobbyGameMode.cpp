// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	// Super keyword is used to call the parent class's implementation of a function
	Super::PostLogin(NewPlayer);

	if (GameState)
	{
		int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				1,
				60.f,
				FColor::Yellow,
				FString::Printf(TEXT("Players in game: %d"), NumberOfPlayers)
			);
		}
		APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>();
		if(PlayerState)
		{
			FString PlayerName = PlayerState->GetPlayerName();
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					60.f,
					FColor::Orange,
					FString::Printf(TEXT("%s has joined the game!"), *PlayerName)
				);
			}
		
			FName SessionName = PlayerState->SessionName;
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					60.f,
					FColor::White,
					FString::Printf(TEXT("%s"), *SessionName.ToString())
				);
			}
		}

		if (NumberOfPlayers > 1)
		{
			UWorld* World = GetWorld();

			if(World)
			{
				bUseSeamlessTravel = true;
				World->ServerTravel(FString("/Game/Maps/ShooterMap?listen"));
			}
		}
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	APlayerState* PlayerState = Exiting->GetPlayerState<APlayerState>();
	if(PlayerState)
	{
		int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();
		
		GEngine->AddOnScreenDebugMessage(
				1,
				60.f,
				FColor::Yellow,
				FString::Printf(TEXT("Players in game: %d"), NumberOfPlayers - 1)
		);
		
		FString PlayerName = PlayerState->GetPlayerName();
		GEngine->AddOnScreenDebugMessage(
		-1,
		60.f,
		FColor::Cyan,
		FString::Printf(TEXT("%s has exited the game!"), *PlayerName)
	);
	}
}

