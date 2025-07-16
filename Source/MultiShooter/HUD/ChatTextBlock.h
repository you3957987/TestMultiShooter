#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatTextBlock.generated.h"


UCLASS()
class MULTISHOOTER_API UChatTextBlock : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TextBlock;
};
