#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "SoloPlayerStart.generated.h"

UCLASS()
class MULTISHOOTER_API ASoloPlayerStart : public APlayerStart
{
	GENERATED_BODY()
	
public:
	// 스폰 쿨다운 관련
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bSpawned = false;

	FTimerHandle SpawnCooldownTimerHandle;

	// 스폰 쿨다운 시작 함수
	void StartSpawnCooldown();

	// 쿨다운 해제 함수
	void ResetSpawnCooldown();

	// 현재 스폰 가능 여부 반환
	UFUNCTION()
	bool IsSpawned() const { return bSpawned; }
};
