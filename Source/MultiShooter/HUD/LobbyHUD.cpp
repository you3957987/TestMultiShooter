#include "LobbyHUD.h"
#include "MultiShooter/GameState/LobbyGameState.h" // Ãß°¡
#include "GameFramework/GameStateBase.h"
#include "Components/TextBlock.h"

void ULobbyHUD::UpdatePlayerCountFromGameState()
{
    int32 MaxPlayer = 77;
    int32 CurrentPlayer = 0;

    if (UWorld* World = GetWorld())
    {
        if (ALobbyGameState* LobbyGS = World->GetGameState<ALobbyGameState>())
        {
            MaxPlayer = LobbyGS->MaxPlayers;
            CurrentPlayer = LobbyGS->PlayerArray.Num();
        }
        else if (AGameStateBase* GameState = World->GetGameState())
        {
            CurrentPlayer = GameState->PlayerArray.Num();
        }
    }

    if (CurState)
    {
        CurState->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentPlayer, MaxPlayer)));
    }
    if (StartText)
    {
        if (CurrentPlayer == MaxPlayer)
        {
            StartText->SetVisibility(ESlateVisibility::Visible);
        }
        else
        {
            StartText->SetVisibility(ESlateVisibility::Collapsed);
        }
    }
}
