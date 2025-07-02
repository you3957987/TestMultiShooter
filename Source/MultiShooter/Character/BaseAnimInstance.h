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

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bWeaponEquip; // 캐릭터가 무기를 장착했는지 여부를 저장합니다.

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bIsAiming; // 캐릭터가 조준 중인지 여부를 저장합니다

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float YawOffset; // 캐릭터의 Yaw 오프셋을 저장합니다. 조준 시에 사용됩니다.

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float Lean; // 캐릭터의 기울기를 저장합니다. 조준 시에 사용됩니다.

	FRotator CharacterRotationLastFrame; // 이전 프레임의 캐릭터 회전값을 저장합니다.
	FRotator CharacterRotation; // 현재 프레임의 캐릭터 회전값
	FRotator DeltaRotation; // 현재 프레임과 이전 프레임의 회전값 차이를 저장합니다.

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float AO_Pitch; // 캐릭터의 AO 피치를 저장합니다. 조준 시에 사용됩니다.

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float AO_Yaw; // 캐릭터의 AO 요 를 저장합니다. 조준 시에 사용됩니다.

public:
	virtual void NativeInitializeAnimation() override; // 비긴 플레이랑 유사
	virtual void NativeUpdateAnimation(float DeltaTime) override; // 매 프레임마다 호출 틱이랑 유사
	
};
