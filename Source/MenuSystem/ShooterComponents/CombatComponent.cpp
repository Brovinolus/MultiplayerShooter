// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MenuSystem/Weapon/Weapon.h"
#include "MenuSystem/Character/ShootingCharacter.h"
#include "Net/UnrealNetwork.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;
	//BaseWalkSpeed = 300.f;
	//AimWalkSpeed = 300.f;
}


void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

/*
void UCombatComponent::SetWalking(bool IsWalking)
{
	bWalking = IsWalking;
	
	if (IsWalking)
	{
		if (Character)
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		}
	}
	else
	{
		if (Character)
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = BaseJogSpeed;
		}
	}
}


void UCombatComponent::ServerSetWalking_Implementation(bool IsWalking)
{
	bWalking = IsWalking;
	
	if (IsWalking)
	{
		if (Character)
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		}
	}
	else
	{
		if (Character)
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = BaseJogSpeed;
		}
	}
}*/

void UCombatComponent::SetAimingYawRotation(float AimingYaw)
{
	AimingYawRotation = AimingYaw;
	
	ServerSetAimingYawRotation(AimingYaw);
}

void UCombatComponent::SetAimingPitchRotation(float AimingPitch)
{
	AimingPitchRotation = AimingPitch;
	ServerSetAimingPitchRotation(AimingPitch);
}

void UCombatComponent::ServerSetAimingPitchRotation_Implementation(float AimingPitch)
{
	AimingPitchRotation = AimingPitch;
}

void UCombatComponent::ServerSetAimingYawRotation_Implementation(float AimingYaw)
{
	AimingYawRotation = AimingYaw;
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, AimingYawRotation);
	DOREPLIFETIME(UCombatComponent, AimingPitchRotation);
	//DOREPLIFETIME(UCombatComponent, bWalking);
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	EquippedWeapon = WeaponToEquip;

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}

	EquippedWeapon->SetOwner(Character);
}

