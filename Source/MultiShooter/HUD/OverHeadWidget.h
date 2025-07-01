#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverHeadWidget.generated.h"


UCLASS()
class MULTISHOOTER_API UOverHeadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY( meta = (BindWidget) )
	class UTextBlock* DisplayText;

	void SetDisplayText(const FString& Text);
	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole( APawn* InPawn, FString PlayerName );

protected:
	virtual void NativeDestruct() override;
	
};
