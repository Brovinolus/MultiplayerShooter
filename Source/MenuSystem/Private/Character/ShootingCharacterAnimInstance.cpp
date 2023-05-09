// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/ShootingCharacterAnimInstance.h"
#include "KismetAnimationLibrary.h"
#include "ShootingCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UShootingCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ShootingCharacter = Cast<AShootingCharacter>(TryGetPawnOwner());
}

void UShootingCharacterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (ShootingCharacter == nullptr)
	{
		ShootingCharacter = Cast<AShootingCharacter>(TryGetPawnOwner());
	}

	if(ShootingCharacter == nullptr) return;

	FVector Velocity = ShootingCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	FRotator Rotation = ShootingCharacter->GetBaseAimRotation();
	FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
	Direction = CalculateDirection(Velocity, YawRotation);

	bIsInAir = ShootingCharacter->GetCharacterMovement()->IsFalling();

	bIsAccelerating = ShootingCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	
}
