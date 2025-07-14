#include "WeaponPickUp.h"
#include "MultiShooter/Weapon/BaseWeapon.h"
#include "Engine/World.h"

AWeaponPickUp::AWeaponPickUp()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AWeaponPickUp::BeginPlay()
{
	Super::BeginPlay();

}

