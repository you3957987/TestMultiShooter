#pragma once

#include "CoreMinimal.h"
#include "PickUp.h"
#include "GrenadePickUp.generated.h"

UCLASS()
class MULTISHOOTER_API AGrenadePickUp : public APickUp
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "자체설정")
	int32 GrenadeAmount = 1; // 획득할 수류탄 개수

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult); // 오버랩 시작 함수
	
};
