#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Menu.h"
#include "ServerList.generated.h"


UCLASS()
class MULTIPLAYERSESSIONS_API UServerList : public UUserWidget
{
	GENERATED_BODY()
	
	UPROPERTY(meta = (BindWidget))
	class UButton* ServerSelectButton; // 서버 선택 버튼

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ServerTextBlock; // 서버 이름 텍스트 블록

	UFUNCTION()
	void OnServerSelected();

	UPROPERTY()
	FString StoredSessionName;

	UPROPERTY()
	FString StoredMatchType;

	UPROPERTY()
	int32 StoredNumPlayer;

	UPROPERTY()
	int32 StoredMaxPlayer;

	UPROPERTY()
	UMenu* MenuWidget;

public:
	void SetServerListInfo(FString SessionName, FString MatchType, int32 NumPlayer, int32 MaxPlayer);

	void SetMenuWidget(UMenu* InMenuWidget) { MenuWidget = InMenuWidget; }
};
