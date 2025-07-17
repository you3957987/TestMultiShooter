#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileBullet.generated.h"


UCLASS()
class MULTISHOOTER_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void OnHit( UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit ) override; 
	//충돌 시 호출되는 함수

public:
	AProjectileBullet();

#if WITH_EDITOR // 에디터에서 프로퍼티가 변경되었을 때 호출되는 함수 == Projectile.h 의 초기 속도(자체설정) 변경시 프로젝타일 무브먼트에 바로 값 적용되도록 하기!!
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override; // 프로퍼티가 변경되었을 때 호출되는 함수
#endif

};
