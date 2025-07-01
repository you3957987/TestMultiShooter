#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "BaseAnimInstance.generated.h"

UCLASS()
class MULTISHOOTER_API UBaseAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class ABaseCharacter* BaseCharacter; // 이 애니메이션 인스턴스가 소속된 캐릭터를 참조합니다.

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float Speed; // 캐릭터의 속도를 저장합니다.

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bIsAir; // 캐릭터가 공중에 있는지 여부를 저장합니다.

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating; // 캐릭터가 가속 중인지 여부를 저장합니다.

public:
	virtual void NativeInitializeAnimation() override; // 비긴 플레이랑 유사
	virtual void NativeUpdateAnimation(float DeltaTime) override; // 매 프레임마다 호출 틱이랑 유사
	
};
