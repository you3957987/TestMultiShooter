#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "HitScanWeapon.generated.h"

UCLASS()
class MULTISHOOTER_API AHitScanWeapon : public ABaseWeapon
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticle; // 총구 플래시 이펙트

	UPROPERTY(EditAnywhere)
	float DinstanceToSphere = 800.f; // 타겟과의 거리

	UPROPERTY(EditAnywhere)
	float SphereRadius = 75.f; // 타겟과의 거리

	UPROPERTY(EditAnywhere)
	bool bUseScatter = false; // 산탄 사용 여부

protected:
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget); // 산란을 고려한 트레이스 끝점 계산 함수

	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit ); // 무기 트레이스 히트 함수

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;
	
	UPROPERTY(EditAnywhere)
	USoundCue* HitSound; // 충돌 사운드

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticle; // 충돌 이펙트 파티클

public:
	virtual void Fire(const FVector& HitTarget) override; // 발사 함수, 타겟 위치를 인자로 받음
};
