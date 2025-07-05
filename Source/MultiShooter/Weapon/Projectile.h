#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class MULTISHOOTER_API AProjectile : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox; // 충돌 박스 컴포넌트

	UPROPERTY(EditAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent; // 발사체 이동 컴포넌트

	UPROPERTY(EditAnywhere)
	UParticleSystem* TraceParticle; // 충돌 시 생성될 파티클 시스템

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticle; // 충돌 시 생성될 파티클 시스템

	UPROPERTY(EditAnywhere)
	USoundCue* ImpactSound; // 충돌 시 재생될 사운드 큐

	class UParticleSystemComponent* TraceParticleComponent; // 파티클 시스템 컴포넌트

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit( UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit ); // 충돌 시 호출되는 함수

	UPROPERTY(EditAnywhere)
	float Damage = 20.f; // 발사체가 입힐 피해량

public:
	AProjectile();	
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

};
