#include "OverHeadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

#include "GameSave.h"
#include "Menu.h"
#include "MultiShooter/PlayerController/BasePC.h"
#include "MultiShooter/CharacterState/CharacterState.h"

void UOverHeadWidget::NativeDestruct()
{
    //RemoveFromParent(); // 위젯을 레벨에서 제거합니다.
    Super::NativeDestruct();
}

void UOverHeadWidget::OnNameChanged(const FString& PlayerName)
{
    // 로컬 세이브 파일에도 이름을 저장합니다.
    if (SaveData)
    {
        SaveData->PlayerName = PlayerName;
        UGameplayStatics::SaveGameToSlot(SaveData, SaveSlot::ProfileSlot, 0);
    }
    
    // 즉각적인 시각적 피드백을 위해 로컬 위젯의 텍스트를 바로 변경합니다.
    SetDisplayText(PlayerName);
}

// 게임 인스턴스에서 이름 바꾸는 델리게이트 여기서 설정. 이거는 베이스 캐릭터 블루프린트에서 비긴 플레이에서 실행함.

void UOverHeadWidget::ShowPlayerNetRole(APawn* InPawn, FString PlayerNameTemp)
{
    if ( InPawn == nullptr ) return;
    
    APlayerState* PS = InPawn ? InPawn->GetPlayerState() : nullptr;
    
    TArray<UUserWidget*> FoundWidgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UMenu::StaticClass());
    UMenu* Menu = nullptr;
    if (FoundWidgets.Num() > 0)
    {
        Menu = Cast<UMenu>(FoundWidgets[0]); // umenu는 한번만 생성할꺼니깐 첫번째꺼만
    }
    if (Menu)
    {
        if ( Menu->OnPlayerNameSet.IsAlreadyBound(this, &UOverHeadWidget::OnNameChanged) == false ) // 이미 바인딩 되어있으면 또 바인딩 안되게
        {
            Menu->OnPlayerNameSet.AddDynamic(this, &UOverHeadWidget::OnNameChanged);
        }
    }
    
    FString PlayerName;
    SaveData = Cast<UGameSave>(UGameplayStatics::LoadGameFromSlot(SaveSlot::ProfileSlot, 0));
    
    if ( SaveData->PlayerName != NickName::DefaultName ) // 기본 이름이 아닐때만
    {
        PlayerName = SaveData->PlayerName;
    }
    else
    {
        // 기본 이름이면 ps 에서 스팀 이름 가져와서 savedata에 저장
        if ( PS && SaveData )
        {
            PlayerName = PS->GetPlayerName(); // 스팀 이름 가져오기
            SaveData->PlayerName = PlayerName;
            UGameplayStatics::SaveGameToSlot(SaveData, SaveSlot::ProfileSlot, 0);
        }
    }
    
    /*
    ENetRole RemoteRole = InPawn->GetLocalRole();
    FString Role;
    switch (RemoteRole)
    {
        case ENetRole::ROLE_Authority: Role = TEXT("Authority"); break;
        case ENetRole::ROLE_AutonomousProxy: Role = TEXT("Autonomous Proxy"); break;
        case ENetRole::ROLE_SimulatedProxy: Role = TEXT("Simulated Proxy"); break;
        default: Role = TEXT("None"); break;
    }
    */
    FString DisplayString = FString::Printf(TEXT("%s"), *PlayerName);
    SetDisplayText(DisplayString);
}

void UOverHeadWidget::SetPlayerName(const FString& PlayerName)
{
    SetDisplayText(PlayerName);
}

void UOverHeadWidget::BindMenuToHeadWidget()
{
    SaveData = Cast<UGameSave>(UGameplayStatics::LoadGameFromSlot(SaveSlot::ProfileSlot, 0));
    
    TArray<UUserWidget*> FoundWidgets;
    UWidgetBlueprintLibrary::GetAllWidgetsOfClass(GetWorld(), FoundWidgets, UMenu::StaticClass());
    UMenu* Menu = nullptr;
    if (FoundWidgets.Num() > 0)
    {
        Menu = Cast<UMenu>(FoundWidgets[0]); // umenu는 한번만 생성할꺼니깐 첫번째꺼만
    }
    if (Menu)
    {
        if ( Menu->OnPlayerNameSet.IsAlreadyBound(this, &UOverHeadWidget::OnNameChanged) == false ) // 이미 바인딩 되어있으면 또 바인딩 안되게
        {
            Menu->OnPlayerNameSet.AddDynamic(this, &UOverHeadWidget::OnNameChanged);
        }
    }
}

void UOverHeadWidget::SetDisplayText(const FString& Text)
{
    if (DisplayText)
    {
        DisplayText->SetText(FText::FromString(Text)); // 위젯의 텍스트를 설정합니다.
    }
}


