// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"

#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MenuSystem/Character/ShooterCharacter.h"
#include "Sound/SoundCue.h"
#include "MenuSystem/MenuSystem.h"
#include "Net/UnrealNetwork.h"

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility,ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic,ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECR_Block);

	//ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	//ProjectileMovementComponent->bRotationFollowsVelocity = true;
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionBox->IgnoreActorWhenMoving(GetOwner(), true);
	
	if (Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
			);
	}
	
	if (WidgetHitClass)
	{
		WidgetHitInstance = CreateWidget<UUserWidget>(GetWorld(), WidgetHitClass);

		if (WidgetHitInstance)
		{
			WidgetHitInstance->AddToViewport();
			WidgetHitInstance->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	
	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
		GetWorldTimerManager().SetTimer(DestroyProjectileTimer, this, &AProjectile::DestroyProjectile, Lifetime, false);
	}
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	ShooterCharacterReceivingHit = Cast<AShooterCharacter>(OtherActor);
	if (ShooterCharacterReceivingHit)
	{
		Client_ShowHitWidget();
	}
	
	Destroy();
}

void AProjectile::DestroyProjectile()
{
	Destroy();
}

void AProjectile::ShowHitWidget()
{
	if (WidgetHitInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Widget displayed"));

		const float DisplayDuration = 0.5f;
			
		WidgetHitInstance->SetVisibility(ESlateVisibility::Visible);
		GetWorldTimerManager().SetTimer(DestroyWidgetHitTimer, [this]() {
			if (WidgetHitInstance)
			{
				WidgetHitInstance->SetVisibility(ESlateVisibility::Hidden);
			}
		}, DisplayDuration, false);
	}
}

void AProjectile::Client_ShowHitWidget_Implementation()
{
	ShowHitWidget();
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// It doesn't work on slow machines. Solution: replicating or ssr
void AProjectile::Destroyed()
{
	Super::Destroyed();
	
	if (StoneImpactParticles && ShooterCharacterReceivingHit == nullptr)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), StoneImpactParticles, GetActorTransform());
	}
	else
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), CharacterImpactParticles, GetActorTransform());
	}

	if (StoneImpactSounds && ShooterCharacterReceivingHit == nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(this, StoneImpactSounds, GetActorLocation());
	}
	else
	{
		UGameplayStatics::PlaySoundAtLocation(this, CharacterImpactSounds, GetActorLocation());
	}
}

