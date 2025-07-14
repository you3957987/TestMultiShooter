#pragma once

#include "CoreMinimal.h"
#include "PickUp.h"
#include "InvisiblePickUp.generated.h"

UCLASS()
class MULTISHOOTER_API AInvisiblePickUp : public APickUp
{
	GENERATED_BODY()

	int32 InvisibleTime = 5; // 투명 시간 설정


protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult); // 오버랩 시작 함수
	
public:
	AInvisiblePickUp();
};
