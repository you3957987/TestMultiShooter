#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatTextBlock.h"
#include "ChatBox.generated.h"

class UScrollBox;
class UEditableTextBox;
class UChatTextBlock;
class APlayerController;
class ABlasterPlayerState;

UCLASS()
class MULTISHOOTER_API UChatBox : public UUserWidget
{
	GENERATED_BODY()

	UPROPERTY()
	APlayerController* OwnerController;

	UPROPERTY()
	class ACharacterState* OwnerCharacterState;


protected:
	virtual void NativeConstruct() override; // 위젯이 생성될 때 호출되는 함수

public:

	// 텍스트가 커밋되었을 때 호출되는 함수
	void OnTextCommitted(const FText& Text, const FString& PlayerName);

	UPROPERTY(meta = (BindWidget))
	UScrollBox* ChatTextBox; // 채팅 치면 나오는 스크롤 되는 박스

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* ChatInputBox; // 채팅 입력 박스

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	TSubclassOf<UChatTextBlock> ChatTextBlockClass; // 이거는 이거 자기 넣어주면 됨

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	TArray<UChatTextBlock*> ChatTextBlocks;

};