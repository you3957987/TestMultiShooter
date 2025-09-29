#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "BTTask_Patrol.generated.h"


UCLASS()
class ENEMY_API UBTTask_Patrol : public UBTTask_BlackboardBase
{
	GENERATED_BODY()

	UBlackboardComponent* BlackboardComp;
	
public:
	UBTTask_Patrol();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
