#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "MultiShooter/TypeHeader/Team.h"
#include "TeamPlayerStart.generated.h"

UCLASS()
class MULTISHOOTER_API ATeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "자체설정")
	ETeam Team; // 팀 정보

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
