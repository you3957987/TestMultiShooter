#include "FlagGameMode.h"

#include "MultiShooter/Weapon/Flag.h"
#include "MultiShooter/Flag/FlagZone.h"
#include "MultiShooter/GameState/BaseGameState.h"



void AFlagGameMode::PlayerEliminated(ABaseCharacter* EliminatedCharacter, ABasePC* EliminatedController, ABasePC* AttackerController)
{
	AMainGameMode::PlayerEliminated(EliminatedCharacter, EliminatedController, AttackerController);
	// 팀 게임 모드가 아닌 메인 게임 모드의 플레이어 제거 함수 호출
}

void AFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* FlagZone)
{
	bool bValidCapture = Flag->GetTeam() != FlagZone->Team; // 깃발이 상대 팀의 존에 들어갔는지 확인
	ABaseGameState* BaseGameState = Cast<ABaseGameState>(GameState);
	if (GameState)
	{
		if (FlagZone->Team == ETeam::ET_BlueTeam)
		{
			BaseGameState->BlueTeamScoreAdd(); // 블루 팀의 점수 증가
		}
		else if (FlagZone->Team == ETeam::ET_RedTeam)
		{
			BaseGameState->RedTeamScoreAdd(); // 블루 팀의 점수 증가
		}
	}
}
