#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterUI.generated.h"


UCLASS()
class MULTISHOOTER_API UCharacterUI : public UUserWidget
{
	GENERATED_BODY()
	

public:

	UPROPERTY(meta = (BindWidget)) // 위젯 바인딩
	class UTextBlock* HealthText; // 플레이어 이름 텍스트 블록 바인딩

	UPROPERTY(meta = (BindWidget)) // 위젯 바인딩
	class UProgressBar* HealthBar; // 체력 바 바인딩
	
};
