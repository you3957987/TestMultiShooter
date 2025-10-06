#include "EnemyBoss/BTTask/BTTask_SelectAttack.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_SelectAttack::UBTTask_SelectAttack()
{
	NodeName = "Select Attack";

	// FName으로만 사용 가능
	BlackboardKey.AddNameFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_SelectAttack, BlackboardKey));
}

EBTNodeResult::Type UBTTask_SelectAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (AttackPatterns.Num() == 0)
	{
		return EBTNodeResult::Failed;
	}

	float TotalWeight = 0.0f;
	for (const FAttackPattern& Pattern : AttackPatterns)
	{
		TotalWeight += Pattern.Weight;
	}

	if (TotalWeight <= 0.0f)
	{
		return EBTNodeResult::Failed;
	}

	const float RandomWeight = FMath::FRandRange(0.0f, TotalWeight);

	float CurrentWeightSum = 0.0f;
	FName SelectedAttackName = NAME_None;

	for (const FAttackPattern& Pattern : AttackPatterns)
	{
		CurrentWeightSum += Pattern.Weight;
		if (RandomWeight <= CurrentWeightSum)
		{
			SelectedAttackName = Pattern.AttackName;
			UE_LOG(LogTemp, Warning, TEXT("Selected Attack Pattern: %s"), *SelectedAttackName.ToString());
			break;
		}
	}

	if (UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent())
	{
		BlackboardComp->SetValueAsName(GetSelectedBlackboardKey(), SelectedAttackName);
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::Failed;
}
