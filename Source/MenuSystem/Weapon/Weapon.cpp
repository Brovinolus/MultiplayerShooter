// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "../Character/ShooterCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "MenuSystem/PlayerController/ShooterPlayerController.h"
#include "Net/UnrealNetwork.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AreaSphere=CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}

	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::AWeapon::OnSphereEndOverlap);
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetController();
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SeepResult)
{
	AShooterCharacter* ShootingCharacter = Cast<AShooterCharacter>(OtherActor);

	if (ShootingCharacter)
	{
		ShootingCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AShooterCharacter* ShootingCharacter = Cast<AShooterCharacter>(OtherActor);

	if (ShootingCharacter)
	{
		ShootingCharacter->SetOverlappingWeapon(nullptr);
	}
}

int32 AWeapon::SetAmmo(int32 NewAmmoValue)
{
	Ammo = FMath::Clamp(NewAmmoValue, 0, MagCapacity);
	SetHUDAmmo();
	return Ammo;
}

// need to apply server reconciliation
void AWeapon::SpendRound()
{
	Ammo = SetAmmo(Ammo - 1);
	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else if (ShooterOwnerCharacter && ShooterOwnerCharacter->IsLocallyControlled())
	{
		++Sequence;
	}
}

void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return;

	Ammo = ServerAmmo;
	--Sequence;
	Ammo -= Sequence;
	SetHUDAmmo();
}

void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	Ammo = SetAmmo(Ammo + AmmoToAdd);
	ClientAddAmmo(AmmoToAdd);
}

void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) return;
	
	Ammo = SetAmmo(Ammo + AmmoToAdd);
}

// since we are going to client side predicting, we are not going to replicate but use RPCs
//void AWeapon::OnRep_Ammo()

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (Owner == nullptr)
	{
		ShooterOwnerCharacter = nullptr;
		ShooterOwnerController = nullptr;
	}
	else
	{
		SetHUDAmmo();
	}
}

void AWeapon::SetHUDAmmo()
{
	ShooterOwnerCharacter = ShooterOwnerCharacter == nullptr
								? Cast<AShooterCharacter>(GetOwner())
								: ShooterOwnerCharacter;
	if (ShooterOwnerCharacter)
	{
		ShooterOwnerController = ShooterOwnerController == nullptr
									 ? Cast<AShooterPlayerController>(ShooterOwnerCharacter->Controller)
									 : ShooterOwnerController;

		if(IsValid(ShooterOwnerController))
		{
			ShooterOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;
	OnWeaponStateSet();
}

void AWeapon::OnRep_WeaponState()
{
	OnWeaponStateSet();
}

void AWeapon::OnWeaponStateSet()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AddDisableSSR_OnHighPing();
		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		RemoveDisableSSR_OnHighPing();
		break;
	}
}

void AWeapon::SetController()
{
	if (!HasSetController && HasAuthority() && ShooterOwnerCharacter && ShooterOwnerCharacter->Controller)
	{
		ShooterOwnerController = ShooterOwnerController == nullptr ? Cast<AShooterPlayerController>(ShooterOwnerCharacter->Controller) : ShooterOwnerController;
		if (ShooterOwnerController && !ShooterOwnerController->HighPingDelegate.IsBound())
		{
			ShooterOwnerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnHighPing);
			HasSetController = true;
		}
	}
}

void AWeapon::AddDisableSSR_OnHighPing()
{
	ShooterOwnerCharacter = ShooterOwnerCharacter == nullptr
								? Cast<AShooterCharacter>(GetOwner())
								: ShooterOwnerCharacter;
	
	if (ShooterOwnerCharacter && bUseServerSideRewind)
	{
		ShooterOwnerController = ShooterOwnerController == nullptr
									 ? Cast<AShooterPlayerController>(ShooterOwnerCharacter->Controller)
									 : ShooterOwnerController;

		if(IsValid(ShooterOwnerController) && HasAuthority())
		{
			if (!ShooterOwnerController->HighPingDelegate.IsBound())
			{
				UE_LOG(LogTemp, Warning, TEXT("Add DisableSSR"));
				ShooterOwnerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnHighPing);
			}
		}
	}
}

void AWeapon::RemoveDisableSSR_OnHighPing()
{
	ShooterOwnerCharacter = ShooterOwnerCharacter == nullptr
								? Cast<AShooterCharacter>(GetOwner())
								: ShooterOwnerCharacter;
	if (ShooterOwnerCharacter && bUseServerSideRewind)
	{
		ShooterOwnerController = ShooterOwnerController == nullptr
									 ? Cast<AShooterPlayerController>(ShooterOwnerCharacter->Controller)
									 : ShooterOwnerController;

		if(IsValid(ShooterOwnerController) && HasAuthority())
		{
			if (ShooterOwnerController->HighPingDelegate.IsBound())
			{
				UE_LOG(LogTemp, Warning, TEXT("Remove DisableSSR"));
				ShooterOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnHighPing);
			}
		}
	}
}

void AWeapon::OnHighPing(bool bPingHigh)
{
	bUseServerSideRewind = !bPingHigh;
	UE_LOG(LogTemp, Warning, TEXT("PingHigh: %s"), bUseServerSideRewind ? TEXT("true") : TEXT("false"));
}

void AWeapon::SetCanShowParticlesInFireAnimation(bool bCanShowParticles)
{
	bCanShowParticlesInFireAnimation = bCanShowParticles;
}

bool AWeapon::IsAmmoEmpty()
{
	return Ammo <= 0;
}

void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::FireWeapon(const FVector& HitTarget)
{
	if (FireAnimationWithoutParticles)
	{
		if (bCanShowParticlesInFireAnimation)
		{
			WeaponMesh->PlayAnimation(FireAnimation, false);
		}
		else
		{
			WeaponMesh->PlayAnimation(FireAnimationWithoutParticles, false);
		}
	}
	
	SpendRound();
}

void AWeapon::WeaponDropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
	ShooterOwnerCharacter = nullptr;
	ShooterOwnerController = nullptr;
}

