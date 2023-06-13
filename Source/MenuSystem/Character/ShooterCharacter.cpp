// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
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
#include "MenuSystem/ShooterComponents/LagCompensationComponent.h"
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
	
	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	if(Combat)
	{
		BaseWalkSpeed = Combat->BaseWalkSpeed;
		AimWalkSpeed = Combat->AimWalkSpeed;
		SprintSpeed = Combat->SprintSpeed;
	}

	// Hit capsules for server-side rewind
	
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	BoxCollision.Add(FName("head"), head);

	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	BoxCollision.Add(FName("pelvis"), pelvis);

	spine_04 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_04"));
	spine_04->SetupAttachment(GetMesh(), FName("spine_04"));
	BoxCollision.Add(FName("spine_04"), spine_04);

	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	BoxCollision.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	BoxCollision.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	BoxCollision.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	BoxCollision.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	BoxCollision.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	BoxCollision.Add(FName("hand_r"), lowerarm_r);

	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	BoxCollision.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	BoxCollision.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	BoxCollision.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	BoxCollision.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	BoxCollision.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	BoxCollision.Add(FName("foot_r"), foot_r);

	for (auto& Box : BoxCollision)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox);
			Box.Value->SetCollisionResponseToAllChannels(ECR_Ignore);
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	/*
	neck_01 = CreateDefaultSubobject<UCapsuleComponent>(TEXT("neck_01"));
	neck_01->SetupAttachment(GetMesh(), FName("neck_01"));
	neck_01->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleCollision.Add(FName("neck_01"), neck_01);
	
	neck_02 = CreateDefaultSubobject<UCapsuleComponent>(TEXT("neck_02"));
	neck_02->SetupAttachment(GetMesh(), FName("neck_02"));
	neck_02->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleCollision.Add(FName("neck_02"), neck_02);

	pelvis = CreateDefaultSubobject<UCapsuleComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	pelvis->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleCollision.Add(FName("pelvis"), pelvis);

	spine_02 = CreateDefaultSubobject<UCapsuleComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	spine_02->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleCollision.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UCapsuleComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	spine_03->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleCollision.Add(FName("spine_03"), spine_03);

	spine_04 = CreateDefaultSubobject<UCapsuleComponent>(TEXT("spine_04"));
	spine_04->SetupAttachment(GetMesh(), FName("spine_04"));
	spine_04->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleCollision.Add(FName("spine_04"), spine_04);

	spine_05 = CreateDefaultSubobject<UCapsuleComponent>(TEXT("spine_05"));
	spine_05->SetupAttachment(GetMesh(), FName("spine_05"));
	spine_05->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleCollision.Add(FName("spine_05"), spine_05);

	upperarm_l = CreateDefaultSubobject<UCapsuleComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	upperarm_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleCollision.Add(FName("upperarm_l"), upperarm_l);

	upperarm_r = CreateDefaultSubobject<UCapsuleComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	upperarm_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleCollision.Add(FName("upperarm_r"), upperarm_r);

	clavicle_l = CreateDefaultSubobject<UCapsuleComponent>(TEXT("clavicle_l"));
	clavicle_l->SetupAttachment(GetMesh(), FName("clavicle_l"));
	clavicle_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleCollision.Add(FName("clavicle_l"), clavicle_l);

	clavicle_r = CreateDefaultSubobject<UCapsuleComponent>(TEXT("clavicle_r"));
	clavicle_r->SetupAttachment(GetMesh(), FName("clavicle_r"));
	clavicle_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleCollision.Add(FName("clavicle_r"), clavicle_r);

	lowerarm_l = CreateDefaultSubobject<UCapsuleComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	lowerarm_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleCollision.Add(FName("lowerarm_l"), lowerarm_l);

	lowerarm_r = CreateDefaultSubobject<UCapsuleComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	lowerarm_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleCollision.Add(FName("lowerarm_r"), lowerarm_r);

	hand_l = CreateDefaultSubobject<UCapsuleComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	hand_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleCollision.Add(FName("hand_l"), hand_l);

	hand_r = CreateDefaultSubobject<UCapsuleComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	hand_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleCollision.Add(FName("hand_r"), lowerarm_r);

	thigh_l = CreateDefaultSubobject<UCapsuleComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	thigh_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleCollision.Add(FName("thigh_l"), thigh_l);

	thigh_r = CreateDefaultSubobject<UCapsuleComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	thigh_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleCollision.Add(FName("thigh_r"), thigh_r);

	calf_l = CreateDefaultSubobject<UCapsuleComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	calf_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleCollision.Add(FName("calf_l"), calf_l);

	calf_r = CreateDefaultSubobject<UCapsuleComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	calf_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleCollision.Add(FName("calf_r"), calf_r);

	foot_l = CreateDefaultSubobject<UCapsuleComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	foot_l->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleCollision.Add(FName("foot_l"), foot_l);

	foot_r = CreateDefaultSubobject<UCapsuleComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	foot_r->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CapsuleCollision.Add(FName("foot_r"), foot_r);*/
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

	//StorePhysicsAsset();
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

	if (LagCompensation)
	{
		//StorePhysicsAsset();
		//SetHitCapsuleSize();
		
		LagCompensation->Character = this;
		if (Controller)
		{
			LagCompensation->Controller = Cast<AShooterPlayerController>(Controller);
		}
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
				AnimInstance->Montage_Play(RifleReloadMontage);
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

void AShooterCharacter::StorePhysicsAsset()
{
	UPhysicsAsset* PhysicsAsset = GetMesh()->GetPhysicsAsset();

	if (PhysicsAsset)
	{
		for (const auto& SkeletalBody : PhysicsAsset->SkeletalBodySetups)
		{
			FPhysicAssetElement PhysicAssetElement;
			const FName& BName = SkeletalBody->BoneName;
			const FTransform& BoneWorldTransform = GetMesh()->GetBoneTransform(GetMesh()->GetBoneIndex(BName));
			PhysicAssetElement.BoneTransform = BoneWorldTransform;
			for (const auto& Capsule : SkeletalBody->AggGeom.SphylElems)
			{
				PhysicAssetElement.CapsuleInfo = Capsule;
				HitCollisionData.Add(BName, PhysicAssetElement);
			}
		}
	}
}

/*
void AShooterCharacter::SetHitCapsuleSize()
{
	for (const auto& PACapsule : HitCollisionData)
	{
		for (auto& CollisionPair : CapsuleCollision)
		{
			if (CollisionPair.Key == PACapsule.Key)
			{
				UE_LOG(LogTemp, Warning, TEXT("PACapsule.Key: %s"), CapsuleCollision.Contains(PACapsule.Key) ? TEXT("true") : TEXT("false"));
				UE_LOG(LogTemp, Warning, TEXT("CollisionPair.Key: %s"), *CollisionPair.Key.ToString());
				
				//UCapsuleComponent* HitCapsuleComponent = CollisionPair.Value;

				if(CollisionPair.Value)
				{
					UE_LOG(LogTemp, Warning, TEXT("PACapsule.Key: %s"), *PACapsule.Key.ToString());
					UE_LOG(LogTemp, Warning, TEXT("PACapsule.Radius: %f"), PACapsule.Value.CapsuleInfo.Radius);
					UE_LOG(LogTemp, Warning, TEXT("CollisionPair: %s"), *CollisionPair.Value->GetName());
					UE_LOG(LogTemp, Warning, TEXT("CollisionPair Radius: %f"), CollisionPair.Value->GetScaledCapsuleRadius());
					UCapsuleComponent* HitCapsuleComponent = nullptr;
					FName CaspuleName = CollisionPair.Key;
					HitCapsuleComponent = NewObject<UCapsuleComponent>(this, CaspuleName);
					HitCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
					HitCapsuleComponent->SetCapsuleRadius(PACapsule.Value.CapsuleInfo.Radius);
					HitCapsuleComponent->SetCapsuleHalfHeight(PACapsule.Value.CapsuleInfo.Length / 2);
					HitCapsuleComponent->SetWorldRotation(PACapsule.Value.CapsuleInfo.Rotation);
					HitCapsuleComponent->RegisterComponent();
					HitCapsuleComponent->SetupAttachment(GetMesh(), FName(CaspuleName));

					CapsuleCollisionCopy.Add(CollisionPair.Key, HitCapsuleComponent);
					
					//CollisionPair.Value->SetCapsuleRadius(PACapsule.Value.CapsuleInfo.Radius);
					//CollisionPair.Value->SetCapsuleHalfHeight(PACapsule.Value.CapsuleInfo.Length / 2);
					//CollisionPair.Value->SetWorldRotation(PACapsule.Value.CapsuleInfo.Rotation);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("CollisionPair.Value: nullptr"));
				}
			}
		}
		
		for (auto Capsule : CapsuleCollision)
		{
			if(Capsule.Key == PACapsule.Key)
			{
				UE_LOG(LogTemp, Warning, TEXT("Capsule.Key: %s"), *Capsule.Key.ToString());
				UE_LOG(LogTemp, Warning, TEXT("PACapsule.Key: %s"), *PACapsule.Key.ToString());
				UE_LOG(LogTemp, Warning, TEXT("Radius %f"), PACapsule.Value.CapsuleInfo.Radius);
				Capsule.Value->SetCapsuleRadius(PACapsule.Value.CapsuleInfo.Radius);
				Capsule.Value->SetCapsuleHalfHeight(PACapsule.Value.CapsuleInfo.Length / 2);
				Capsule.Value->SetWorldRotation(PACapsule.Value.CapsuleInfo.Rotation);
			}
		}
	}
}*/

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

bool AShooterCharacter::CanJump()
{
	return bCanJump;
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

ECombatState AShooterCharacter::GetCombatState() const
{
	if (Combat == nullptr) ECombatState::ECS_MAX;
	return Combat->CombatState;
}

bool AShooterCharacter::IsLocallyReloading()
{
	if (Combat == nullptr) return false;
	return Combat->bLocallyReloading;
}



