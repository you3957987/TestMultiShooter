#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponSpawnPoint.generated.h"

class ABaseWeapon;

UCLASS()
class MULTISHOOTER_API AWeaponSpawnPoint : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Spawning")
	TArray<TSubclassOf<ABaseWeapon>> WeaponClassesToSpawn;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_SpawnedWeapon)
	ABaseWeapon* SpawnedWeapon;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	float RespawnTime = 5.0f;

	FTimerHandle RespawnTimerHandle;

protected:
	virtual void BeginPlay() override;

	void SpawnWeapon();

	UFUNCTION()
	void OnWeaponEquipped();

	UFUNCTION()
	void OnRep_SpawnedWeapon();

public:
	AWeaponSpawnPoint();
};