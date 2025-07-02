#include "BaseAnimInstance.h"
#include "BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

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

    bWeaponEquip = BaseCharacter->IsWeaponEquipped(); // 캐릭터가 무기를 장착했는지 확인합니다.

    bIsAiming = BaseCharacter->IsAiming(); // 캐릭터가 조준 중인지 확인합니다.

    FRotator AimRotation = BaseCharacter->GetBaseAimRotation();
    FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BaseCharacter->GetVelocity()); // 캐릭터의 이동 방향을 기반으로 회전값을 계산합니다.
    //UE_LOG(LogTemp, Warning, TEXT("AimRotation Yaw: %f"), AimRotation.Yaw);

    FRotator DeltaROt = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation); // 이동 방향과 조준 방향의 차이를 계산합니다.
    DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaROt, DeltaTime, 15.0f); // 차이를 보간합니다.
    YawOffset = DeltaRotation.Yaw; // Yaw 오프셋을 저장합니다.

    CharacterRotationLastFrame = CharacterRotation; // 이전 프레임의 회전값을 저장합니다.
    CharacterRotation = BaseCharacter->GetActorRotation(); // 현재 프레임의 캐릭터 회전값을 가져옵니다.
    const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame); // 현재 프레임과 이전 프레임의 회전값 차이를 계산합니다.
    const float Target = Delta.Yaw / DeltaTime; // Yaw 차이를 가져옵니다.
    const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.0f); // Yaw 차이를 보간합니다.
    Lean = FMath::Clamp(Interp, -90.0f, 90.0f); // Lean 값을 -90도에서 90도 사이로 제한합니다.
    
    AO_Yaw = BaseCharacter->GetAO_Yaw(); // 캐릭터의 AO_Yaw 값을 가져옵니다.
    AO_Pitch = BaseCharacter->GetAO_Pitch(); // 캐릭터의 AO_Pitch 값을 가져옵니다.
}