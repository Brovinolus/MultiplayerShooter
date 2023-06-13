// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MenuSystem/ShooterTypes/TurningInPlace.h"
#include "MenuSystem/ShooterTypes/CombatState.h"
#include "ShooterCharacter.generated.h"

USTRUCT(BlueprintType)
struct FPhysicAssetElement
{
	GENERATED_BODY()

	UPROPERTY()
	FTransform BoneTransform;

	UPROPERTY()
	FKSphylElem CapsuleInfo;
};

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

	void UpdateHUDHealth();
	
	UPROPERTY()
	TMap<FName, const FPhysicAssetElement> HitCollisionData;

	UPROPERTY()
	TMap<FName, class UBoxComponent*> BoxCollision;
	
	//UPROPERTY()
	//TMap<FName, UCapsuleComponent*> CapsuleCollision;

	//UPROPERTY()
	//TMap<FName, UCapsuleComponent*> CapsuleCollisionCopy;

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

	// Hit capsules for server-side rewind

	UPROPERTY(EditAnywhere)
	UBoxComponent* head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_04;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_r;

	/*
	*UPROPERTY(EditAnywhere)
	UCapsuleComponent* head;
	
	UPROPERTY(EditAnywhere)
	UCapsuleComponent*neck_01;

	UPROPERTY(EditAnywhere)
	UCapsuleComponent* neck_02;
	
	UPROPERTY(EditAnywhere)
	UCapsuleComponent* pelvis;
	
	UPROPERTY(EditAnywhere)
	UPROPERTY(EditAnywhere)
	UCapsuleComponent* spine_03;

	UCapsuleComponent* spine_02;

	UPROPERTY(EditAnywhere)
	UCapsuleComponent* spine_04;

	UPROPERTY(EditAnywhere)
	UCapsuleComponent* spine_05;

	UPROPERTY(EditAnywhere)
	UCapsuleComponent* upperarm_l;

	UPROPERTY(EditAnywhere)
	UCapsuleComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
	UCapsuleComponent* clavicle_l;

	UPROPERTY(EditAnywhere)
	UCapsuleComponent* clavicle_r;

	UPROPERTY(EditAnywhere)
	UCapsuleComponent* lowerarm_l;

	UPROPERTY(EditAnywhere)
	UCapsuleComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
	UCapsuleComponent* hand_l;

	UPROPERTY(EditAnywhere)
	UCapsuleComponent* hand_r;

	UPROPERTY(EditAnywhere)
	UCapsuleComponent* thigh_l;

	UPROPERTY(EditAnywhere)
	UCapsuleComponent* thigh_r;

	UPROPERTY(EditAnywhere)
	UCapsuleComponent* calf_l;

	UPROPERTY(EditAnywhere)
	UCapsuleComponent* calf_r;

	UPROPERTY(EditAnywhere)
	UCapsuleComponent* foot_l;

	UPROPERTY(EditAnywhere)
	UCapsuleComponent* foot_r;
	*/
	
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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UCombatComponent> Combat;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class ULagCompensationComponent> LagCompensation;
	
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

	/**
	 * PhysicsAsset for server-side rewind
	 */
	//void CreateCapsules();
	void StorePhysicsAsset();
	void SetHitCapsuleSize();

	/*
	UPROPERTY(EditAnywhere)
	TObjectPtr<UParticleSystem> CharacterImpactParticles;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundCue> CharacterImpactSounds;

	UPROPERTY()
	TObjectPtr<class AShooterCharacter> ShooterCharacterReceivingHit;

	float Lifetime = 2.f;
	FTimerHandle DestroyProjectileTimer;

	void DestroyProjectile();

	FTimerHandle DestroyWidgetHitTimer;

	void ShowHitWidget();
	*/
	
public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	bool CanJump();
	FORCEINLINE float GetAimingYawRotation() const { return AimingYawRotation; }
	FORCEINLINE float GetAimingPitchRotation() const { return AimingPitchRotation; }
	TObjectPtr<AWeapon> GetEquippedWeapon();
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	FORCEINLINE float GetBaseWalkSpeed() const { return BaseWalkSpeed; }
	FORCEINLINE float GetAimWalkSpeed() const { return AimWalkSpeed; }
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool IsCharacterEliminated() const { return bCharacterEliminated; }
	ECombatState GetCombatState() const;
	bool IsLocallyReloading();
	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensation; }
};
//NetEmulation.PktLag 100