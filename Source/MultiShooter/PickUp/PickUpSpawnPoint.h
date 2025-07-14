#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickUpSpawnPoint.generated.h"

UCLASS()
class MULTISHOOTER_API APickUpSpawnPoint : public AActor
{
	GENERATED_BODY()

	FTimerHandle SpawnPickUpTimer; // 픽업 아이템 생성 타이머 핸들

	UPROPERTY(EditAnywhere)
	float SpawnPickUpTimeMin = 5.f; // 픽업 아이템 생성 주기 최소값

	UPROPERTY(EditAnywhere)
	float SpawnPickUpTimeMax = 10.f; // 픽업 아이템 생성 주기 최대값
	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TArray< TSubclassOf<class APickUp> > PickUpClasses; // 픽업 아이템 클래스 배열

	class APickUp* SpawnedPickUp; // 생성된 픽업 아이템 포인터

	void SpawnPickUp(); // 픽업 아이템을 생성하는 함수

	UFUNCTION()
	void StartSpawnPickUpTimer( AActor* DestroyedActor ); // 픽업 아이템 생성 타이머 시작
	void FinishedSpawnPickUpTimer(); // 픽업 아이템 생성 타이머가 끝났을 때 호출되는 함수

public:	
	APickUpSpawnPoint();

	virtual void Tick(float DeltaTime) override;

};
