#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "LobbyGameState.generated.h"


UCLASS()
class MULTISHOOTER_API ALobbyGameState : public AGameState
{
	GENERATED_BODY()

public:
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Lobby")
    int32 MaxPlayers = 0;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
