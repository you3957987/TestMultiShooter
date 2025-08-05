#include "LobbyController.h"
#include "EnhancedInputComponent.h" // UEnhancedInputComponent
#include "InputAction.h"            // UInputAction (만약 IA_EnterToChat이 UInputAction* 타입이면)
#include "Components/EditableTextBox.h" // UEditableTextBox
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h" // UGameplayStatics

#include "MultiShooter/HUD/ChatBox.h"              // UChatBox
#include "MultiShooter/GameMode/LobbyGameMode.h"    // AMainGameMode


void ALobbyController::BeginPlay()
{
	Super::BeginPlay(); // 부모 클래스의 BeginPlay 함수를 호출합니다.
	
	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ALobbyController::OnMouseClickAnywhere);
		InputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &ALobbyController::OnMouseClickAnywhere);
	}
	AddChatBox();
}

void ALobbyController::SetupInputComponent()
{
	Super::SetupInputComponent(); // 부모 클래스의 SetupInputComponent 함수를 호출합니다.
	
	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	if (!EnhancedInput) return;

	EnhancedInput->BindAction(IA_EnterToChat, ETriggerEvent::Started, this, &ALobbyController::StartChat); // 채팅 입력 액션 바인딩
}

void ALobbyController::OnMouseClickAnywhere()
{
	if (bIsChatting)
	{
		// 챗 박스 외부 클릭 시 채팅 종료
		bIsChatting = false;
		FInputModeGameOnly InputModeGameOnly;
		SetInputMode(InputModeGameOnly);
		SetShowMouseCursor(false);
	}
}

void ALobbyController::StartChat()
{
	bIsChatting = !bIsChatting; // 채팅 상태를 토글합니다.

	if (bIsChatting)
	{
		ChatBox->SetVisibility(ESlateVisibility::Visible); // 항상 보이게
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(ChatBox->ChatInputBox->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // 마우스 잠금 모드를 설정합니다.
		SetInputMode(InputMode);
		SetShowMouseCursor(true);
	}
	else
	{
		FInputModeGameOnly InputModeGameOnly;
		SetInputMode(InputModeGameOnly);
		SetShowMouseCursor(false); // 마우스 커서 숨기기
	}
}

void ALobbyController::AddChatBox()
{
	// 로컬 컨트롤러에서만 채팅 박스 생성
	if (IsLocalController() && ChatBoxClass)
	{
		ChatBox = CreateWidget<UChatBox>(this, ChatBoxClass);
		if (ChatBox && ChatBox->ChatInputBox) // 채팅 박스가 유효하고 아직 뷰포트에 추가되지 않은 경우
		{
			ChatBox->AddToViewport(100);
			ChatBox->ChatInputBox->SetHintText(FText::FromString("Text.....")); // 채팅 입력 박스에 힌트 텍스트 설정
			ChatBox->SetVisibility(ESlateVisibility::Visible); // 기다리는 동안은 채팅 숨기기
			ChatBox->ChatInputBox->SetRevertTextOnEscape(true); // 채팅 입력 박스에서 Escape 키를 누르면 텍스트가 되돌려지도록 설정합니다.
			ChatBox->ChatInputBox->OnTextCommitted.AddDynamic(this, &ALobbyController::OnChatCommitted);
		}
	}
}

void ALobbyController::OnChatCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (CommitMethod != ETextCommit::OnEnter) return;

	FString PlayerName{ "" };

	APlayerState* TempPlayerState = GetPlayerState<APlayerState>();
	if (TempPlayerState)
	{
		PlayerName = TempPlayerState->GetPlayerName();
	}
	ServerChatCommitted(Text, PlayerName); // 서버에 채팅 메시지를 보냅니다.

	// 채팅 입력 후 다시 입력창에 포커스 & 마우스 커서 유지
	if (ChatBox && ChatBox->ChatInputBox)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(ChatBox->ChatInputBox->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		SetShowMouseCursor(true);
	}
}

void ALobbyController::ServerChatCommitted_Implementation(const FText& Text, const FString& PlayerName)
{
	ALobbyGameMode* GameMode = Cast<ALobbyGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		GameMode->SendChatMsg(Text, PlayerName);
	}
}

void ALobbyController::ClientChatCommitted_Implementation(const FText& Text, const FString& PlayerName)
{
	if (ChatBox)
	{
		ChatBox->OnTextCommitted(Text, PlayerName);
	}
}
