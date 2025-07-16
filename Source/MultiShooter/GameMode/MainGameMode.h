#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MainGameMode.generated.h"

namespace MatchState
{
	extern MULTISHOOTER_API const FName Cooldown; // 매치 상태: 쿨다운 == 엔드 게임 이후 누가 승자인지 확인하는 시간
}

UCLASS()
class MULTISHOOTER_API AMainGameMode : public AGameMode
{
	GENERATED_BODY()

	// 리스폰시 가장 다른 플레이어와의 거리가 먼 플레이어 시작 위치를 선택하는 함수
	AActor* GetSpawnPointWithLargestMinimumDistance(TArray<AActor*> PlayerStarts, TArray<AActor*> Players) const;
	float GetMinimumDistance(const AActor* PlayerStart, const TArray<AActor*> Players) const;

	float CountDownTime = 0.0f; // 카운트다운 시간

protected:
	virtual void BeginPlay() override; // 게임 모드가 시작될 때 호출되는 함수

public:
	AMainGameMode();
	virtual void Tick(float DeltaTime) override; // 게임 모드의 틱 함수
	virtual void OnMatchStateSet() override; // 매치 상태가 변경될 때 호출되는 함수
	virtual void Logout(AController* Exiting) override; // 플레이어가 강종으로 나갈떄

	virtual void PlayerEliminated(class ABaseCharacter* EliminatedCharacter, class ABasePC* EliminatedController,class ABasePC* AttackerController ); 
	// 플레이어가 제거되었을 때 호출되는 함수
	virtual void RequestRespawn(class ACharacter* EliminatedCharacter, class AController* EliminatedController); 
	// 플레이어가 제거되었을 때 리스폰 요청하는 함수

	void PlayerLeftGame(class ACharacterState* PlayerLeaving); // 플레이어가 게임을 떠났을 때 호출되는 함수

	FORCEINLINE float GetCountDownTime() const { return CountDownTime; } // 카운트다운 시간 반환 함수

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.0f; // 워밍업 시간

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.0f; // 워밍업 시간

	float LevelStartingTime = 0.f; // 레벨 시작 시간

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.0f; // 워밍업 시간

	void SendChatMsg(const FText& Text, const FString& PlayerName);
};
