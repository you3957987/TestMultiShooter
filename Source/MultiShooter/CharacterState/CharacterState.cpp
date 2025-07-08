#include "CharacterState.h"
#include "MultiShooter/Character/BaseCharacter.h"
#include "MultiShooter/PlayerController/BasePC.h"
#include "Net/UnrealNetwork.h" // 네트워크 관련 헤더 파일



void ACharacterState::BeginPlay()
{
    Super::BeginPlay();
}

void ACharacterState::OnRep_Score()
{
    Super::OnRep_Score();

    BaseCharacter = BaseCharacter == nullptr ? Cast<ABaseCharacter>(GetPawn()) : BaseCharacter; // 캐릭터 상태를 관리하는 캐릭터 클래스 포인터를 가져옵니다.
    if( BaseCharacter == nullptr ) return;
    
    Controller = Controller == nullptr ? Cast<ABasePC>(GetPlayerController()) : Controller; // 플레이어 컨트롤러 포인터를 가져옵니다.
    if( Controller == nullptr ) return;

    Controller->SetHUDScore( GetScore() ); // 플레이어 컨트롤러의 HUD에 점수를 설정합니다.

}

void ACharacterState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ACharacterState, Defeats); // Defeats 변수를 네트워크에서 복제합니다.
    DOREPLIFETIME(ACharacterState, KilledBy); // KilledBy 변수를 네트워크에서 복제합니다.
}

void ACharacterState::OnRep_Defeats()
{
    BaseCharacter = BaseCharacter == nullptr ? Cast<ABaseCharacter>(GetPawn()) : BaseCharacter; // 캐릭터 상태를 관리하는 캐릭터 클래스 포인터를 가져옵니다.
    if( BaseCharacter == nullptr ) return;
    
    Controller = Controller == nullptr ? Cast<ABasePC>(GetPlayerController()) : Controller; // 플레이어 컨트롤러 포인터를 가져옵니다.
    if( Controller == nullptr ) return;

    Controller->SetHUDDefeat(Defeats); // 플레이어 컨트롤러의 HUD에 죽은 횟수를 설정합니다.
}

void ACharacterState::OnRep_KilledBy()
{
    BaseCharacter = BaseCharacter == nullptr ? Cast<ABaseCharacter>(GetPawn()) : BaseCharacter; // 캐릭터 상태를 관리하는 캐릭터 클래스 포인터를 가져옵니다.
    if( BaseCharacter == nullptr ) return;
    
    Controller = Controller == nullptr ? Cast<ABasePC>(GetPlayerController()) : Controller; // 플레이어 컨트롤러 포인터를 가져옵니다.
    if( Controller == nullptr ) return;

    Controller->SetHUDKilledBy(KilledBy); // 플레이어 컨트롤러의 HUD에 죽은 메시지를 설정합니다.
}

void ACharacterState::AddToScore(float ScoreAmount)
{
    SetScore(GetScore() + ScoreAmount); // 점수를 추가합니다.

    BaseCharacter = BaseCharacter == nullptr ? Cast<ABaseCharacter>(GetPawn()) : BaseCharacter; // 캐릭터 상태를 관리하는 캐릭터 클래스 포인터를 가져옵니다.
    if( BaseCharacter == nullptr ) return;
    
    Controller = Controller == nullptr ? Cast<ABasePC>(GetPlayerController()) : Controller; // 플레이어 컨트롤러 포인터를 가져옵니다.
    if( Controller == nullptr ) return;

    Controller->SetHUDScore(GetScore()); // 플레이어 컨트롤러의 HUD에 점수를 설정합니다.
}

void ACharacterState::AddToDefeat(int32 DefeatAmount)
{
    Defeats += DefeatAmount; // 죽은 횟수를 추가합니다.

    BaseCharacter = BaseCharacter == nullptr ? Cast<ABaseCharacter>(GetPawn()) : BaseCharacter; // 캐릭터 상태를 관리하는 캐릭터 클래스 포인터를 가져옵니다.
    if( BaseCharacter == nullptr ) return;
    
    Controller = Controller == nullptr ? Cast<ABasePC>(GetPlayerController()) : Controller; // 플레이어 컨트롤러 포인터를 가져옵니다.
    if( Controller == nullptr ) return;

    Controller->SetHUDDefeat(Defeats); // 플레이어 컨트롤러의 HUD에 죽은 횟수를 설정합니다.
}

void ACharacterState::UpdateDeathMessage(FString KillerName)
{
    KilledBy = KillerName; // 죽은 메시지를 업데이트합니다.

    BaseCharacter = BaseCharacter == nullptr ? Cast<ABaseCharacter>(GetPawn()) : BaseCharacter; // 캐릭터 상태를 관리하는 캐릭터 클래스 포인터를 가져옵니다.
    if( BaseCharacter == nullptr ) return;
    
    Controller = Controller == nullptr ? Cast<ABasePC>(GetPlayerController()) : Controller; // 플레이어 컨트롤러 포인터를 가져옵니다.
    if( Controller == nullptr ) return;

    Controller->SetHUDKilledBy(KilledBy); // 플레이어 컨트롤러의 HUD에 죽은 메시지를 설정합니다.
    
}