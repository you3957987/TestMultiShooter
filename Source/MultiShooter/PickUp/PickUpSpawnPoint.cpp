#include "PickUpSpawnPoint.h"
#include "PickUp.h" // 픽업 아이템 헤더 파일

APickUpSpawnPoint::APickUpSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true; // 이 액터는 네트워크에서 복제됩니다.

}

void APickUpSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
	StartSpawnPickUpTimer((AActor*)nullptr); // 픽업 아이템 생성 타이머 시작
}

void APickUpSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// 비긴 플레이에서 실행 후 FinishedSpawnPickUpTimer 실행
void APickUpSpawnPoint::StartSpawnPickUpTimer(AActor* DestroyedActor)
{
	const float SpawnTime = FMath::RandRange(SpawnPickUpTimeMin, SpawnPickUpTimeMax); // 픽업 아이템 생성 주기를 랜덤하게 설정합니다.
	GetWorldTimerManager().SetTimer(SpawnPickUpTimer, 
		this, 
		&APickUpSpawnPoint::FinishedSpawnPickUpTimer, 
		SpawnTime); // 타이머를 설정하여 일정 시간 후에 SpawnPickUp 함수를 호출합니다.
}

// 서버에서만 SpawnPickUp 실행
void APickUpSpawnPoint::FinishedSpawnPickUpTimer()
{
	if( HasAuthority() ) // 서버에서만 실행되도록 합니다.
	{
		SpawnPickUp(); // 픽업 아이템을 생성합니다.	
	}
}

void APickUpSpawnPoint::SpawnPickUp()
{
	int32 NumPickupClasses = PickUpClasses.Num(); // 픽업 아이템 클래스의 개수를 가져옵니다.
	if (NumPickupClasses > 0)
	{
		SpawnedPickUp = GetWorld()->SpawnActor<APickUp>(PickUpClasses[FMath::RandRange(0, NumPickupClasses - 1)], GetActorTransform()); // 랜덤하게 픽업 아이템을 생성합니다.
		if (HasAuthority()) // 서버에서만 실행되도록 합니다.
		{
			SpawnedPickUp->OnDestroyed.AddDynamic(this, &APickUpSpawnPoint::StartSpawnPickUpTimer); // 생성된 픽업 아이템이 파괴될 때 StartSpawnPickUpTimer 함수를 호출하도록 바인딩합니다.
		}
	}
}

