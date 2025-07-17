#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTISHOOTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY()
	class ABaseCharacter* Character; // 캐릭터 클래스 포인터

	// -----------------------------------------------------------------------------체력 관련ㅜ
	//
	bool bHealing = false; // 체력 회복 중인지 여부를 나타내는 변수
	float HealingRate = 0.0f; // 체력 회복 속도
	float AmountToHeal = 0.0f; // 회복할 체력 양
	//
	// -----------------------------------------------------------------------------체력 관련ㅗ

	// -----------------------------------------------------------------------------쉴드 관련ㅜ
	//
	bool bShielding = false; // 쉴드 회복 중인지 여부를 나타내는 변수
	float ShieldingRate = 0.0f; // 쉴드 회복 속도
	float AmountToShield = 0.0f; // 회복할 쉴드 양
	//
	// -----------------------------------------------------------------------------쉴드 관련ㅗ

	// -----------------------------------------------------------------------------속도 관련ㅜ
	//
	FTimerHandle SpeedBuffTimer; // 속도 버프 타이머 핸들
	void ResetSpeed();
	float InitialSpeed; // 초기 속도
	UFUNCTION( NetMulticast, Reliable )
	void MulticastSpeedBuff( float BaseSpeed );
	//
	// -----------------------------------------------------------------------------속도 관련ㅗ

	// -----------------------------------------------------------------------------점프 관련ㅜ
	//
	FTimerHandle JumpBuffTimer; // 점프 버프 타이머 핸들
	void ResetJump();
	float InitialJumpZVelocity; // 초기 점프 속도

	UFUNCTION( NetMulticast, Reliable )
	void MulticastJumpBuff( float JumpVelocity );
	//
	// -----------------------------------------------------------------------------점프 관련ㅗ

protected:
	virtual void BeginPlay() override;

	void HealRampUp(float DeltaTime); // 체력 회복을 위한 램프업 함수
	void ShieldRampUp(float DeltaTime); // 쉴드 회복을 위한 램프업 함수

public:	
	UBuffComponent();
	friend class ABaseCharacter; // ABaseCharacter 클래스가 이 컴포넌트에 접근할 수 있도록 합니다.

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	void Heal( float HealAmount, float HealingTime ); // 체력을 회복하는 함수
	void Shield( float ShieldAmount, float ShieldTime ); // 쉴드를 회복하는 함수

	void BuffSpeed( float BaseSpeedBuff, float SpeedBuffTime ); // 속도 버프를 적용하는 함수
	void SetInitialSpeed( float BaseSpeed );
	
	void BuffJump( float JumpBuff, float JumpBuffTime );
	void SetInitialJumpZVelocity( float BaseJumpZVelocity );
	
};
