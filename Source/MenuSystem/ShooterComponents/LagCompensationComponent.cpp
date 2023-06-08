// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"

#include "MenuSystem/Character/ShooterCharacter.h"
#include "DrawDebugHelpers.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}


void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	FFramePackage Package;
	SaveFramePackage(Package);
	ShowFramePackage(Package);
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package)
{
	for (auto& PhysicAssetInfo : Package.HitPhysicsAssetInfo)
	{
		auto WorldTransform = PhysicAssetInfo.Value.CapsuleTransform * PhysicAssetInfo.Value.BoneTransform;
		DrawDebugCapsule(
			GetWorld(),
			WorldTransform.GetLocation(), 
			PhysicAssetInfo.Value.CapsuleLength / 2 + PhysicAssetInfo.Value.CapsuleRadius, 
			PhysicAssetInfo.Value.CapsuleRadius,
			WorldTransform.GetRotation(), 
			FColor::Red,
			true
		);
	}
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character ==nullptr ? Cast<AShooterCharacter>(GetOwner()) : Character;

	if (Character)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		for (const auto& PhysicsAsset : Character->HitCollisionData)
		{
			FPhysicAssetInformation PhysicAssetInformation;
			PhysicAssetInformation.BoneTransform = PhysicsAsset.Value.BoneTransform;
			PhysicAssetInformation.CapsuleTransform = PhysicsAsset.Value.CapsuleInfo.GetTransform();
			PhysicAssetInformation.CapsuleLength = PhysicsAsset.Value.CapsuleInfo.Length;
			PhysicAssetInformation.CapsuleRadius = PhysicsAsset.Value.CapsuleInfo.Radius;

			Package.HitPhysicsAssetInfo.Add(PhysicsAsset.Key, PhysicAssetInformation);
		}
	}
}


void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

