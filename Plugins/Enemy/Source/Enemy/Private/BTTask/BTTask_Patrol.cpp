#include "BTTask/BTTask_Patrol.h"
#include "AIController.h"
#include "BaseEnemy.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Engine/TargetPoint.h"

UBTTask_Patrol::UBTTask_Patrol()
{
	NodeName = "Patrol";
}

EBTNodeResult::Type UBTTask_Patrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::ExecuteTask(OwnerComp, NodeMemory);

	if ( OwnerComp.GetAIOwner() == nullptr ) return EBTNodeResult::Failed;

	ABaseEnemy* Enemy = Cast<ABaseEnemy>( OwnerComp.GetAIOwner()->GetPawn() );
	if ( Enemy == nullptr ) return EBTNodeResult::Failed;

	BlackboardComp = BlackboardComp ? BlackboardComp : OwnerComp.GetBlackboardComponent();
	if ( BlackboardComp == nullptr ) return EBTNodeResult::Failed;
	
	if ( Enemy->PatrolPoints.Num() == 0 )
	{
		OwnerComp.GetAIOwner()->MoveToLocation(BlackboardComp->GetValueAsVector(TEXT("StartLocation")));
	}
	else if ( Enemy->PatrolPoints.Num() == 1 )
	{
		OwnerComp.GetAIOwner()->MoveToLocation(Enemy->PatrolPoints[0]->GetActorLocation());
	}
	else
	{
		
	}

	return EBTNodeResult::Succeeded;
}


