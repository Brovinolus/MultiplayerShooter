// Fill out your copyright notice in the Description page of Project Settings.


#include "ShootingCharacterAnimInstance.h"
#include "KismetAnimationLibrary.h"
#include "ShootingCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
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

	bIsInAir = ShootingCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = ShootingCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bIsWeaponEquipped = ShootingCharacter->IsWeaponEquipped();
	EquippedWeapon = ShootingCharacter->GetEquippedWeapon();
	bIsCrouched = ShootingCharacter->bIsCrouched;
	bIsAiming = ShootingCharacter->IsAiming();
	TurningInPlace = ShootingCharacter->GetTurningInPlace();
	bCharacterEiliminated = ShootingCharacter->IsCharacterEliminated();

	// Direction
	FRotator Rotation = ShootingCharacter->GetBaseAimRotation();
	FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
	Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, YawRotation);
	//UE_LOG(LogTemp, Warning, TEXT("Direction: %f"), Direction);

	//Yaw Offset for Strafing
	FRotator AimRotation = ShootingCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShootingCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	//DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaSeconds, 5.f);
	YawOffset = DeltaRot.Yaw;
	
	// Leaning
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = ShootingCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(
		CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaSeconds;
	const  float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);
	
	float MaxSpeed = ShootingCharacter->GetBaseWalkSpeed();
	// alpha from 0 to 0.4
	AlphaMoveSpeed = UKismetMathLibrary::MapRangeClamped(
	Speed, 0.f, MaxSpeed, 0.f, 0.4f);

	// Aim Offsets
	AimingYawRotation = ShootingCharacter->GetAimingYawRotation();
	AimingPitchRotation = ShootingCharacter->GetAimingPitchRotation();

	if(bIsWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && ShootingCharacter->GetMesh())
	{
		// Left hand
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(
			FName("LeftHandSocket"), RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		ShootingCharacter->GetMesh()->TransformToBoneSpace(
			FName("weapon_r"), LeftHandTransform.GetLocation(),
			FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		// Right hand transform
		FTransform RightHandTransform = ShootingCharacter->GetMesh()->GetSocketTransform(
			FName("weapon_r"), RTS_World);

		if(ShootingCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			
			// Right hand rotation
			RightHandRotation = UKismetMathLibrary::FindLookAtRotation(
				RightHandTransform.GetLocation(), ShootingCharacter->GetHitTarget());

			// weapon rotation
			FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(
				FName("Muzzle", RTS_World));

			FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));

			DrawDebugLine(
				GetWorld(), MuzzleTipTransform.GetLocation(),
				MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f, FColor::Red);

			DrawDebugLine(
				GetWorld(), MuzzleTipTransform.GetLocation(),
				ShootingCharacter->GetHitTarget(), FColor::Green);
		}
	}
}
