#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "ProjectileWeapon.generated.h"

UCLASS()
class MULTISHOOTER_API AProjectileWeapon : public ABaseWeapon
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass; // 발사체 클래스

public:

	// 발사체를 생성하고 발사하는 함수
	virtual void Fire(const FVector& HitTarget) override;
	
};
