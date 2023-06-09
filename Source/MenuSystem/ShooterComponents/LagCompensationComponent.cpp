// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"

#include "MenuSystem/Character/ShooterCharacter.h"
#include "DrawDebugHelpers.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}


void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	//FFramePackage Package;
	//SaveFramePackage(Package);
	//ShowFramePackage(Package);
}

/*
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
			false,
			4.f
		);
	}
}

void ULagCompensationComponent::ShowFramePackageCapsule(FFramePackage& Package)
{
	for (auto& CapsuleInfo : Package.HitPhysicsAssetInfo)
	{
		DrawDebugCapsule(
			GetWorld(),
			CapsuleInfo.Value.CapsuleTransform.GetLocation(), 
			CapsuleInfo.Value.CapsuleLength / 2 + CapsuleInfo.Value.CapsuleRadius, 
			CapsuleInfo.Value.CapsuleRadius,
			CapsuleInfo.Value.CapsuleTransform.GetRotation(), 
			FColor::Red,
			false,
			4.f
		);
	}
}

void ULagCompensationComponent::SaveFramePackageCapsule(FFramePackage& Package)
{
	Character = Character ==nullptr ? Cast<AShooterCharacter>(GetOwner()) : Character;

	if (Character)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		for (const auto& CapsuleInfo : Character->CapsuleCollisionCopy)
		{
			FPhysicAssetInformation PhysicAssetInformation;
			PhysicAssetInformation.CapsuleTransform = CapsuleInfo.Value->GetComponentTransform();
			PhysicAssetInformation.CapsuleLength = CapsuleInfo.Value->GetScaledCapsuleHalfHeight();
			PhysicAssetInformation.CapsuleRadius = CapsuleInfo.Value->GetScaledCapsuleRadius();

			Package.HitPhysicsAssetInfo.Add(CapsuleInfo.Key, PhysicAssetInformation);
		}
	}
}

void ULagCompensationComponent::TickCapsule()
{
	if (FrameHistory.size() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackageCapsule(ThisFrame);
		FrameHistory.push_front(ThisFrame);
	}
	else
	{
		float HistoryLength = FrameHistory.front().Time - FrameHistory.back().Time;
		while (HistoryLength > MaxRecordTime)
		{
			FrameHistory.pop_back();
			HistoryLength = FrameHistory.front().Time - FrameHistory.back().Time;
		}
		FFramePackage ThisFrame;
		SaveFramePackageCapsule(ThisFrame);
		FrameHistory.push_front(ThisFrame);

		ShowFramePackageCapsule(ThisFrame);
	}
}

void ULagCompensationComponent::TickPhysicAsset()
{
	if (FrameHistory.size() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.push_front(ThisFrame);
	}
	else
	{
		float HistoryLength = FrameHistory.front().Time - FrameHistory.back().Time;
		while (HistoryLength > MaxRecordTime)
		{
			FrameHistory.pop_back();
			HistoryLength = FrameHistory.front().Time - FrameHistory.back().Time;
		}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.push_front(ThisFrame);

		ShowFramePackage(ThisFrame);
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
}*/

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package)
{
	for (auto& BoxInfo : Package.HitPhysicsAssetInfo)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location, 
			BoxInfo.Value.BoxExtent, 
			FQuat(BoxInfo.Value.Rotation),
			FColor::Red,
			false,
			4.f
		);
	}
}

void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character ==nullptr ? Cast<AShooterCharacter>(GetOwner()) : Character;

	if (Character)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		for (const auto& BoxPair : Character->BoxCollision)
		{
			FPhysicAssetInformation BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();

			Package.HitPhysicsAssetInfo.Add(BoxPair.Key, BoxInformation);
		}
	}
}


void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (FrameHistory.size() <= 1)
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.push_front(ThisFrame);
	}
	else
	{
		float HistoryLength = FrameHistory.front().Time - FrameHistory.back().Time;
		while (HistoryLength > MaxRecordTime)
		{
			FrameHistory.pop_back();
			HistoryLength = FrameHistory.front().Time - FrameHistory.back().Time;
		}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.push_front(ThisFrame);

		ShowFramePackage(ThisFrame);
	}
}

