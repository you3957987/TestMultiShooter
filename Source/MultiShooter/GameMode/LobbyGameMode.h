#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

UCLASS()
class MULTISHOOTER_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()
	

public:
	virtual void PostLogin(APlayerController* NewPlayer) override; // 플레이어가 로그인할 때 호출되는 함수

protected:
    void StartGame();
};
