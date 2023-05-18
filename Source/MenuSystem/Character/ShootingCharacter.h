// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MenuSystem/ShooterTypes/TurningInPlace.h"
#include "ShootingCharacter.generated.h"

UCLASS()
class AShootingCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AShootingCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastHit(const FVector_NetQuantize& ImpactPoint);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void AimOffset(float DeltaTime);
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();
	void PlayHitReactMontage();
	
private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<class USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	TObjectPtr<class UCameraComponent> FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UWidgetComponent> OverHeadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	TObjectPtr<class AWeapon> OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class UCombatComponent> Combat;
	
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();
	
	float AimingYawRotation;
	float InterpAimingYaw;
	float AimingPitchRotation;
	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = Combat)
	TObjectPtr<UAnimMontage> FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<TObjectPtr<UAnimMontage>> HitReactMontage;

	UPROPERTY(EditAnywhere, Category = TurningAnimation)
	float AngleToTurn;

	UPROPERTY(EditAnywhere, Category = AimingAnimation)
	float MaxAimingPitchAngle;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UParticleSystem> CharacterImpactParticles;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class USoundCue> CharacterImpactSounds;
	
	float BaseWalkSpeed;
	float AimWalkSpeed;

	void HideCharacterIfCameraClose();
	UPROPERTY(EditAnywhere)
	float CameraHideThreshold = 200.f;

public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	FORCEINLINE float GetAimingYawRotation() const { return AimingYawRotation; }
	FORCEINLINE float GetAimingPitchRotation() const { return AimingPitchRotation; }
	TObjectPtr<AWeapon> GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE float GetBaseWalkSpeed() const { return BaseWalkSpeed; }
	FORCEINLINE float GetAimWalkSpeed() const { return AimWalkSpeed; }
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
};
