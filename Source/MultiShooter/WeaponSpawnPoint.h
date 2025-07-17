#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponSpawnPoint.generated.h"

UCLASS()
class MULTISHOOTER_API AWeaponSpawnPoint : public AActor
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

    // 여러 무기 클래스 배열
    UPROPERTY(EditAnywhere, Category = "자체설정")
    TArray<TSubclassOf<class ABaseWeapon>> WeaponClassesToSpawn;

    UPROPERTY(EditAnywhere, Category = "자체설정")
    float RespawnTime = 10.f;

    UPROPERTY()
    ABaseWeapon* SpawnedWeapon;

    FTimerHandle RespawnTimerHandle;

    void SpawnWeapon();

    UFUNCTION()
    void OnWeaponEquipped();

public:
    AWeaponSpawnPoint();
};