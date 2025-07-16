#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

// 히트박스 정보를 저장하는 구조체
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

// 프레임 정보. 히트 시간, 히트박스 정보, 캐릭터 정보를 저장하는 구조체
USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time; // 프레임 시간
	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo; // 히트박스 정보

	UPROPERTY()
	ABaseCharacter* Character; // 이 프레임 패키지를 저장한 캐릭터
};

// 서버 사이드 리와인드 결과를 저장하는 구조체 == 히트가 확인되었는지 여부와 헤드샷 여부를 저장합니다.
USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed; // 히트가 확인되었는지 여부
	UPROPERTY()
	bool bHeadShot; // 헤드샷 여부
};

// 샷건용 서버 사이드 리와인드 결과 == 헤드샷과 몸샷 횟수를 저장하는 구조체
USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap< ABaseCharacter*, uint32 > HeadShot; // 헤드샷 횟수

	UPROPERTY()
	TMap< ABaseCharacter*, uint32 > BodyShot; //  몸샷 횟수
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

	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime); // 프레임 사이를 보간하는 함수

	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package, 
		ABaseCharacter* HitCharacter, 
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize& HitLocation
	);
	FShotgunServerSideRewindResult ShotgunConfirmHit(
		const TArray<FFramePackage>& Packages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations
	);

	FServerSideRewindResult ProjectileConfirmHit(
		const FFramePackage& Package,
		ABaseCharacter* HitCharacter, 
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize100& InitialVelocity, 
		float HitTime
	); // 발사체 히트 확인 함수

	void SaveFramePackage(FFramePackage& Package); // 프레임 패키지를 저장하는 함수
	FFramePackage GetFrameToCheck(ABaseCharacter* HitCharacter, float HitTime);
	void CacheBoxPosition(ABaseCharacter* HitCharacter, FFramePackage& OutFramePackage); // 캐릭터의 히트박스 위치를 저장하는 함수
	void MoveBoxes(ABaseCharacter* HitCharacter, const FFramePackage& Package); // 패키지 정보로 히트박스 위치를 이동하는 함수
	void ResetHitBoxes(ABaseCharacter* HitCharacter, const FFramePackage& Package); // 히트박스를 초기화하는 함수
	void EnabledCharacterMeshCollision(ABaseCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled); // 캐릭터 메쉬 충돌을 활성화/비활성화하는 함수
	void SetFramePackageOnTick(); // 틱 마다 프레임 패키지를 설정하는 함수

public:	
	ULagCompensationComponent();

	friend class ABaseCharacter; // ABaseCharacter 클래스에서 이 컴포넌트를 사용할 수 있도록 합니다.

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ShowFramePackage(const FFramePackage& Package, FColor Color); // 프레임 패키지를 보여주는 함수

	FServerSideRewindResult ServerSideRewind(
		class ABaseCharacter* HitCharacter, 
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation, 
		float HitTime
	); // 서버 측에서 프레임 데이터를 이용하여 히트 리와인드하는 함수 187 강의부터 설명

	FServerSideRewindResult ProjectileServerSideRewind(
		ABaseCharacter* HitCharacter, 
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize100& IntialVelocity,
		float HitTime
	);

	FShotgunServerSideRewindResult ShotgunServerSideRewind(
		const TArray<ABaseCharacter*> HitCharacters,
		const FVector_NetQuantize& TraceStart, 
		const TArray<FVector_NetQuantize>& HitLocations, 
		float HitTime
	);

	// 서버 사이드 리와인드 결과를 바탕으로 대미지를 적용하는 함수
	UFUNCTION( Server, Reliable )
	void ServerScoreRequest(
		ABaseCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime
	);

	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(
		const TArray<ABaseCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart, 
		const TArray<FVector_NetQuantize>& HitLocations, 
		float HitTime
	);

	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(
		ABaseCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize100& InitialVelocity, 
		float HitTime
	);

};
