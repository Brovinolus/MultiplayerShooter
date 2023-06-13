// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState:uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayNmae = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

/*
UENUM(BlueprintType)
enum class EWeaponType:uint8
{
	EWT_Rifle UMETA(DisplayName = "Rifle"),
	EWT_Pistol UMETA(DisplayName = "Pistol")
};*/

UCLASS()
class MENUSYSTEM_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;
	void SetHUDAmmo();
	void AddDisableSSR_OnHighPing();
	void RemoveDisableSSR_OnHighPing();
	void ShowPickupWidget(bool bShowWidget);
	virtual void FireWeapon(const FVector& HitTarget);
	void WeaponDropped();
	void AddAmmo(int32 AmmoToAdd);

	/*
	 * Textures for the weapon crosshairs
	 */
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	TObjectPtr<UTexture2D> CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	TObjectPtr<UTexture2D> CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	TObjectPtr<UTexture2D> CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	TObjectPtr<UTexture2D> CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	TObjectPtr<UTexture2D> CrosshairsBottom;

	/**
	 * Zoomed FOV while aiming
	 */
	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	FVector ZoomedCameraLocation = FVector(8.5f, 0.f, 60.f);

	UPROPERTY(EditAnywhere)
	FVector ZoomedCrouchCameraLocation = FVector(0.f, 0.f, 20.f);

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	/**
	 * Automatic fire
	 */

	UPROPERTY(EditAnywhere, Category = "Combat")
	float FireDelay = 0.15f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bAutomatic = true;

	bool bDestroyWeapon = false;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class USoundCue> EquipSound;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundCue> DropSound;

protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SeepResult
		);
	
	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
		);

	UPROPERTY(Replicated, EditAnywhere)
	bool bUseServerSideRewind = false;

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UFUNCTION()
	void OnHighPing(bool bPingHigh);

	void OnWeaponStateSet();
private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;
	
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	TObjectPtr<class USphereComponent> AreaSphere;

	// We can replicate the variable since I set the bReplicates = true in the Weapon.cpp
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	EWeaponType WeaponType;
	
	UFUNCTION()
	void OnRep_WeaponState();

	bool HasSetController = false;

	void SetController();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	TObjectPtr<class UWidgetComponent> PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	TObjectPtr<UAnimationAsset> FireAnimation;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	TObjectPtr<UAnimationAsset> FireAnimationWithoutParticles;

	UPROPERTY(EditAnywhere)
	int32 Ammo;

	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

	// The number of unprocessed server requests for Ammo
	// Incremented in SpendRound, decremented in ClientUpdateAmmo
	int32 Sequence = 0;

	UPROPERTY(EditAnywhere)
	int32 MaxAmmo;

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

	int32 SetAmmo(int32 NewAmmoValue);
	
	void SpendRound();

	bool bCanShowParticlesInFireAnimation = true;

	UPROPERTY()
	TObjectPtr<class AShooterCharacter> ShooterOwnerCharacter;
	
	UPROPERTY()
	TObjectPtr<class AShooterPlayerController> ShooterOwnerController;
public:
	void SetWeaponState(EWeaponState State);
	void SetCanShowParticlesInFireAnimation(bool bCanShowParticles);
	FORCEINLINE TObjectPtr<USphereComponent> GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE TObjectPtr<USkeletalMeshComponent> GetWeaponMesh() const { return WeaponMesh;  }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE FVector3d GetZoomedCameraLocation() const { return ZoomedCameraLocation; }
	FORCEINLINE FVector3d GetZoomedCrouchCameraLocation() const { return ZoomedCrouchCameraLocation; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	bool IsAmmoEmpty();
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE float GetDamage() const { return Damage; }
};
