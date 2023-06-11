// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <deque>

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FPhysicAssetInformation
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
	TMap<FName, FPhysicAssetInformation> HitPhysicsAssetInfo;
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
	void ServerSideRewind(AShooterCharacter* HitCharacter, const FVector_NetQuantize&);
	
protected:
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package);
	//void SaveFramePackageCapsule(FFramePackage& Package);

	//void TickCapsule();
	//void TickPhysicAsset();
private:
	UPROPERTY()
	TObjectPtr<AShooterCharacter> Character;

	UPROPERTY()
	TObjectPtr<APlayerController> Controller;

	std::deque<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;
public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
		
};
