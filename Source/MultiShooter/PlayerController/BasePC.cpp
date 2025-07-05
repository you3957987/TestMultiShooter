#include "BasePC.h"
#include "MultiShooter/HUD/CharacterHUD.h"
#include "MultiShooter/HUD/CharacterUI.h"
#include "Components/Progressbar.h"
#include "Components/TextBlock.h"


void ABasePC::BeginPlay()
{
    Super::BeginPlay();

    CharacterHUD = Cast<ACharacterHUD>(GetHUD()); // 현재 플레이어의 HUD를 캐릭터 HUD로 캐스팅합니다.
}

void ABasePC::SetHUDHealth(float Health, float MaxHealth)
{
    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD; // 캐릭터 HUD가 nullptr인 경우 현재 플레이어의 HUD를 캐스팅합니다.

    if (CharacterHUD && CharacterHUD->CharacterUI 
        && CharacterHUD->CharacterUI->HealthBar && CharacterHUD->CharacterUI->HealthText ) // 캐릭터 HUD와 캐릭터 UI가 유효한 경우
    {
        const float HealthPercent = Health / MaxHealth; // 체력 비율 계산
        CharacterHUD->CharacterUI->HealthBar->SetPercent(HealthPercent); // 체력 바의 비율을 설정합니다.
        
        FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth) ); // 체력 텍스트 포맷팅
        CharacterHUD->CharacterUI->HealthText->SetText(FText::FromString(HealthText)); // 체력 텍스트를 설정합니다.
    }
}

