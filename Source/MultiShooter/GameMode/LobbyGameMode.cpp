#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "MultiplayerSessionsSubsystem.h"
#include "TimerManager.h"

void ALobbyGameMode::StartGame()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (Subsystem == nullptr) return;

		UWorld* world = GetWorld();
		if (world)
		{
			bUseSeamlessTravel = true; // 원활한 전환을 위해 Seamless Travel을 활성화합니다.

			FString MatchType = Subsystem->DesiredMatchType;
			if (MatchType == "FreeForAll")
			{
				world->ServerTravel("/Game/Maps/GameMap?listen"); // 플레이어가 2명일 때 게임 맵으로 이동합니다.
			}
			else if (MatchType == "Team")
			{
				world->ServerTravel("/Game/Maps/TeamGameMap?listen");
			}
			else if (MatchType == "TeamFlag")
			{
				world->ServerTravel("/Game/Maps/TeamFlagGameMap?listen");
			}
		}
	}
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    int32 NumOfPlayer = GameState.Get()->PlayerArray.Num(); // 플레이어가 로그인할 때 게임 상태의 플레이어 배열을 업데이트합니다.

	UGameInstance* GameInstance = GetGameInstance();
    if (GameInstance)
    {
		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		check(Subsystem); // 서브시스템이 유효한지 확인합니다.

        if (NumOfPlayer == Subsystem->DesiredNumPublicConnections) // 플레이어 수가 일정 수 이상이면 게임 맵으로 이동.
        {
			FTimerHandle GameStartTimer;
			GetWorldTimerManager().SetTimer(
				GameStartTimer,
				this,
				&ALobbyGameMode::StartGame,
				3.f,
				false
			);
        }
    }
}