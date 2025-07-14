#pragma once

#include "CoreMinimal.h"
#include "PickUp.h"
#include "SpeedPickUp.generated.h"

UCLASS()
class MULTISHOOTER_API ASpeedPickUp : public APickUp
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float BaseSpeedBuff = 1600.f;

	UPROPERTY(EditAnywhere)
	float SpeedBuffTime = 10.f; // 속도 버프 지속 시간
	
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult); // 오버랩 시작 함수

public:
	ASpeedPickUp();
};
