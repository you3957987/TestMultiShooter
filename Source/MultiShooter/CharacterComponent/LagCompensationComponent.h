#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;
	UPROPERTY()
	FRotator Rotation;
	UPROPERTY()
	FVector BoxExtent; // 박스 범위
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time; // 프레임 시간
	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo; // 히트박스 정보

};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed; // 히트가 확인되었는지 여부
	UPROPERTY()
	bool bHeadShot; // 헤드샷 여부
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTISHOOTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY()
	class ABaseCharacter* Character; // 이 컴포넌트를 사용하는 캐릭터를 저장합니다.
	UPROPERTY()
	class ABasePC* Controller; // 플레이어 컨트롤러를 저장합니다.

	TDoubleLinkedList<FFramePackage> FrameHistory; // 프레임 패키지를 저장하는 리스트
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float MaxRecordTime = 4.f; // 최대 기록 시간 (초 단위) 테스트용  4초, 실제로는 0.1초정도 4초면 4초동안 프레임을 저장하는 것

protected:
	virtual void BeginPlay() override;

	void SaveFramePackage(FFramePackage& Package); // 프레임 패키지를 저장하는 함수
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime); // 프레임 사이를 보간하는 함수
	FServerSideRewindResult ConfirmHit(const FFramePackage& Package, ABaseCharacter* HitCharacter, 
		const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation);
	void CacheBoxPosition(ABaseCharacter* HitCharacter, FFramePackage& OutFramePackage); // 캐릭터의 히트박스 위치를 저장하는 함수
	void MoveBoxes(ABaseCharacter* HitCharacter, const FFramePackage& Package); // 패키지 정보로 히트박스 위치를 이동하는 함수
	void ResetHitBoxes(ABaseCharacter* HitCharacter, const FFramePackage& Package); // 히트박스를 초기화하는 함수
	void EnabledCharacterMeshCollision(ABaseCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled); // 캐릭터 메쉬 충돌을 활성화/비활성화하는 함수

public:	
	ULagCompensationComponent();

	friend class ABaseCharacter; // ABaseCharacter 클래스에서 이 컴포넌트를 사용할 수 있도록 합니다.

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ShowFramePackage(const FFramePackage& Package, FColor Color); // 프레임 패키지를 보여주는 함수

	FServerSideRewindResult ServerSiedRewind(class ABaseCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation, float HitTime); // 서버 측에서 프레임 데이터를 이용하여 히트 리와인드하는 함수 187 강의부터 설명


};
