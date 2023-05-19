// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGameMode.h"
#include "MenuSystem/Character/ShootingCharacter.h"
#include "MenuSystem/PlayerController/ShooterPlayerController.h"

void AShooterGameMode::PlayerEliminated(TObjectPtr<AShootingCharacter> EliminatedCharacter,
                                        TObjectPtr<AShooterPlayerController> VictimController, TObjectPtr<AShooterPlayerController> AttackerController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->CharacterEliminated();
	}
}
