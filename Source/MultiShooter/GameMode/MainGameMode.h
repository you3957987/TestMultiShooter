#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MainGameMode.generated.h"

UCLASS()
class MULTISHOOTER_API AMainGameMode : public AGameMode
{
	GENERATED_BODY()

	AActor* GetSpawnPointWithLargestMinimumDistance(TArray<AActor*> PlayerStarts, TArray<AActor*> Players) const;
	float GetMinimumDistance(const AActor* PlayerStart, const TArray<AActor*> Players) const;

protected:

public:
	virtual void PlayerEliminated(class ABaseCharacter* EliminatedCharacter, 
		class ABasePC* EliminatedController,
		class ABasePC* AttackerController
	); // 플레이어가 제거되었을 때 호출되는 함수

	virtual void RequestRespawn(class ACharacter* EliminatedCharacter, class AController* EliminatedController);
	// 플레이어가 제거되었을 때 리스폰 요청하는 함수
};
