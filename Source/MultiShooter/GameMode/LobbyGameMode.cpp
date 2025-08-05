#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "MultiplayerSessionsSubsystem.h"
#include "TimerManager.h"

#include "MultiShooter/PlayerController/LobbyController.h" // ALobbyController
#include "MultiShooter/GameState/LobbyGameState.h" // ALobbyGameState

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
				world->ServerTravel("/Game/Maps/GameMap?listen");
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

	int32 NumOfPlayer = GameState.Get()->PlayerArray.Num();

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (Subsystem)
		{
			// MaxPlayers를 항상 최신 값으로 세팅 (최초 한 번만 세팅해도 무방)
			if (ALobbyGameState* LobbyGS = GetGameState<ALobbyGameState>())
			{
				LobbyGS->MaxPlayers = Subsystem->DesiredNumPublicConnections;
			}

			// 플레이어 수가 최대치에 도달하면 게임 시작 타이머 설정
			if (NumOfPlayer == Subsystem->DesiredNumPublicConnections)
			{
				FTimerHandle GameStartTimer;
				GetWorldTimerManager().SetTimer(
					GameStartTimer,
					this,
					&ALobbyGameMode::StartGame,
					5.f,
					false
				);
			}
		}
	}
}

// 플레이어가 채팅 쓰면 다른 플레이어 컨틀롤러 전부 순회 후 ClientChatCommitted 함수를 호출하여 채팅 메시지를 전송
void ALobbyGameMode::SendChatMsg(const FText& Text, const FString& PlayerName)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ALobbyController* PC = Cast<ALobbyController>(*It);
		if (PC)
		{
			PC->ClientChatCommitted(Text, PlayerName);
		}
	}
}