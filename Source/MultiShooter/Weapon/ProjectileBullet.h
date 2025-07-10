#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileBullet.generated.h"


UCLASS()
class MULTISHOOTER_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()

protected:
	virtual void OnHit( UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit ) override; 
	//충돌 시 호출되는 함수

public:

	AProjectileBullet();


};
