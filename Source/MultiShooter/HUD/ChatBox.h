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
	virtual void NativeConstruct() override;

public:

	void OnTextCommitted(const FText& Text, const FString& PlayerName);

	UPROPERTY(meta = (BindWidget))
	UScrollBox* ChatTextBox;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* ChatInputBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	TSubclassOf<UChatTextBlock> ChatTextBlockClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	TArray<UChatTextBlock*> ChatTextBlocks;

};