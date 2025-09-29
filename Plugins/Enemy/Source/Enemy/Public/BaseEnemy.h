#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseEnemy.generated.h"

UCLASS()
class ENEMY_API ABaseEnemy : public ACharacter
{
	GENERATED_BODY()

	friend class AEnemyAiController; // EnemyAiController에서 BaseEnemy의 protected 멤버에 접근할 수 있도록 합니다.

protected:
	virtual void BeginPlay() override;

	void PollInit(); // 틱에서 하는 초기화
	
	UPROPERTY(EditAnywhere, Category="자체설정")
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, Category="자체설정")
	float Health = 100.f;

	UPROPERTY(EditAnywhere, Category="자체설정")
	float AttackRange = 100.f;// 이거 수정하면 자동으로 AttackRangeSphere의 반지름도 수정되게 만듬

	UPROPERTY(EditAnywhere, Category="자체설정")
	float DetectRange = 700.f; // AI가 플레이어를 인식하는 범위

	UPROPERTY(EditAnywhere, Category="자체설정")
	float ChaseRange = 1000.f; // AI가 플레이어를 추적하는 범위
	
	UPROPERTY(EditAnywhere, Category="자체설정")
	float AttackDelay = 1.5f; // 공격 딜레이
	
	// 타겟으로 삼을 캐릭터를 저장할 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "자체설정")
	TObjectPtr<ACharacter> TargetCharacter;
	bool bTargetInitalize = false; // 캐릭터 초기화 여부를 나타내는 변수
	
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* AttackRangeSphere; // 충돌 구 컴포넌트

	UPROPERTY(VisibleAnywhere)
	class USphereComponent* DetectRangeSphere; // 인식 범위 확인용 컴포넌트

	UPROPERTY(VisibleAnywhere)
	class USphereComponent* ChaseRangeSphere;; // 추적 범위 확인용 컴포넌트

public:
	ABaseEnemy();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// 데미지 처리 함수 재정의
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintPure)
	FORCEINLINE bool IsDead() const { return Health <= 0.f; }; // 체력이 0 이하이면 죽은 상태로 간주합니다.

	// ATargetPoint 액터를 패트롤 지점으로 사용하기 위한 새 프로퍼티
	UPROPERTY(EditAnywhere, Category = "자체설정", meta=(DisplayName="Patrol Points"))
	TArray<TObjectPtr<class ATargetPoint>> PatrolPoints;

	
	void Attack();

#if WITH_EDITOR
	// 에디터에서 프로퍼티가 변경될 때 호출됩니다.
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
