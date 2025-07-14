#pragma once

#include "CoreMinimal.h"
#include "PickUp.h"
#include "NiagaraComponent.h"          
#include "NiagaraSystem.h"            
#include "HealthPickUp.generated.h"

UCLASS()
class MULTISHOOTER_API AHealthPickUp : public APickUp
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	float HealthAmount = 100.f; // 회복할 체력 양

	UPROPERTY(EditAnywhere)
	float HealingTime = 5.f; // 체력 회복 시간

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult); // 오버랩 시작 함수

public:
	AHealthPickUp();
	
};
