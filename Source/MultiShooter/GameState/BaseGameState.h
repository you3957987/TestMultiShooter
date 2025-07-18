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

	//---------------------------------------------------------------------------------------------------팀 게임 관련 ㅜ

	TArray<class ACharacterState*> RedTeam; // 레드 플레이어 배열

	TArray<class ACharacterState*> BlueTeam; // 파랑 플레이어 배열
	
	UPROPERTY(ReplicatedUsing = "OnRep_RedTeamScore")
	float RedTeamScore = 0.f; // 레드 팀 점수
	UPROPERTY(ReplicatedUsing = "OnRep_BlueTeamScore")
	float BlueTeamScore = 0.f; // 블루 팀 점수

	void RedTeamScoreAdd();
	void BlueTeamScoreAdd();

	UFUNCTION()
	void OnRep_RedTeamScore(); // 레드 팀 점수 변경 시 호출되는 함수
	UFUNCTION()
	void OnRep_BlueTeamScore(); // 블루 팀 점수 변경 시 호출되는 함수

};
