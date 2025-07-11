#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"
#include "Projectile.generated.h"

UCLASS()
class MULTISHOOTER_API AProjectile : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	class UParticleSystem* TraceParticle; // 충돌 시 생성될 파티클 시스템

	UPROPERTY()
	class UParticleSystemComponent* TraceParticleComponent; // 파티클 시스템 컴포넌트

	FTimerHandle DestroyTimer; // 파괴 타이머 핸들

	UPROPERTY( EditAnywhere )
	float DestroyTime = 3.f; // 파괴 시간, 발사된 후 5초 후에 파괴됩니다.

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit( UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit ); 
	// 충돌 시 호출되는 함수

	UPROPERTY( VisibleAnywhere )
	UStaticMeshComponent* ProjectileMesh; //메쉬 컴포넌트

	UPROPERTY(EditAnywhere)
	float Damage = 20.f; // 발사체가 입힐 피해량

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox; // 충돌 박스 컴포넌트

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticle; // 충돌 시 생성될 파티클 시스템

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound; // 충돌 시 재생될 사운드 큐

	UPROPERTY(EditAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent; // 발사체 이동 컴포넌트

	UPROPERTY( EditAnywhere )
	class UNiagaraSystem* TrailSystem;

	UPROPERTY()
	class UNiagaraComponent* TrailComponent; // 나이아가라 트레일 컴포넌트

	void SpawnTrailSystem(); // 나이아가라 트레일 시스템을 생성하는 함수

	void StartDestroyTimer(); // 파괴 타이머를 시작하는 함수
	void DestroyTimerFinished(); // 파괴 타이머가 끝났을 때 호출되는 함수

	void ExplodeDamage(); // 폭발 피해를 입히는 함수

	UPROPERTY( EditAnywhere )
	float DamageInnerRadius = 200.f; // 폭발 피해 반경
	UPROPERTY( EditAnywhere )
	float DamageOuterRadius = 400.f; // 폭발 피해 외부 반경

public:
	AProjectile();	
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

	FORCEINLINE UBoxComponent* GetCollisionBox() const { return CollisionBox; }

};
