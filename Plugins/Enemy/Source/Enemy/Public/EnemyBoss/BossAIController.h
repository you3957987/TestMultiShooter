#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BossAIController.generated.h"


UCLASS()
class ENEMY_API ABossAIController : public AAIController
{
	GENERATED_BODY()
	
	class AGameStateBase* GameState;

	class ABaseBossEnemy* ControlledEnemy;

	class APawn* Targetpawn;

	UPROPERTY(EditAnywhere, Category="자체설정")
	class UBehaviorTree* BehaviorTree; // B 트리

	class UBlackboardComponent* BlackboardComp; // 블랙보드 컴포넌트
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	
	void PollInit(); // 틱에서 하는 초기화
	bool bFocusToPlayer = false; // 타깃 플레이어 설정 후 행동 트리 초기화용
};
