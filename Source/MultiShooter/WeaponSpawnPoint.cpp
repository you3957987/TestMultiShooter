#include "WeaponSpawnPoint.h"
#include "MultiShooter/Weapon/BaseWeapon.h"
#include "Engine/World.h"
#include "TimerManager.h"

AWeaponSpawnPoint::AWeaponSpawnPoint()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AWeaponSpawnPoint::BeginPlay()
{
    Super::BeginPlay();
    SpawnWeapon();
}

void AWeaponSpawnPoint::SpawnWeapon()
{
    if (WeaponClassesToSpawn.Num() == 0) return;

    int32 Index = FMath::RandRange(0, WeaponClassesToSpawn.Num() - 1);
    TSubclassOf<ABaseWeapon> WeaponClass = WeaponClassesToSpawn[Index];
    if (!WeaponClass) return;

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    FVector SpawnLocation = GetActorLocation() + FVector(0, 0, 100);

    SpawnedWeapon = GetWorld()->SpawnActor<ABaseWeapon>(
        WeaponClass,
        SpawnLocation,
        GetActorRotation(),
        SpawnParams
    );

    if (SpawnedWeapon)
    {
        SpawnedWeapon->OnEquippedDelegate.AddDynamic(this, &AWeaponSpawnPoint::OnWeaponEquipped);
    }
}

void AWeaponSpawnPoint::OnWeaponEquipped()
{
    GetWorld()->GetTimerManager().SetTimer(
        RespawnTimerHandle,
        this,
        &AWeaponSpawnPoint::SpawnWeapon,
        RespawnTime,
        false
    );
}