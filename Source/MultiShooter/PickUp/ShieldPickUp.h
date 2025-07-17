#pragma once

#include "CoreMinimal.h"
#include "PickUp.h"
#include "ShieldPickUp.generated.h"

UCLASS()
class MULTISHOOTER_API AShieldPickUp : public APickUp
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "자체설정")
	float ShieldAmmount = 100.f; // 회복할 체력 양

	UPROPERTY(EditAnywhere, Category = "자체설정")
	float ShieldTime = 5.f; // 체력 회복 시간
	
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult); // 오버랩 시작 함수

public:
	AShieldPickUp();
};
