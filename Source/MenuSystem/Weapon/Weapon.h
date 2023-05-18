// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState:uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayNmae = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class MENUSYSTEM_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
	void ShowPickupWidget(bool bShowWidget);
	virtual void FireWeapon(const FVector& HitTarget);

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

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;
	
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	TObjectPtr<class USphereComponent> AreaSphere;

	// We can replicate the variable since I set the bReplicates = true in the Weapon.cpp
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	TObjectPtr<class UWidgetComponent> PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	TObjectPtr<UAnimationAsset> FireAnimation;

public:
	void SetWeaponState(EWeaponState State);
	FORCEINLINE TObjectPtr<USphereComponent> GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE TObjectPtr<USkeletalMeshComponent> GetWeaponMesh() const { return WeaponMesh;  }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE FVector3d GetZoomedCameraLocation() const { return ZoomedCameraLocation; }
	FORCEINLINE FVector3d GetZoomedCrouchCameraLocation() const { return ZoomedCrouchCameraLocation; }
};
