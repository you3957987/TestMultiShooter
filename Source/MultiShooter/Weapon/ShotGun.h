#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "ShotGun.generated.h"

UCLASS()
class MULTISHOOTER_API AShotGun : public AHitScanWeapon
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	uint32 NumberOfPellets = 10; // 샷건의 총알 개수

public:
	virtual void FireShotgun(const TArray<FVector_NetQuantize>& HitTargets);

	void ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets);

	
};
