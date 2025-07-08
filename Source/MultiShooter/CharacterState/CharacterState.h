#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CharacterState.generated.h"

UCLASS()
class MULTISHOOTER_API ACharacterState : public APlayerState
{
	GENERATED_BODY()

	UPROPERTY()
	class ABaseCharacter* BaseCharacter; // 캐릭터 상태를 관리하는 캐릭터 클래스 포인터
	UPROPERTY()
	class ABasePC* Controller; // 플레이어 컨트롤러 포인터

	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;

	UPROPERTY(ReplicatedUsing = OnRep_KilledBy)
	FString KilledBy; // 죽은 메시지
	
public:
	virtual void OnRep_Score() override; // 점수가 변경되었을 때 호출되는 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_Defeats(); // Defeats가 변경되었을 때 호출되는 함수 (추가적인 로직이 필요할 경우 구현)
	UFUNCTION()
	void OnRep_KilledBy(); // KilledBy가 변경되었을 때 호출되는 함수 (추가적인 로직이 필요할 경우 구현)

	void AddToScore(float ScoreAmount); // 점수를 추가하는 함수
	void AddToDefeat(int32 DefeatAmount);  // 죽은 횟수를 추가하는 함수
	void UpdateDeathMessage( FString KillerName ); // 죽은 메시지를 업데이트하는 함수
	
};
