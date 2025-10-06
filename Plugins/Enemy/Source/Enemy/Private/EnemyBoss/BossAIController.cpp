#include "EnemyBoss/BossAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "EnemyBoss/BaseBossEnemy.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"

void ABossAIController::BeginPlay()
{
	Super::BeginPlay();
	GameState = UGameplayStatics::GetGameState(GetWorld());
}

void ABossAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PollInit();
}

void ABossAIController::PollInit()
{
	if ( bFocusToPlayer == false )
	{
		if ( GameState && GameState->HasMatchStarted() ) // 매치 시작되고 생성되는 플레이어 생성 시점에 플레이어한테 포커스
		{
			Targetpawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0); // 타깃 폰 설정
			
			ControlledEnemy = Cast<ABaseBossEnemy>(GetPawn());

			if ( BehaviorTree )
			{
				RunBehaviorTree(BehaviorTree); // B 트리 실행 + 이걸 가장 먼저 해야 블랙보드 컴포넌트가 생성됨
				
				BlackboardComp = GetBlackboardComponent();
				if ( BlackboardComp && Targetpawn )
				{
					BlackboardComp->SetValueAsFloat("AttackDelay", ControlledEnemy->AttackDelay);
					BlackboardComp->SetValueAsBool("CanAttack", true);
				}
			}
			bFocusToPlayer = true;
		}
	}
}
