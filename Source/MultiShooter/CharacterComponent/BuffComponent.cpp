#include "BuffComponent.h"
#include "MultiShooter/Character/BaseCharacter.h" // 캐릭터 클래스 헤더 파일
#include "GameFramework/CharacterMovementComponent.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();


	
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime); // 매 프레임마다 체력 회복 램프업 함수 호출
	ShieldRampUp(DeltaTime); // 매 프레임마다 쉴드 회복 램프업 함수 호출
}

void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	bHealing = true; // 체력 회복 시작
	HealingRate = HealAmount / HealingTime; // 체력 회복 속도 계산
	AmountToHeal += HealAmount; // 회복할 체력 양 설정
}

void UBuffComponent::HealRampUp(float DeltaTime)
{	
	if ( !bHealing || Character == nullptr || Character->IsDead() ) return; // 체력 회복 중이 아니거나 캐릭터가 nullptr이거나 죽은 경우 함수를 종료합니다.
	
	const float HealThisFrame = HealingRate * DeltaTime; // 현재 프레임에서 회복할 체력 양 계산

	Character->SetHealth( FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.0f, Character->GetMaxHealth()) ); // 캐릭터의 체력을 회복합니다.
	Character->UpdateHUDHealth(); // HUD 체력 업데이트
	
	AmountToHeal -= HealThisFrame; // 회복할 체력 양 감소
	if ( AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth() ) // 회복할 체력 양이 0 이하가 되면
	{
		bHealing = false; // 체력 회복 중지
		AmountToHeal = 0.0f; // 회복할 체력 양 초기화
	}
}

void UBuffComponent::Shield(float ShieldAmount, float ShieldTime)
{
	bShielding = true; // 쉴드 회복 시작
	ShieldingRate = ShieldAmount / ShieldTime; // 쉴드 회복 속도 계산
	AmountToShield += ShieldAmount; // 회복할 쉴드 양 설정
}

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if ( !bShielding || Character == nullptr || Character->IsDead() ) return; // 쉴드 회복 중이 아니거나 캐릭터가 nullptr이거나 죽은 경우 함수를 종료합니다.

	const float ShieldThisFrame = ShieldingRate * DeltaTime; // 현재 프레임에서 회복할 쉴드 양 계산

	Character->SetShield( FMath::Clamp(Character->GetShield() + ShieldThisFrame, 0.0f, Character->GetMaxShield()) ); // 캐릭터의 쉴드를 회복합니다.
	Character->UpdateHUDShield(); // HUD 방어력 업데이트

	AmountToShield -= ShieldThisFrame; // 회복할 쉴드 양 감소
	if ( AmountToShield <= 0.f || Character->GetShield() >= Character->GetMaxShield() ) // 회복할 쉴드 양이 0 이하가 되면
	{
		bShielding = false; // 쉴드 회복 중지
		AmountToShield = 0.0f; // 회복할 쉴드 양 초기화
	}
}

void UBuffComponent::SetInitialSpeed( float BaseSpeed )
{
	InitialSpeed = BaseSpeed; // 초기 속도를 설정합니다.
}

void UBuffComponent::BuffSpeed(float BaseSpeedBuff, float SpeedBuffTime)
{
	if( Character == nullptr || Character->IsDead() ) return; // 캐릭터가 nullptr이거나 죽은 경우 함수를 종료합니다.

	Character->GetWorldTimerManager().SetTimer(SpeedBuffTimer, this, &UBuffComponent::ResetSpeed, SpeedBuffTime, false); // 속도 버프 타이머 설정

	if( Character->GetCharacterMovement() )
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeedBuff; // 캐릭터의 최대 이동 속도를 증가시킵니다.
	}
	MulticastSpeedBuff(BaseSpeedBuff); // 모든 클라이언트에 속도 버프를 적용합니다.
}

void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed)
{
	if( Character && Character->GetCharacterMovement() )
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed; // 캐릭터의 최대 이동 속도를 증가시킵니다.
	}
}

void UBuffComponent::ResetSpeed()
{
	if( Character == nullptr && Character->GetCharacterMovement() )
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = InitialSpeed; // 캐릭터의 최대 이동 속도를 초기 속도로 되돌립니다.
	}
	MulticastSpeedBuff(InitialSpeed); // 모든 클라이언트에 초기 속도로 되돌립니다.
}

void UBuffComponent::SetInitialJumpZVelocity( float BaseJumpZVelocity )
{
	InitialJumpZVelocity = BaseJumpZVelocity; // 초기 점프 속도를 설정합니다.
}

void UBuffComponent::BuffJump(float JumpBuff, float JumpBuffTime)
{
	Character->GetWorldTimerManager().SetTimer(JumpBuffTimer, this, &UBuffComponent::ResetJump, JumpBuffTime, false); // 속도 버프 타이머 설정

	if( Character->GetCharacterMovement() )
	{
		Character->GetCharacterMovement()->JumpZVelocity = JumpBuff; // 캐릭터의 점프 속도를 증가시킵니다.
	}
	MulticastJumpBuff(JumpBuff); // 모든 클라이언트에 점프 버프를 적용합니다.
}

void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	if( Character && Character->GetCharacterMovement() )
	{
		Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity; // 캐릭터의 점프 속도 세팅
	}
}

void UBuffComponent::ResetJump()
{
	if( Character && Character->GetCharacterMovement() )
	{
		Character->GetCharacterMovement()->JumpZVelocity = InitialJumpZVelocity; // 캐릭터의 점프 속도를 초기 속도로 되돌립니다.
	}
	MulticastJumpBuff(InitialJumpZVelocity); // 모든 클라이언트에 초기 점프 속도로 되돌립니다.
}



