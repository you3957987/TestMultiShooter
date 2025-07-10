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
	virtual void Fire(const FVector& HitTarget) override; // 발사 함수, 타겟 위치를 인자로 받음
	
};
