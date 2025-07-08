#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MultiSHooter/TypeHeader/TurningInPlace.h" // 조준시 좌우로 360도 회전하는게 아니라 90도 정도로만 돌아가게 만들기 위한 헤더
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

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	ETurningInPlace TurningInPlace; // 캐릭터가 조준 중일 때 좌우로 회전하는 상태를 제한하기 위한 열거형 변수!

	// 이거 91 강의 무슨 애니메이션 흔들림 없앤다는데 좀 이상해서 안씀. 안써도 그리 이상하지 않음
	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bRotateRootBone; // 루트 본을 회전할지 여부를 저장하는 변수
	// 이거 91 강의 무슨 애니메이션 흔들림 없앤다는데 좀 이상해서 안씀. 안써도 그리 이상하지 않음

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bDead; // 캐릭터가 죽었는지 여부를 저장하는 변수

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bReloading; // 캐릭터가 재장전 중인지 여부를 저장하는 변수

public:
	virtual void NativeInitializeAnimation() override; // 비긴 플레이랑 유사
	virtual void NativeUpdateAnimation(float DeltaTime) override; // 매 프레임마다 호출 틱이랑 유사
	
};
