// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"

#include "Projectile.h"
#include "Engine/SkeletalMeshSocket.h"

void AProjectileWeapon::FireWeapon(const FVector& HitTarget)
{
	Super::FireWeapon(HitTarget);

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleSocket = GetWeaponMesh()->GetSocketByName("Muzzle");
	UWorld* World = GetWorld();

	if (!MuzzleSocket || !World) return;

	FTransform SocketTransform = MuzzleSocket->GetSocketTransform(GetWeaponMesh());
	FVector ToTarget = HitTarget - SocketTransform.GetLocation();
	FRotator TargetRotation = ToTarget.Rotation();

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.Instigator = InstigatorPawn;

	AProjectile* SpawnedProjectile = nullptr;
	
	bool bServer = InstigatorPawn && InstigatorPawn->HasAuthority();
	bool bLocallyControlled = InstigatorPawn && InstigatorPawn->IsLocallyControlled();
	
	if (bUseServerSideRewind)
	{
		if (bServer) // server
		{
			if (bLocallyControlled) // server, host - use replicated projectile, no SSR
			{
				UE_LOG(LogTemp, Warning, TEXT("server, host: spawn ProjectileClass"));
				
				SpawnedProjectile = World->SpawnActor<AProjectile>(
					ProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParameters
				);
				SpawnedProjectile->bUseServerSideRewind = false;
				SpawnedProjectile->Damage = Damage;
			}
			else // server, not locally controlled - spawn non-replicated projectile, use SSR
			{
				UE_LOG(LogTemp, Warning, TEXT("server, not locally controlled: spawn ServerSideRewindProjectileClass "));
				
				SpawnedProjectile = World->SpawnActor<AProjectile>(
					ServerSideRewindProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParameters
				);
				SpawnedProjectile->bUseServerSideRewind = true;
			}
		}
		else // client, using SSR
		{
			// client, locally controlled - spawn non-replicated projectile, use SSR
			if (bLocallyControlled)
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(
					ServerSideRewindProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParameters
				);
				SpawnedProjectile->bUseServerSideRewind = true;
				SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
				SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->
					InitialSpeed;
				SpawnedProjectile->Damage = Damage;

				UE_LOG(LogTemp, Warning, TEXT("client, using SSR"));
			}
			else // client, not locally controlled - spawn non-replicated projectile, no SSR
			{
				SpawnedProjectile = World->SpawnActor<AProjectile>(
					ServerSideRewindProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParameters
				);
				SpawnedProjectile->bUseServerSideRewind = false;
			}
		}
	}
	else // weapon not using SSR
	{
		if (bServer)
		{
			SpawnedProjectile = World->SpawnActor<AProjectile>(
				ProjectileClass,
				SocketTransform.GetLocation(),
				TargetRotation,
				SpawnParameters
			);
			SpawnedProjectile->bUseServerSideRewind = false;
			SpawnedProjectile->Damage = Damage;
		}
	}
}

