#include "OverHeadWidget.h"
#include "Components/TextBlock.h"


void UOverHeadWidget::NativeDestruct()
{
    //RemoveFromParent(); // 위젯을 레벨에서 제거합니다.
    Super::NativeDestruct();

}

void UOverHeadWidget::ShowPlayerNetRole(APawn* InPawn , FString PlayerName)
{
    ENetRole RemoteRole = InPawn->GetLocalRole(); // 플레이어의 네트워크 역할을 가져옵니다.
    
    FString Role;
    switch (RemoteRole)
    {
        case ENetRole::ROLE_Authority: // 권한이 있는 경우(서버는 모든 캐릭터에게 권한이 있음)
            Role = TEXT("Authority");
            break;
        case ENetRole::ROLE_AutonomousProxy: // 자율 프록시인 경우( 클라가 조종중인 캐릭터)
            Role = TEXT("Autonomous Proxy");
            break;
        case ENetRole::ROLE_SimulatedProxy: // 시뮬레이션된 프록시인 경우( 클라가 다른 캐릭터를 볼 때 )
            Role = TEXT("Simulated Proxy");
            break;
        case ENetRole::ROLE_None: // 역할이 없는 경우
        default: // 기본값은 None
            Role = TEXT("None");
            break;
    }
    // 이름과 권한을 모두 표시
    //FString DisplayString = FString::Printf(TEXT("이름: %s\n권한: %s"), *PlayerName, *Role);
    FString DisplayString = FString::Printf(TEXT("이름: %s"), *PlayerName);
    SetDisplayText(DisplayString); // 위젯에 네트워크 역할을 표시합니다.
}

void UOverHeadWidget::SetDisplayText(const FString& Text)
{
    if (DisplayText)
    {
        DisplayText->SetText(FText::FromString(Text)); // 위젯의 텍스트를 설정합니다.
    }
}


