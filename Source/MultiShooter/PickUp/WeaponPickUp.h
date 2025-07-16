#pragma once

#include "CoreMinimal.h"
#include "PickUp.h"
#include "WeaponPickUp.generated.h"

UCLASS()
class MULTISHOOTER_API AWeaponPickUp : public APickUp
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	TSubclassOf<class ABaseWeapon> WeaponClassToSpawn;

public:
	AWeaponPickUp();
};
