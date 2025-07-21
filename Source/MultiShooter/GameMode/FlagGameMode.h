#pragma once

#include "CoreMinimal.h"
#include "TeamsGameMode.h"
#include "FlagGameMode.generated.h"


UCLASS()
class MULTISHOOTER_API AFlagGameMode : public ATeamsGameMode
{
	GENERATED_BODY()
	

public:
	virtual void PlayerEliminated(class ABaseCharacter* EliminatedCharacter,
		class ABasePC* EliminatedController, class ABasePC* AttackerController) override; // 깃발 게임 전용 플레이어 제거시 함수

	void FlagCaptured(class AFlag* Flag, class AFlagZone* FlagZone);
};
