#include "BTService/BTService_CheckSightAndDetect.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EnemyAiController.h"


UBTService_CheckSightAndDetect::UBTService_CheckSightAndDetect()
{
	NodeName = "Check Sight And DetectRange";
}

void UBTService_CheckSightAndDetect::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* TargetPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (TargetPawn == nullptr) return;

	AEnemyAiController* AIController = Cast<AEnemyAiController>(OwnerComp.GetAIOwner());
	if (AIController == nullptr) return;

	// 시야와 탐지 범위 확인
	if (AIController->LineOfSightTo(TargetPawn) && AIController->IsInArrangeOfDetectRange())
	{
		// 플레이어를 감지하면 블랙보드에 위치와 타겟 정보를 업데이트합니다.
		OwnerComp.GetBlackboardComponent()->SetValueAsVector(GetSelectedBlackboardKey(), TargetPawn->GetActorLocation());
	}
	else
	{
		OwnerComp.GetBlackboardComponent()->ClearValue(GetSelectedBlackboardKey());
	}
}

