#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyHUD.generated.h"


UCLASS()
class MULTISHOOTER_API ULobbyHUD : public UUserWidget
{
	GENERATED_BODY()
	

public:
	UPROPERTY(meta = (BindWidget)) // 위젯 바인딩
	class UTextBlock* CurState;

	UPROPERTY(meta = (BindWidget)) // 위젯 바인딩
	class UTextBlock* StartText;

	UFUNCTION(BlueprintCallable)
	void UpdatePlayerCountFromGameState();
};
