#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

UCLASS()
class MULTISHOOTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

protected:
	virtual void OnHit( UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit ) override; // 충돌 시 호출되는 함수, AProjectile의 OnHit를 오버라이드하여 구현합니다.
	virtual void BeginPlay() override; // 게임 시작 시 호출되는 함수, AProjectile의 BeginPlay를 오버라이드하여 구현합니다.
	virtual void Destroyed() override; // 발사체가 파괴될 때 호출되는 함수, AProjectile의 Destroyed를 오버라이드하여 구현합니다.

	UPROPERTY( EditAnywhere )
	USoundCue* ProjectileLoop;

	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent; // 발사체 루프 사운드 컴포넌트

	UPROPERTY( EditAnywhere )
	USoundAttenuation* SoundAttenuation; // 사운드 감쇠 설정

	UPROPERTY( VisibleAnywhere )
	class URocketMovementComponent* RocketMovementComponent; // 로켓 이동 컴포넌트

public:
	AProjectileRocket();

	
};
