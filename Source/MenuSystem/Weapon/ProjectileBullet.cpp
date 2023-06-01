// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "MenuSystem/HUD/ShooterHUD.h"
#include "MenuSystem/PlayerController/ShooterPlayerController.h"

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
                              FVector NormalImpulse, const FHitResult& Hit)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	
	if (OwnerCharacter)
	{
		AController* OwnerController = OwnerCharacter->Controller;

		if (OwnerController)
		{
			UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());

			Controller = Controller == nullptr ? Cast<AShooterPlayerController>(OwnerCharacter->Controller) : Controller;
			HUD = HUD == nullptr ? Cast<AShooterHUD>(Controller->GetHUD()) : HUD;
			if(HUD)
			{
				if (OtherActor->CanBeDamaged())
				{
					UE_LOG(LogTemp, Warning, TEXT("Hit"));
				}
			}
		}
	}
	
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);

}
