#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseBossEnemy.generated.h"

UCLASS()
class ENEMY_API ABaseBossEnemy : public ACharacter
{
	GENERATED_BODY()

	friend class ABossAIController; // BossAIController에서 BaseBossEnemy의 protected 멤버에 접근할 수 있도록 합니다.
	
protected:
	virtual void BeginPlay() override;

	// 원거리 공격 타입일 때 발사체 클래스
	UPROPERTY(EditAnywhere, Category="자체설정")
	TSubclassOf<class ABaseEnemyProjectile> ProjectileClass;
	// 최대 체력
	UPROPERTY(EditAnywhere, Category="자체설정")
	float MaxHealth = 100.f;
	// 현재 체력
	UPROPERTY(EditAnywhere, Category="자체설정")
	float Health = 100.f;
	// 이동 속도 = 여기서 값 바꾸면 자동으로 캐릭터 무브먼트 컴포넌트의 MaxWalkSpeed도 바뀜
	UPROPERTY(EditAnywhere, Category="자체설정")
	float MoveSpeed = 300.f;
	// 공격 딜레이
	UPROPERTY(EditAnywhere, Category="자체설정")
	float AttackDelay = 1.5f;
	// 타겟으로 삼을 캐릭터를 저장할 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "자체설정")
	TObjectPtr<ACharacter> TargetCharacter;


	
public:
	ABaseBossEnemy();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;



#if WITH_EDITOR
	// 에디터에서 프로퍼티가 변경될 때 호출됩니다.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
