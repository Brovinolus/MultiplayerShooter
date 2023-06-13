// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class MENUSYSTEM_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

	// Server-side rewind
	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	// 2 decimal place of precision
	FVector_NetQuantize100 InitialVelocity;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000;
	
	float Damage = 20.f;
	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(
		UPrimitiveComponent* HitComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit);

	UPROPERTY(EditAnywhere, Category = "Widgets")
	TSubclassOf<UUserWidget> WidgetHitClass;

	UPROPERTY()
	TObjectPtr<UUserWidget> WidgetHitInstance;
	
private:
	UPROPERTY(EditAnywhere)
	TObjectPtr<class UBoxComponent> CollisionBox;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UParticleSystem> Tracer;
	
	TObjectPtr<UParticleSystemComponent> TracerComponent;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UParticleSystem> StoneImpactParticles;

	UPROPERTY(EditAnywhere)
	TObjectPtr<class USoundCue> StoneImpactSounds;

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

	UFUNCTION(Client, Reliable)
	void Client_ShowHitWidget();
};
