// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "MenuSystem/ShooterComponents/CombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "MenuSystem/Weapon/Weapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "MenuSystem/MenuSystem.h"
#include "MenuSystem/GameModes/ShooterGameMode.h"
#include "MenuSystem/PlayerController/ShooterPlayerController.h"
#include "MenuSystem/ShooterState/ShooterPlayerState.h"

AShooterCharacter::AShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
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
	
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->NavAgentProps.bCanFly = false;
	GetCharacterMovement()->AirControl = false;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
	MaxAimingPitchAngle = 75.f;
	AngleToTurn = 75.f;
	
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	if(Combat)
	{
		BaseWalkSpeed = Combat->BaseWalkSpeed;
		AimWalkSpeed = Combat->AimWalkSpeed;
		SprintSpeed = Combat->SprintSpeed;
	}
}

void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AShooterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AShooterCharacter, Health);
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateHUDHealth();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AShooterCharacter::ReceiveDamage);
	}
}

void AShooterCharacter::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);

	GetShooterPlayerState();
	AimOffset(DeltaTime);
	HideCharacterIfCameraClose();
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AShooterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AShooterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &AShooterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &AShooterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AShooterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AShooterCharacter::SprintButtonPressed);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AShooterCharacter::SprintButtonReleased);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);
}

void AShooterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}
}

void AShooterCharacter::PlayFireMontage(bool bAiming)
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
	}
}

void AShooterCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_Pistol:
			if (PistolReloadMontage)
			{
				AnimInstance->Montage_Play(PistolReloadMontage);
			}	
			break;
		case EWeaponType::EWT_Rifle:
			if (PistolReloadMontage)
			{
				AnimInstance->Montage_Play(PistolReloadMontage);
			}
			break;
		}
	}
}

void AShooterCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance)
	{
		bool bNotEmpty = HitReactMontage.Num() > 0;

		if (bNotEmpty)
		{
			int32 RandomIndex = FMath::RandRange(0, HitReactMontage.Num() - 1);
			UAnimMontage* RandomMontage = HitReactMontage[RandomIndex];

			if (RandomMontage)
			{
				AnimInstance->Montage_Play(RandomMontage);
			}
		}
	}
}

void AShooterCharacter::PlayDeathMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance)
	{
		bool bNotEmpty = DeathMontage.Num() > 0;

		if (bNotEmpty)
		{
			int32 RandomIndex = FMath::RandRange(0, DeathMontage.Num() - 1);
			UAnimMontage* RandomMontage = DeathMontage[RandomIndex];

			if (RandomMontage)
			{
				AnimInstance->Montage_Play(RandomMontage);
			}
		}
	}
}

void AShooterCharacter::SprintButtonPressed()
{
	if (Combat)
	{
		Combat->SprintButtonPressed(true);
	}
}

void AShooterCharacter::SprintButtonReleased()
{
	if (Combat)
	{
		Combat->SprintButtonPressed(false);
	}
}

void AShooterCharacter::ReloadButtonPressed()
{
	if (Combat)
	{
		Combat->ReloadWeapon();
	}
}

void AShooterCharacter::FireButtonPressed()
{
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void AShooterCharacter::FireButtonReleased()
{
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void AShooterCharacter::MoveForward(float Value)
{
	if (Controller!=nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if (Controller!=nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.0f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value);
}

void AShooterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AShooterCharacter::EquipButtonPressed()
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

void AShooterCharacter::CrouchButtonPressed()
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

void AShooterCharacter::AimButtonPressed()
{
	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

void AShooterCharacter::AimButtonReleased()
{
	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void AShooterCharacter::AimOffset(float DeltaTime)
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

void AShooterCharacter::Jump()
{
	if(!bCanJump) return;

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();

		bCanJump = false;
	
		StartJumpTimer();
	}
}

void AShooterCharacter::StartJumpTimer()
{
	GetWorldTimerManager().SetTimer(
		JumpTimer,
		this,
		&AShooterCharacter::JumpTimerFinished,
		JumpDelay
	);
}

void AShooterCharacter::JumpTimerFinished()
{
	bCanJump = true;
}

void AShooterCharacter::TurnInPlace(float DeltaTime)
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

void AShooterCharacter::MulticastHit_Implementation(const FVector_NetQuantize& HitLocation)
{
	/*
	if(CharacterImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), CharacterImpactParticles, HitLocation);
	}

	if (CharacterImpactSounds)
	{
		UGameplayStatics::PlaySoundAtLocation(this, CharacterImpactSounds, HitLocation);
	}
	
	PlayHitReactMontage();
	*/
}

void AShooterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void AShooterCharacter::HideCharacterIfCameraClose()
{
	if (!IsLocallyControlled()) return;

	bool MeshVisibility = ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size()
		< CameraHideThreshold) ? false : true; 
	GetMesh()->SetVisibility(MeshVisibility);

	//UE_LOG(LogTemp, Warning, TEXT("Visible: %s"), MeshVisibility ? TEXT("true") : TEXT("false"));
	
	if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
	{
		Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = !MeshVisibility;
		Combat->EquippedWeapon->SetCanShowParticlesInFireAnimation(MeshVisibility);
	}
}

void AShooterCharacter::ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType,
	AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
	PlayHitReactMontage();

	if(Health == 0.f)
	{
		AShooterGameMode* ShooterGameMode = GetWorld()->GetAuthGameMode<AShooterGameMode>();
		if (ShooterGameMode)
		{
			VictimController = VictimController == nullptr
				                          ? Cast<AShooterPlayerController>(Controller)
				                          : VictimController;
			AShooterPlayerController* AttackerController = Cast<AShooterPlayerController>(InstigatorController);
			ShooterGameMode->PlayerEliminated(this, VictimController, AttackerController);
		}
	}
}

// calling it in the GameMode, which means we are calling it on the server
void AShooterCharacter::CharacterEliminated()
{
	if (Combat && Combat->EquippedWeapon)
	{
		if (Combat->EquippedWeapon->bDestroyWeapon)
		{
			Combat->EquippedWeapon->Destroy();
		}
		else
		{
			Combat->EquippedWeapon->WeaponDropped();
		}
	}
	
	MulticastCharacterEliminated();
	GetWorldTimerManager().SetTimer(
		CharacterEliminatedTimer,
		this,
		&AShooterCharacter::CharacterEliminatedTimerFinished,
		CharacterEliminatedDelay);
}

void AShooterCharacter::MulticastCharacterEliminated_Implementation()
{
	if (ShooterPlayerController)
	{
		ShooterPlayerController->SetHUDWeaponAmmo(0);
	}
	
	bCharacterEliminated = true;
	PlayDeathMontage();

	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	if (ShooterPlayerController)
	{
		DisableInput(ShooterPlayerController);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AShooterCharacter::CharacterEliminatedTimerFinished()
{
	AShooterGameMode* ShooterGameMode = GetWorld()->GetAuthGameMode<AShooterGameMode>();
	if (ShooterGameMode)
	{
		// we check controller in the GameMode
		ShooterGameMode->RequestRespawn(this, Controller);
	}
}

void AShooterCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();
}

void AShooterCharacter::UpdateHUDHealth()
{
	VictimController = VictimController == nullptr
								  ? Cast<AShooterPlayerController>(Controller)
								  : VictimController;
	if (VictimController)
	{
		VictimController->SetHUDHealth(Health, MaxHealth);
	}
}

void AShooterCharacter::GetShooterPlayerState()
{
	if (ShooterPlayerState == nullptr)
	{
		ShooterPlayerState = GetPlayerState<AShooterPlayerState>();
		
		if (ShooterPlayerState)
		{
			ShooterPlayerState->AddToScore(0.f);
			ShooterPlayerState->AddToDeaths(0);
		}
	}
}

void AShooterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
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

bool AShooterCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool AShooterCharacter::IsAiming()
{
	return (Combat && Combat->bAiming);
}

void AShooterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
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

TObjectPtr<AWeapon> AShooterCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector AShooterCharacter::GetHitTarget() const
{
	if (Combat == nullptr) return FVector();
	return Combat->HitTarget;
}



