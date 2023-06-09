// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MenuSystem/ShooterTypes/TurningInPlace.h"
#include "MenuSystem/Weapon/Weapon.h"
#include "ShootingCharacterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class UShootingCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	virtual  void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = true))
	TObjectPtr<AShooterCharacter> ShooterCharacter;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	float Direction;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	float AimingYawRotation;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	float AimingPitchRotation;
	
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bIsWeaponEquipped;

	UPROPERTY()
	TObjectPtr<AWeapon> EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bIsCrouched;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bIsAiming;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	FTransform LeftHandTransform;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	ETurningInPlace TurningInPlace;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	EWeaponType WeaponType;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	float YawOffset;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	float Lean;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	float AlphaMoveSpeed;

	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	FRotator DeltaRotation;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	FRotator RightHandRotation;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bLocallyControlled;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bCharacterEiliminated;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bUseFabrik;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bUseAimOffsets;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = true))
	bool bTransformRightHand;
};
