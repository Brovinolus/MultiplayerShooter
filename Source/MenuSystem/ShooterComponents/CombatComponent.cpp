// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "Camera/CameraComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MenuSystem/MenuSystem.h"
#include "MenuSystem/Weapon/Weapon.h"
#include "MenuSystem/Character/ShooterCharacter.h"
#include "MenuSystem/HUD/ShooterHUD.h"
#include "MenuSystem/PlayerController/ShooterPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "MenuSystem/GameModes/ShooterGameMode.h"
#include "Sound/SoundCue.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;

	SprintSpeed = 600.f;
	BaseWalkSpeed = 300.f;
	AimWalkSpeed = 300.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME_CONDITION(UCombatComponent, MaxWeaponAmmo, COND_OwnerOnly);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
			
			DefaultCameraLocation = Character->GetFollowCamera()->GetRelativeLocation();
			CurrentCameraLocation = DefaultCameraLocation;
		}

		if (Character->HasAuthority())
		{
			InitializeMaxAmmo();
		}
	}
	SpawnDefaultWeapon();
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if(Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceToShoot(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	if(EquippedWeapon == nullptr) return;
	
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::SprintButtonPressed(bool bPressed)
{
	if(!bAiming)
	{
		bSprintButtonPressed = bPressed;

		ServerSetSprinting(bPressed);

		if (Character && !Character->bIsCrouched)
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = bSprintButtonPressed ? SprintSpeed : BaseWalkSpeed;
		}
	}
}

void UCombatComponent::ServerSetSprinting_Implementation(bool bIsSprinting)
{
	bSprintButtonPressed = bIsSprinting;
	
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bSprintButtonPressed ? SprintSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::Fire()
{
	if(!Character) return;
	
	if(!bSprintButtonPressed && !Character->GetCharacterMovement()->IsFalling())
	{
		if(CanFire())
		{
			if (!Character->HasAuthority())
			{
				ServerFire(HitTarget);
			}

			LocalFire(HitTarget);
			if(EquippedWeapon)
			{
				bCanFire = false;
				CrosshairShootFactor = CrosshairShootFactorTarget;
			}
			StartFireTimer();
		}
	}
}

bool UCombatComponent::CanFire()
{
	if (!EquippedWeapon) return false;
	if(bLocallyReloading) return false;
	if (EquippedWeapon->IsAmmoEmpty()) return false;
	if(!bCanFire) return false;
	if(CombatState == ECombatState::ECS_Reloading) return false;
	
	return true;
}

void UCombatComponent::InitializeMaxAmmo()
{
	MaxAmmoMap.Emplace(EWeaponType::EWT_Rifle, 120);
	MaxAmmoMap.Emplace(EWeaponType::EWT_Pistol, 60);
}

void UCombatComponent::OnRep_MaxWeaponAmmo()
{
	Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDWeaponMaxAmmo(MaxWeaponAmmo);
	}
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;

	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;
	
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if(Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	
	LocalFire(TraceHitTarget);
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if(EquippedWeapon == nullptr) return;
	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->FireWeapon(TraceHitTarget);
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	if (EquippedWeapon && EquippedWeapon->bDestroyWeapon)
	{
		EquippedWeapon->Destroy();
	}
	else if (EquippedWeapon)
	{
		EquippedWeapon->WeaponDropped();
	}

	EquippedWeapon = WeaponToEquip;

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}

	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();

	if (MaxAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		MaxWeaponAmmo = MaxAmmoMap[EquippedWeapon->GetWeaponType()];
	}
	
	Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDWeaponMaxAmmo(MaxWeaponAmmo);
		Controller->SetHUDWeaponType(EquippedWeapon->GetWeaponType());
	}

	if (EquippedWeapon->EquipSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			EquippedWeapon->EquipSound,
			Character->GetActorLocation()
		);
	}
	
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::ReloadWeapon()
{
	// we call server rpc is there is a point
	if(EquippedWeapon == nullptr) return;

	if (MaxWeaponAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon->GetAmmo() < EquippedWeapon->
		GetMagCapacity() && !bLocallyReloading)
	{
		ServerReload();
		HandleReload();
		bLocallyReloading = true;
	}
}

void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	
	int32 ReloadAmount = AmountToReload();

	if (MaxAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		MaxAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		MaxWeaponAmmo = MaxAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDWeaponMaxAmmo(MaxWeaponAmmo);
	}
	
	EquippedWeapon->AddAmmo(ReloadAmount);
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	
	CombatState = ECombatState::ECS_Reloading;
	if(!Character->IsLocallyControlled()) HandleReload();
}

void UCombatComponent::FinishReloading()
{
	if (Character == nullptr) return;
	bLocallyReloading = false;
	if(Character->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		UpdateAmmoValues();
	}
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		if(Character && !Character->IsLocallyControlled()) HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFireButtonPressed)
		{
			Fire();
		}
		break;
	}
}

void UCombatComponent::HandleReload()
{
	if (Character)
	{
		Character->PlayReloadMontage();
	}
}

int32 UCombatComponent::AmountToReload()
{
	if(EquippedWeapon == nullptr) return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo();
	
	if (MaxAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountMaxAmmo = MaxAmmoMap[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmountMaxAmmo);
		return FMath::Clamp(RoomInMag, 0, Least);
	}
	return 0;
}

void UCombatComponent::SpawnDefaultWeapon()
{
	AShooterGameMode* ShooterGameMode = Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(this));
	UWorld* World = GetWorld();

	if (ShooterGameMode && World && Character && !Character->IsCharacterEliminated() && DefaultWeapon)
	{
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeapon);
		StartingWeapon->bDestroyWeapon = true;
		EquipWeapon(StartingWeapon);
	}
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDWeaponType(EquippedWeapon->GetWeaponType());
		}
		const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		}
		
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;

		if (EquippedWeapon->EquipSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				EquippedWeapon->EquipSound,
				Character->GetActorLocation()
			);
		}
	}
}

void UCombatComponent::TraceToShoot(FHitResult& TraceHitResult)
{
	FVector2D ViewPortSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewPortSize);
	}

	FVector2D CenterOfViewport(ViewPortSize.X / 2.f, ViewPortSize.Y/ 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CenterOfViewport,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);
	
	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;
		
		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 200.f);
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECC_SkeletalMesh
		);

		if(!TraceHitResult.bBlockingHit) TraceHitResult.ImpactPoint = End;
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;
	
	Controller = Controller == nullptr ? Cast<AShooterPlayerController>(Character->Controller) : Controller;

	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<AShooterHUD>(Controller->GetHUD()) : HUD;

		if (HUD)
		{
			if(!Character || !EquippedWeapon) return;
			
			FHUDPackage HUDPackage;
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;
			float Speed = Velocity.Size();

			bool bShowCrosshair;

			bShowCrosshair = !bSprintButtonPressed && !Character->GetCharacterMovement()->IsFalling() ? true : false;

			if(bShowCrosshair)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
			}
			// Crosshair spread

			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(
				WalkSpeedRange, VelocityMultiplierRange, Speed);

			// Jump
			/*
			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(
					CrosshairInAirFactor, CrosshairInAirFactorTarget, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(
					CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}*/

			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, CrosshairAimFactorTarget, DeltaTime, 30.f);
			}
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			CrosshairShootFactor = FMath::FInterpTo(CrosshairShootFactor, 0.f, DeltaTime, 40.f);

			HUDPackage.CrosshairSpread =
				CrosshairBaseValue +
					CrosshairVelocityFactor +
						CrosshairInAirFactor +
							CrosshairAimFactor +
								CrosshairShootFactor;
			
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;

	if(bAiming)
	{
		CurrentFOV = FMath::FInterpTo(
			CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());

		if(Character && Character->bIsCrouched)
		{
			CurrentCameraLocation = FMath::VInterpTo(
				CurrentCameraLocation,
				EquippedWeapon->GetZoomedCrouchCameraLocation(),
				DeltaTime,
				EquippedWeapon->GetZoomInterpSpeed()
			);
		}
		else
		{
			CurrentCameraLocation = FMath::VInterpTo(
				CurrentCameraLocation,
				EquippedWeapon->GetZoomedCameraLocation(),
				DeltaTime,
				EquippedWeapon->GetZoomInterpSpeed()
			);
		}
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
		CurrentCameraLocation = FMath::VInterpTo(CurrentCameraLocation, DefaultCameraLocation, DeltaTime,
													 EquippedWeapon->GetZoomInterpSpeed());
	}

	if(Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
		Character->GetFollowCamera()->SetRelativeLocation(CurrentCameraLocation);
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	if(!bSprintButtonPressed)
	{
		bAiming = bIsAiming;
		ServerSetAiming(bIsAiming);
	
		if (Character && !Character->bIsCrouched)
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
		}
		
		bAimButtonPressed = bIsAiming;
	}
}

void UCombatComponent::OnRep_Aiming()
{
	if (Character && Character->IsLocallyControlled())
	{
		bAiming = bAimButtonPressed;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

