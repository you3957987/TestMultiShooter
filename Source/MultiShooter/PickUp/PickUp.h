#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickUp.generated.h"

UCLASS()
class MULTISHOOTER_API APickUp : public AActor
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	class USphereComponent* OverlapShere;

	UPROPERTY(EditAnywhere)
	class USoundCue* PickUpSound; // 아이템 획득 사운드
	
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent * PickUpMesh; // 아이템 획득 메쉬

	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* PickUpEffectComponent; // 체력 회복 이펙트 컴포넌트

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickUpEffect; // 체력 회복 이펙트 시스템

	FTimerHandle BindOverlapTimer; // 오버랩 타이머 핸들
	float BindOverlapTime = 0.25f; // 오버랩 타이머 시간

	void FinishBindOverlapTimer(); // 오버랩 타이머가 끝났을 때 호출되는 함수
	
protected:
	virtual void BeginPlay() override;
	virtual void Destroyed() override;
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult); // 오버랩 시작 함수
	
	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;

public:	
	APickUp();
	virtual void Tick(float DeltaTime) override;

};
