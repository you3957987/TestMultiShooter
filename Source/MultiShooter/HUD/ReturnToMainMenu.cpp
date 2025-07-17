#include "ReturnToMainMenu.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameModeBase.h"
#include "MultiShooter/Character/BaseCharacter.h"

// 위젯 처음 생성 시 호출되는 함수입니다.
bool UReturnToMainMenu::Initialize()
{
	if (Super::Initialize() == false) return false; // 부모 클래스의 Initialize 함수가 실패하면 false를 반환합니다.
	

	return true; // 초기화가 성공하면 true를 반환합니다.
}
// 리턴 버튼 클릭시 호출
void UReturnToMainMenu::MenuSetUp()
{
	AddToViewport(); // 뷰포트에 위젯을 추가합니다.
	SetVisibility(ESlateVisibility::Visible); // 위젯의 가시성을 설정합니다.
	bIsFocusable = true; // 위젯이 포커스를 받을 수 있도록 설정합니다.
	//UE_LOG(LogTemp, Error, TEXT("create"));
	UWorld* World = GetWorld(); // 현재 월드를 가져옵니다.
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController; // 첫 번째 플레이어 컨트롤러를 가져옵니다.
		if (PlayerController)
		{
			FInputModeGameAndUI InputModeData; // 게임과 UI 모드를 모두 사용할 수 있는 입력 모드를 생성합니다.
			InputModeData.SetWidgetToFocus(TakeWidget()); // 현재 위젯에 포커스를 설정합니다
			PlayerController->SetInputMode(InputModeData); // UI 모드로 입력 모드를 설정합니다.
			PlayerController->SetShowMouseCursor(true); // 마우스 커서를 표시합니다.
		}
	}
	if (ReturnButton && ReturnButton->OnClicked.IsBound() == false )
	{
		ReturnButton->OnClicked.AddDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
		// ReturnButton 클릭 시 ReturnButtonClicked 함수를 호출하도록 이벤트를 바인딩합니다.
	}

	UGameInstance* GameInstance = GetGameInstance(); // 현재 게임 인스턴스를 가져옵니다.
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionsSubsystem)
		{
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMainMenu::OnDestroySession);
		}
	}
}

// 리턴 다시 눌러서 다시 게임 시작
void UReturnToMainMenu::MenuTeardown()
{
	RemoveFromParent(); // 위젯을 부모로부터 제거합니다.
	UE_LOG(LogTemp, Error, TEXT("delete"));
	UWorld* World = GetWorld(); // 현재 월드를 가져옵니다.
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController; // 첫 번째 플레이어 컨트롤러를 가져옵니다.
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;

			PlayerController->SetInputMode(InputModeData); // UI 모드로 입력 모드를 설정합니다.
			PlayerController->SetShowMouseCursor(false); // 마우스 커서를 숨깁니다.
		}
	}
	if (ReturnButton && ReturnButton->OnClicked.IsBound() == true )
	{
		ReturnButton->OnClicked.RemoveDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
		// ReturnButton 클릭 시 ReturnButtonClicked 함수를 호출하도록 이벤트를 바인딩합니다.
	}
	if (MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound() == true )
	{
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UReturnToMainMenu::OnDestroySession);
	}
}

// 메인 메뉴로 나가기!!!!
void UReturnToMainMenu::ReturnButtonClicked()
{
	ReturnButton->SetIsEnabled(false); // 버튼을 비활성화합니다. (클릭 방지)

	UWorld* World = GetWorld(); // 현재 월드를 가져옵니다.
	if (World)
	{
		APlayerController* FirstPlayerContoller = World->GetFirstPlayerController();
		if (FirstPlayerContoller)
		{
			ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>( FirstPlayerContoller->GetPawn() ); // 플레이어의 폰을 가져옵니다.
			if (BaseCharacter)
			{
				BaseCharacter->ServerLeaveGame(); // 플레이어가 게임을 떠나도록 서버에 요청합니다.
				BaseCharacter->OnLeftGame.AddDynamic(this, &UReturnToMainMenu::OnPlayerLeftGame); // 게임을 떠날 때 호출되는 델리게이트를 바인딩합니다.
			}
			else
			{
				ReturnButton->SetIsEnabled(true); // BaseCharacter가 없는 경우 버튼을 다시 활성화합니다.
			}
		}
	}

}

// MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMainMenu::OnDestroySession);
// 이렇게 바인딩 되어 있음. 그냥 세션 파괴 되면 == 나가면 자동 호출	
//MenuSetUp 에서 바인딩 하고 MenuTeardown 하면서 바인딩 해제함 
void UReturnToMainMenu::OnDestroySession(bool bWasSuccessful) // 세션 파괴 후 호출되는 함수
{
	if (bWasSuccessful == false)
	{
		ReturnButton->SetIsEnabled(true);
		return; // 세션 파괴가 실패한 경우 버튼을 다시 활성화하고 함수를 종료합니다.
	}

	UWorld* World = GetWorld(); // 현재 월드를 가져옵니다.
	if (World)
	{
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
		if (GameMode)
		{
			GameMode->ReturnToMainMenuHost(); // 호스트 게임 모드에서 메인 메뉴로 돌아갑니다.
		}
		else
		{
			PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController; // 첫 번째 플레이어 컨트롤러를 가져옵니다.
			if (PlayerController)
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText()); // 클라이언트에서 메인 메뉴로 돌아갑니다.
			}
		}
	}
}

// 플레이어가 게임을 떠났을 때 호출되는 함수입니다. 여기서 세션을 파괴합니다.
void UReturnToMainMenu::OnPlayerLeftGame()
{
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->DestroySession(); // 세션을 파괴합니다.
	}
}
