// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"

#include "MenuSystem/Character/ShooterCharacter.h"
#include "DrawDebugHelpers.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/GameplayStaticsTypes.h"
#include "MenuSystem/MenuSystem.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}


FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewindResult(AShooterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	return ProjectileConfirmHit(FrameToCheck, TraceStart, InitialVelocity, HitTime);
}

FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(const FFramePackage& Package, AShooterCharacter* HitCharacter,
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	if (HitCharacter == nullptr) return  FServerSideRewindResult();

	FFramePackage CurrentFrame;
	CacheBoxPosition(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, Package);

	// head
	UBoxComponent* HeadBox = HitCharacter->BoxCollision[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);

	FPredictProjectilePathParams PathParams;
	
	PathParams.bTraceWithCollision = true;
	PathParams.MaxSimTime = MaxRecordTime;
	PathParams.ProjectileRadius = 5.f;
	PathParams.SimFrequency = 15.f;
	PathParams.StartLocation = TraceStart;
	PathParams.TraceChannel = ECC_HitBox;
	PathParams.LaunchVelocity = InitialVelocity;
	PathParams.ActorsToIgnore.Add(GetOwner());
	PathParams.DrawDebugTime = 5.f;
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;

	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);

	if (PathResult.HitResult.bBlockingHit) // headshot
	{
		if (PathResult.HitResult.Component.IsValid())
		{
			UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
			if (Box)
			{
				DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(),
				             FQuat(Box->GetComponentRotation()), FColor::Red);
			}
		}

		ResetHitBoxes(HitCharacter, CurrentFrame);
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		return FServerSideRewindResult{true, true };
	}
	else
	{
		for (auto& HitBoxPair : HitCharacter->BoxCollision)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
			}
		}

		UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);

		if (PathResult.HitResult.bBlockingHit)
		{
			if (PathResult.HitResult.Component.IsValid())
			{
				UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
				if (Box)
				{
					DrawDebugBox(GetWorld(), Box->GetComponentLocation(), Box->GetScaledBoxExtent(),
								 FQuat(Box->GetComponentRotation()), FColor::Red);
				}
			}

			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{true, false };
		}
	}
	
	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{false, false };
}

void ULagCompensationComponent::CacheBoxPosition(AShooterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if(HitCharacter == nullptr) return;

	for (auto& HitBoxPair : HitCharacter->BoxCollision)
	{
		if (HitBoxPair.Value != nullptr)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

void ULagCompensationComponent::MoveBoxes(AShooterCharacter* HitCharacter, const FFramePackage& Package)
{
	if(HitCharacter == nullptr) return;

	for (auto& HitBoxPair : HitCharacter->BoxCollision)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
		}
	}
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame,
	const FFramePackage& YoungerFrame, float HiTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HiTime - OlderFrame.Time) / Distance, 0.f, 1.f);

	FFramePackage InterFramePackage;
	InterFramePackage.Time = HiTime;

	for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;
		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];
		const FBoxInformation& YoungerBox = YoungerPair.Value;

		FBoxInformation InterpBoxInfo;
		InterpBoxInfo.Location = FMath::LerpStable(OlderBox.Location, YoungerBox.Location, InterpFraction);
		InterpBoxInfo.Rotation = FMath::LerpStable(OlderBox.Rotation, YoungerBox.Rotation, InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		InterFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}

	return InterFramePackage;
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
	for (auto& BoxInfo : Package.HitBoxInfo)
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
			FBoxInformation BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();

			Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);
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

