#include "WeaponSpawnPoint.h"
#include "MultiShooter/Weapon/BaseWeapon.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"

AWeaponSpawnPoint::AWeaponSpawnPoint()
{
    PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AWeaponSpawnPoint::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeaponSpawnPoint, SpawnedWeapon);
}

void AWeaponSpawnPoint::BeginPlay()
{
    Super::BeginPlay();
	if (HasAuthority())
	{
		SpawnWeapon();
	}
}

void AWeaponSpawnPoint::SpawnWeapon()
{
    if (!HasAuthority()) return;
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

    OnRep_SpawnedWeapon(); // 서버에서도 델리게이트 바인딩을 위해 호출
}

void AWeaponSpawnPoint::OnWeaponEquipped()
{
    if (HasAuthority())
    {
        GetWorld()->GetTimerManager().SetTimer(
            RespawnTimerHandle,
            this,
            &AWeaponSpawnPoint::SpawnWeapon,
            RespawnTime,
            false
        );
    }
}

void AWeaponSpawnPoint::OnRep_SpawnedWeapon()
{
    if (SpawnedWeapon)
    {
        SpawnedWeapon->OnEquippedDelegate.AddDynamic(this, &AWeaponSpawnPoint::OnWeaponEquipped);
    }
}