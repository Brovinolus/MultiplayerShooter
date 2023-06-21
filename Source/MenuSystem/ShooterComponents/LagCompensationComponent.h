// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <deque>

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MenuSystem/Character/ShooterCharacter.h"
#include "LagCompensationComponent.generated.h"

struct FPredictProjectilePathResult;
struct FPredictProjectilePathParams;

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
	/*
	UPROPERTY()
	FTransform BoneTransform;

	UPROPERTY()
	FTransform CapsuleTransform;

	UPROPERTY()
	float CapsuleLength;

	UPROPERTY()
	float CapsuleRadius;
	*/
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;
};

USTRUCT()
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MENUSYSTEM_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend class AShooterCharacter;
	void ShowFramePackage(const FFramePackage& Package);
	//void ShowFramePackageCapsule(FFramePackage& Package);

	FServerSideRewindResult ProjectileServerSideRewindResult(
		AShooterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(
		AShooterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);
	
protected:
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package);
	void SaveFramePackage();
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HiTime);
	FFramePackage GetFrameToCheck(AShooterCharacter* HitCharacter, float HitTime);
	FServerSideRewindResult ProjectileConfirmHit(
		const FFramePackage& Package,
		AShooterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	void CacheBoxPosition(AShooterCharacter* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(AShooterCharacter* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(AShooterCharacter* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(AShooterCharacter* HitCharacter, ECollisionEnabled::Type CollisionType);

	// Character Hit Notification and Effects
	void CharacterHit();

	UFUNCTION(Server, Reliable)
	void DrawDebugBoxes(FVector Location, FVector BoxExtent, FRotator Rotator);
	UFUNCTION(Server, Reliable)
	void DrawHitResult(const FPredictProjectilePathParams& PathParams, FPredictProjectilePathResult PathResult);

	//void SaveFramePackageCapsule(FFramePackage& Package);
	//void TickCapsule();
	//void TickPhysicAsset();
private:
	UPROPERTY()
	TObjectPtr<AShooterCharacter> Character;

	UPROPERTY()
	TObjectPtr<APlayerController> Controller;

	//std::deque<FFramePackage> FrameHistory;
	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 2.f;
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
		
};
