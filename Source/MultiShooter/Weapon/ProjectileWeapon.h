#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "ProjectileWeapon.generated.h"

UCLASS()
class MULTISHOOTER_API AProjectileWeapon : public ABaseWeapon
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "자체설정")
	TSubclassOf<class AProjectile> ProjectileClass; // 발사체 클래스

	UPROPERTY(EditAnywhere, Category = "자체설정")
	TSubclassOf<class AProjectile> ServerSideRewindProjectileClass; // 서버 사이드 리와인드 발사체 클래스
	// 이거는 bp 에서 똑같은 총알을 복제하니만, 다른 점은 로컬에서만 나갈거니깐 디테일에서 replication 을 체크 해제해야함!!!!!

public:

	// 발사체를 생성하고 발사하는 함수
	virtual void Fire(const FVector& HitTarget) override;
	
};
