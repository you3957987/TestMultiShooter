#pragma once

#include "CoreMinimal.h"
#include "MainGameMode.h"
#include "TeamsGameMode.generated.h"


UCLASS()
class MULTISHOOTER_API ATeamsGameMode : public AMainGameMode // 기본적인 메인 게임 모드 가져오고 추가적인 팀 관리만 추가!
{
	GENERATED_BODY()
	

protected:
	virtual void HandleMatchHasStarted() override; // 게임 시작 시 호출되는 함수 -> 팀 분류 작업

public:
	ATeamsGameMode();
	virtual void PostLogin(APlayerController* NewPlayer) override; // 플레이어가 로그인할 때 호출되는 함수
	virtual void Logout(AController* Exiting) override; // 플레이어가 강종으로 나갈 때 호출되는 함수

	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override; // 팀킬 방지용 함수 오버라이드

	virtual void PlayerEliminated(class ABaseCharacter* EliminatedCharacter, 
		class ABasePC* EliminatedController, class ABasePC* AttackerController) override; // 팀게임 전용 플레이어 제거시 함수

};
