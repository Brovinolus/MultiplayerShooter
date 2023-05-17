// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "MenuSystem/ShooterComponents/CombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "MenuSystem/Weapon/Weapon.h"
#include "Kismet/KismetMathLibrary.h"

AShootingCharacter::AShootingCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 500.f;
	CameraBoom->bUsePawnControlRotation = true;
	
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	
	//bUseControllerRotationYaw = false;
	//GetCharacterMovement()->bOrientRotationToMovement = true;

	OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHeadWidget"));
	OverHeadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	if(Combat)
	{
		BaseWalkSpeed = Combat->BaseWalkSpeed;
		AimWalkSpeed = Combat->AimWalkSpeed;
	}

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->NavAgentProps.bCanFly = false;
	GetCharacterMovement()->AirControl = false;

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
	MaxAimingPitchAngle = 75.f;
	AngleToTurn = 75.f;
}

void AShootingCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AShootingCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void AShootingCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AShootingCharacter::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);
}

void AShootingCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShootingCharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AShootingCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShootingCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AShootingCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AShootingCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AShootingCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AShootingCharacter::FireButtonReleased);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AShootingCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShootingCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AShootingCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShootingCharacter::LookUp);
}

void AShootingCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}

void AShootingCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && FireWeaponMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT("FireWeaponMontage"));
		AnimInstance->Montage_Play(FireWeaponMontage);
	}
}

void AShootingCharacter::FireButtonPressed()
{
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void AShootingCharacter::FireButtonReleased()
{
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void AShootingCharacter::MoveForward(float Value)
{
	if (Controller!=nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void AShootingCharacter::MoveRight(float Value)
{
	if (Controller!=nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void AShootingCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AShootingCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AShootingCharacter::EquipButtonPressed()
{
	if (Combat)
	{
		if (HasAuthority())
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			// calling the function from a client
			// calling without the implementation, it is only for defining
			ServerEquipButtonPressed();
		}
	}
}

void AShootingCharacter::CrouchButtonPressed()
{
	if(GetCharacterMovement()->IsFalling()) return;
	
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

void AShootingCharacter::AimButtonPressed()
{
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void AShootingCharacter::AimButtonReleased()
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void AShootingCharacter::AimOffset(float DeltaTime)
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();

	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir)
	{
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(
			CurrentAimRotation, StartingAimRotation);
		AimingYawRotation = DeltaAimRotation.Yaw;

		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAimingYaw = AimingYawRotation;
		}
		
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}

	if (Speed > 0.f || bIsInAir)
	{
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AimingYawRotation = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	// it behaves not as intended because of compression (look up bitwise operations)
	// the value for the pitch was put into an unsigned form
	AimingPitchRotation = GetBaseAimRotation().Pitch;

	/*
	if (AimingPitchRotation > MaxAimingPitchAngle || AimingPitchRotation < MaxAimingPitchAngle)
	{
		AimingPitchRotation = FMath::Clamp(AimingPitchRotation, -MaxAimingPitchAngle, MaxAimingPitchAngle);
	}
	*/

	if (AimingPitchRotation > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AimingPitchRotation = FMath::GetMappedRangeValueClamped(
			InRange, OutRange, AimingPitchRotation);
	}

	// get the data that server gets from the client
	/*
	if (HasAuthority() && !IsLocallyControlled())
	{
		UE_LOG(LogTemp, Warning, TEXT("AimingPitchRotation: %f"), AimingPitchRotation);
	}*/
}

void AShootingCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void AShootingCharacter::TurnInPlace(float DeltaTime)
{
	//UE_LOG(LogTemp, Warning, TEXT("Yaw: %f"), AimingYawRotation);
	if (AimingYawRotation > AngleToTurn)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if(AimingYawRotation < -AngleToTurn)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}

	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAimingYaw = FMath::FInterpTo(InterpAimingYaw, 0.f, DeltaTime, 4.f);
		AimingYawRotation = InterpAimingYaw;

		if (FMath::Abs(AimingYawRotation) < 5.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void AShootingCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void AShootingCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(false);
		}
	}
	
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool AShootingCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool AShootingCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

void AShootingCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}

	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

TObjectPtr<AWeapon> AShootingCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector AShootingCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}



