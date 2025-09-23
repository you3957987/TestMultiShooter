#include "OverHeadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"


void UOverHeadWidget::NativeDestruct()
{
    //RemoveFromParent(); // 위젯을 레벨에서 제거합니다.
    Super::NativeDestruct();

}

void UOverHeadWidget::ShowPlayerNetRole(APawn* InPawn, FString PlayerNameTemp)
{
    APlayerState* PS = InPawn ? InPawn->GetPlayerState() : nullptr;
    FString PlayerName = PS ? PS->GetPlayerName() : TEXT("Unknown");

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


