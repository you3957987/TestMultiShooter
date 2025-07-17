#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundCue.h"
#include "Projectile.generated.h"

UCLASS()
class MULTISHOOTER_API AProjectile : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UParticleSystem* TraceParticle; // 충돌 시 생성될 파티클 시스템

	UPROPERTY()
	class UParticleSystemComponent* TraceParticleComponent; // 파티클 시스템 컴포넌트

	FTimerHandle DestroyTimer; // 파괴 타이머 핸들

	UPROPERTY( EditAnywhere, Category = "자체설정")
	float DestroyTime = 3.f; // 파괴 시간. 이거 로켓 삭제시 파티클 바로 안지워 지게 설정하는 거임. 로켓이 터지면 파티클이 사라지기 전에 파괴되게 하는거임 중요 X

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit( UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit ); 
	// 충돌 시 호출되는 함수

	UPROPERTY( VisibleAnywhere )
	UStaticMeshComponent* ProjectileMesh; //메쉬 컴포넌트

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UBoxComponent* CollisionBox; // 충돌 박스 컴포넌트

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UParticleSystem* ImpactParticle; // 충돌 시 생성될 파티클 시스템

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class USoundCue* ImpactSound; // 충돌 시 재생될 사운드 큐

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UProjectileMovementComponent* ProjectileMovementComponent; // 발사체 이동 컴포넌트

	UPROPERTY( EditAnywhere, Category = "자체설정")
	class UNiagaraSystem* TrailSystem;

	UPROPERTY()
	class UNiagaraComponent* TrailComponent; // 나이아가라 트레일 컴포넌트

	UPROPERTY(EditAnywhere, Category = "자체설정")
	float DamageInnerRadius = 200.f; // 폭발 피해 반경

	UPROPERTY(EditAnywhere, Category = "자체설정")
	float DamageOuterRadius = 400.f; // 폭발 피해 외부 반경

	void SpawnTrailSystem(); // 나이아가라 트레일 시스템을 생성하는 함수

	void StartDestroyTimer(); // 파괴 타이머를 시작하는 함수
	void DestroyTimerFinished(); // 파괴 타이머가 끝났을 때 호출되는 함수

	void ExplodeDamage(); // 폭발 피해를 입히는 함수

public:
	AProjectile();	
	virtual void Tick(float DeltaTime) override;
	virtual void Destroyed() override;

	FORCEINLINE UBoxComponent* GetCollisionBox() const { return CollisionBox; }

	// 발사체에서의 대미지는 로켓이랑 유탄발사기만 지정하는 거임. 돌격 소총은 총 자체에 대미지 있고 발사시 지정해줌!!!! 사실 헤드샷 대미지는 없어도 되긴 함;;
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float Damage = 0.f; // 발사체가 입힐 피해량
	// 발사체에서의 대미지는 로켓이랑 유탄발사기만 지정하는 거임. 돌격 소총은 총 자체에 대미지 있고 발사시 지정해줌!!!! 사실 헤드샷 대미지는 없어도 되긴 함;;
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float HeadShotDamage = 0.f; // 헤드샷 피해량

	// 서버 사이드 리와인드에서 사용
	UPROPERTY(EditAnywhere, Category = "자체설정")
	bool bUseServerSideRewind = false; // 서버 사이드 리와인드를 사용할지 여부
	
	FVector_NetQuantize TraceStart; // 트레이스 시작 위치
	FVector_NetQuantize100 InitialVelocity; // 초기 속도

	UPROPERTY(EditAnywhere, Category = "자체설정")
	float InitialSpeed = 15000.f; // 초기 속도 값 == 이거 바꾸면 자동으로 발사체 무브먼트 속도 바뀜(프로젝타일 불렛 헤더에서 설정함)

};
