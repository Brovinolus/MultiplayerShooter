// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingCharacterAnimInstance.h"
#include "KismetAnimationLibrary.h"
#include "ShootingCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MenuSystem/Weapon/Weapon.h"

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
	Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, YawRotation);

	bIsInAir = ShootingCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = ShootingCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bIsWeaponEquipped = ShootingCharacter->IsWeaponEquipped();
	EquippedWeapon = ShootingCharacter->GetEquippedWeapon();
	bIsCrouched = ShootingCharacter->bIsCrouched;
	bIsAiming = ShootingCharacter->IsAiming();
	AimingYawRotation = ShootingCharacter->GetAimingYawRotation();
	AimingPitchRotation = ShootingCharacter->GetAimingPitchRotation();

	if(bIsWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && ShootingCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(
			FName("LeftHandSocket"), RTS_World);

		FVector OutPosition;
		FRotator OutRotation;
		
		ShootingCharacter->GetMesh()->TransformToBoneSpace(
			FName("hand_r"), LeftHandTransform.GetLocation(),
			FRotator::ZeroRotator, OutPosition, OutRotation);

		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
	}
}
