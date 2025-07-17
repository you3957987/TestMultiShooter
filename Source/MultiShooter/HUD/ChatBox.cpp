#include "ChatBox.h"
#include "Components/ScrollBox.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "ChatTextBlock.h"
#include "MultiShooter/CharacterState/CharacterState.h"

// 위젯이 생성될 때 호출되는 함수.
void UChatBox::NativeConstruct()
{
	Super::NativeConstruct();

	OwnerController = GetOwningPlayer();
	OwnerCharacterState = OwnerController ? OwnerController->GetPlayerState<ACharacterState>() : nullptr;

	if (ChatInputBox)
	{
		ChatInputBox->SetHintText(FText::FromString("..........")); // 이거 의미 없음. 베이스 pc에서 자체적으로 설정함
		ChatInputBox->SetText(FText::GetEmpty());
	}
}

// 메인 게임 보드에서 ClientChatCommitted 호출하면 거기서 이거 호출
void UChatBox::OnTextCommitted(const FText& Text, const FString& PlayerName)
{
	OwnerController = OwnerController ? OwnerController : GetOwningPlayer();
	OwnerCharacterState = OwnerCharacterState == nullptr ? OwnerCharacterState = OwnerController->GetPlayerState<ACharacterState>() : OwnerCharacterState;
	if (ChatTextBlockClass && OwnerController)
	{
		if (OwnerCharacterState)
		{
			UChatTextBlock* ChatTextBlock = CreateWidget<UChatTextBlock>(OwnerController, ChatTextBlockClass);
			if (ChatTextBlock && ChatTextBlock->TextBlock)
			{
				const FString TextToDisplay = FString::Printf(TEXT("%s: %s"), *PlayerName, *Text.ToString());
				ChatTextBlock->TextBlock->SetText(FText::FromString(TextToDisplay));
				ChatTextBlocks.Add(ChatTextBlock);
				if (ChatTextBox)
				{
					ChatTextBox->AddChild(ChatTextBlock);
					ChatTextBox->ScrollToEnd();
				}
			}
		}
	}
	if (ChatInputBox)
	{
		ChatInputBox->SetText(FText::GetEmpty());
	}
}