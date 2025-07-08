#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BaseGameState.generated.h"

UCLASS()
class MULTISHOOTER_API ABaseGameState : public AGameState
{
	GENERATED_BODY()

	float TopScore = 0.f; // 최고 점수

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void UpdateTopScore( class ACharacterState* ScoringPlayer ); // 최고 점수를 기록한 플레이어를 업데이트하는 함수

	UPROPERTY(Replicated)
	TArray<class ACharacterState*> TopScoreingPlayers; // 최고 점수를 기록한 플레이어들의 배열
	
};
