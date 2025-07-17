#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    int32 NumOfPlayer = GameState.Get()->PlayerArray.Num(); // 플레이어가 로그인할 때 게임 상태의 플레이어 배열을 업데이트합니다.

    if( NumOfPlayer == 2 ) // 플레이어 수가 일정 수 이상이면 게임 맵으로 이동.
    {
        UWorld *world = GetWorld();
        if( world )
        {
            bUseSeamlessTravel = true; // 원활한 전환을 위해 Seamless Travel을 활성화합니다.
            world->ServerTravel("/Game/Maps/GameMap?listen"); // 플레이어가 2명일 때 게임 맵으로 이동합니다.
        }
    }

}