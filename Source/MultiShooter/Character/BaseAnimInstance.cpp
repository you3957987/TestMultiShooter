#include "BaseAnimInstance.h"
#include "BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UBaseAnimInstance::NativeInitializeAnimation()
{
    // 이 함수는 애니메이션이 시작될 때 한 번 호출됩니다.
    // 여기서 초기화 작업을 수행할 수 있습니다.
    Super::NativeInitializeAnimation();

    BaseCharacter = Cast<ABaseCharacter>(TryGetPawnOwner());
}

void UBaseAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
    // 이 함수는 매 프레임마다 호출됩니다.
    // 여기서 애니메이션 상태를 업데이트할 수 있습니다.
    Super::NativeUpdateAnimation(DeltaTime);

    if (BaseCharacter == nullptr) BaseCharacter = Cast<ABaseCharacter>(TryGetPawnOwner()); // 캐릭터가 아직 초기화되지 않았다면 다시 시도합니다.
    if (BaseCharacter == nullptr) return; // 캐릭터가 여전히 nullptr이면 함수 종료

    FVector Velocity = BaseCharacter->GetVelocity();
    Velocity.Z = 0; // Z축 속도는 무시합니다.
    Speed = Velocity.Size(); // 캐릭터의 속도를 계산합니다.

    bIsAccelerating = BaseCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.0f; // 캐릭터가 가속 중인지 확인합니다.

    bIsAir = BaseCharacter->GetCharacterMovement()->IsFalling(); // 캐릭터가 공중에 있는지 확인합니다.
 
}