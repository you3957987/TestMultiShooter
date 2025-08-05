#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

UCLASS()
class MULTISHOOTER_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()

protected:
    void StartGame();


public:
	virtual void PostLogin(APlayerController* NewPlayer) override; // 플레이어가 로그인할 때 호출되는 함수

	void SendChatMsg(const FText& Text, const FString& PlayerName); // 플레이어가 채팅 메시지를 쓰면 다른 플레이어 전부에게 메시지 업데이트
};
