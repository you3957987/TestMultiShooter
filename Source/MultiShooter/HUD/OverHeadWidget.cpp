#include "OverHeadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

#include "MultiShooter/GameInstance/BaseGameInstance.h"

void UOverHeadWidget::NativeDestruct()
{
    //RemoveFromParent(); // 위젯을 레벨에서 제거합니다.
    Super::NativeDestruct();

}

void UOverHeadWidget::OnNameChanged()
{
    UBaseGameInstance* GameInstance = Cast<UBaseGameInstance>(GetGameInstance());
    
    if ( GameInstance )
    {
        FString PlayerName = GameInstance->PlayerName;
        SetDisplayText(PlayerName); // 위젯의 텍스트를 설정합니다.
    }
}

// 게임 인스턴스에서 이름 바꾸는 델리게이트 여기서 설정
void UOverHeadWidget::ShowPlayerNetRole(APawn* InPawn, FString PlayerNameTemp)
{
    APlayerState* PS = InPawn ? InPawn->GetPlayerState() : nullptr;

    UBaseGameInstance* GameInstance = Cast<UBaseGameInstance>(GetGameInstance());

    FString PlayerName;
    if (GameInstance)
    {
        // 중복 바인딩 방지
        if (!GameInstance->OnNameChanged.IsAlreadyBound(this, &UOverHeadWidget::OnNameChanged))
        {
            GameInstance->OnNameChanged.AddDynamic(this, &UOverHeadWidget::OnNameChanged);
        }
        if ( GameInstance->PlayerName != "VICE" ) // 기본 이름이 아닐때만
        {
            PlayerName = GameInstance->PlayerName;
        }
        else
        {
            PlayerName = PS ? PS->GetPlayerName() : TEXT("Unknown"); //이거는 스팀 이름 가져오기
        }
    }
    
    ENetRole RemoteRole = InPawn->GetLocalRole();
    FString Role;
    switch (RemoteRole)
    {
        case ENetRole::ROLE_Authority: Role = TEXT("Authority"); break;
        case ENetRole::ROLE_AutonomousProxy: Role = TEXT("Autonomous Proxy"); break;
        case ENetRole::ROLE_SimulatedProxy: Role = TEXT("Simulated Proxy"); break;
        default: Role = TEXT("None"); break;
    }
    FString DisplayString = FString::Printf(TEXT("%s"), *PlayerName);
    SetDisplayText(DisplayString);
}

void UOverHeadWidget::SetDisplayText(const FString& Text)
{
    if (DisplayText)
    {
        DisplayText->SetText(FText::FromString(Text)); // 위젯의 텍스트를 설정합니다.
    }
}


