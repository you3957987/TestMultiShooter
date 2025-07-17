#pragma once

#include "CoreMinimal.h"
#include "PickUp.h"
#include "JumpPickUp.generated.h"

UCLASS()
class MULTISHOOTER_API AJumpPickUp : public APickUp
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float JumpBuff = 1500.f;

	UPROPERTY(EditAnywhere, Category = "자체설정")
	float JumpBuffTime = 10.f; // 점프 버프 지속 시간

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult); // 오버랩 시작 함수
	
};
