// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ShooterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class MENUSYSTEM_API AShooterPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	/**
	 * Replication notifies
	 */
	UFUNCTION()
	virtual void OnRep_Deaths();
	virtual void OnRep_Score() override;
	
	void AddToScore(float ScoreAmount);
	void AddToDeaths(int32 DeathsAmount);
private:
	UPROPERTY()
	TObjectPtr<class AShooterCharacter> Character;
	UPROPERTY()
	TObjectPtr<class AShooterPlayerController> PlayerController;

	UPROPERTY(ReplicatedUsing = OnRep_Deaths)
	int32 Deaths;

	
};
