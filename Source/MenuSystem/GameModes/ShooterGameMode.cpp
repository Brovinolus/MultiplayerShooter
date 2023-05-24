// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGameMode.h"

#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "MenuSystem/Character/ShooterCharacter.h"
#include "MenuSystem/PlayerController/ShooterPlayerController.h"
#include "MenuSystem/ShooterState/ShooterPlayerState.h"

void AShooterGameMode::PlayerEliminated(TObjectPtr<AShooterCharacter> EliminatedCharacter,
                                        TObjectPtr<AShooterPlayerController> VictimController, TObjectPtr<AShooterPlayerController> AttackerController)
{
	AShooterPlayerState* AttackerPlayerState = AttackerController
		                                          ? Cast<AShooterPlayerState>(AttackerController->PlayerState)
		                                          : nullptr;
	AShooterPlayerState* VictimPlayerState = VictimController
		                                         ? Cast<AShooterPlayerState>(VictimController->PlayerState)
		                                         : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDeaths(1);
	}
	
	if (EliminatedCharacter)
	{
		EliminatedCharacter->CharacterEliminated();
	}
}

void AShooterGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Reset();
		EliminatedCharacter->Destroy();
	}

	if (EliminatedController)
	{
		TArray<AActor*> PlayerStarts;
		TArray<AActor*> Characters;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		UGameplayStatics::GetAllActorsOfClass(this, ACharacter::StaticClass(), Characters);
		
		AActor* SelectedPlayerStart = GetActorWithHighestDistance(PlayerStarts, Characters);
		
		// if there is no other characters, then the PlayStart is random
		SelectedPlayerStart = SelectedPlayerStart == nullptr
			                      ? PlayerStarts[FMath::RandRange(0, PlayerStarts.Num() - 1)]
			                      : SelectedPlayerStart;
		
		RestartPlayerAtPlayerStart(EliminatedController, SelectedPlayerStart);
	}
}

// the highest minimum distance of a Player Start location relative to all characters
AActor* AShooterGameMode::GetActorWithHighestDistance(const TArray<AActor*>& PlayerStarts,
	const TArray<AActor*>& Characters)
{
	if (PlayerStarts.Num() == 0 || Characters.Num() == 0) { return nullptr; }
	
	float MaxDistance = 0.f;
	AActor* PlayerStartWithMaxDistance = nullptr;

	for (AActor* PlayerStart : PlayerStarts)
	{
		float MinDistance = TNumericLimits<float>::Max();
		
		for (AActor* Character : Characters)
		{
			float Distance = PlayerStart->GetDistanceTo(Character);
			MinDistance = FMath::Min(MinDistance, Distance);
		}

		if (MinDistance > MaxDistance)
		{
			MaxDistance = MinDistance;
			PlayerStartWithMaxDistance = PlayerStart;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("MaxDistance: %f"), MaxDistance);
	return PlayerStartWithMaxDistance;
}



