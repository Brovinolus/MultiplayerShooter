// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MenuSystem/ShooterTypes/TurningInPlace.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AShooterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayDeathMontage();
	void CharacterEliminated();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastCharacterEliminated();
	
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastHit(const FVector_NetQuantize& ImpactPoint);

	void UpdateHUDHealth();

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
	void SprintButtonPressed();
	void SprintButtonReleased();
	void ReloadButtonPressed();
	void PlayHitReactMontage();
	void GetShooterPlayerState();
	UFUNCTION()
	void ReceiveDamage(AActor* DamageActor, float Damage, const UDamageType* DamageType,
	                   class AController* InstigatorController, AActor* DamageCauser);
	
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
	TObjectPtr<UAnimMontage> RifleReloadMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	TObjectPtr<UAnimMontage> PistolReloadMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<TObjectPtr<UAnimMontage>> HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	TArray<TObjectPtr<UAnimMontage>> DeathMontage;

	UPROPERTY(EditAnywhere, Category = TurningAnimation)
	float AngleToTurn;

	UPROPERTY(EditAnywhere, Category = AimingAnimation)
	float MaxAimingPitchAngle;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UParticleSystem> CharacterImpactParticles;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class USoundCue> CharacterImpactSounds;

	UPROPERTY()
	TObjectPtr<class AShooterPlayerController> ShooterPlayerController;

	UPROPERTY()
	TObjectPtr<class AShooterPlayerState> ShooterPlayerState;
	
	float BaseWalkSpeed;
	float AimWalkSpeed;
	float SprintSpeed;

	UPROPERTY(EditAnywhere)
	float JumpDelay = 1.f;
	bool bCanJump = true;
	
	FTimerHandle JumpTimer;
	void StartJumpTimer();
	void JumpTimerFinished();
	
	void HideCharacterIfCameraClose();
	UPROPERTY(EditAnywhere)
	float CameraHideThreshold = 200.f;

	/**
	 * PlayerHealth
	 */

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();

	UPROPERTY()
	TObjectPtr<AShooterPlayerController> VictimController;
	bool bCharacterEliminated = false;

	FTimerHandle CharacterEliminatedTimer;

	UPROPERTY(EditDefaultsOnly)
	float CharacterEliminatedDelay = 1.f;

	void CharacterEliminatedTimerFinished();
	
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
	FORCEINLINE bool IsCharacterEliminated() const { return bCharacterEliminated; }
};
