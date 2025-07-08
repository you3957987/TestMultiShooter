#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Announcement.generated.h"

UCLASS()
class MULTISHOOTER_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget)) // 위젯 바인딩
	class UTextBlock* WarmUpTimeText;
	
	UPROPERTY(meta = (BindWidget)) // 위젯 바인딩
	class UTextBlock* AnnouncementText; 

	UPROPERTY(meta = (BindWidget)) // 위젯 바인딩
	class UTextBlock* InfoText; 
};
