#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileGrenade.generated.h"

UCLASS()
class MULTISHOOTER_API AProjectileGrenade : public AProjectile
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "자체설정")
	USoundCue* BounceSound; // 충돌 시 재생될 소리

protected:
	int32 BounceCount = 0; // 튕긴 횟수. 3번만 사운드 재생
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBounce(const FHitResult& Hit, const FVector& ImpactVelocity); // 충돌 시 호출되는 함수
	
public:
	AProjectileGrenade();

	virtual void Destroyed() override;
};
