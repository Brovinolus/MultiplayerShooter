// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/SpringArmComponent.h"

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
}

void AShootingCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AShootingCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	
	PlayerInputComponent->BindAxis("MoveForward", this, &AShootingCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShootingCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &AShootingCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShootingCharacter::LookUp);
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

void AShootingCharacter::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
}



