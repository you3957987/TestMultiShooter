#include "TeamsGameMode.h"
#include "Kismet/GameplayStatics.h"

#include "MultiShooter/TypeHeader/Team.h"
#include "MultiShooter/CharacterState/CharacterState.h"
#include "MultiShooter/GameState/BaseGameState.h"
#include "MultiShooter/PlayerController/BasePC.h"

ATeamsGameMode::ATeamsGameMode()
{
	bTeamMatch = true; // 팀 매치 여부를 true로 설정
}

// 게임 시작 시 호출되는 함수로, 플레이어들을 팀에 분류하는 작업을 수행
void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	ABaseGameState* BaseGameState = Cast<ABaseGameState>(UGameplayStatics::GetGameState(this));
	if (BaseGameState)
	{
		for (auto P_State : BaseGameState->PlayerArray) // 게임 상태에 있는 플레이어 상태들을 순회하면서
		{
			ACharacterState* CharacterState = Cast<ACharacterState>(P_State.Get());
			if (CharacterState && CharacterState->GetTeam() == ETeam::ET_NoTeam)
			{
				if (BaseGameState->BlueTeam.Num() >= BaseGameState->RedTeam.Num()) // 레드팀이 더 적으면 레드팀 추가
				{
					BaseGameState->RedTeam.Add(CharacterState);
					CharacterState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					BaseGameState->BlueTeam.Add(CharacterState);
					CharacterState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}

// 플레이어가 중간에 들어오면 팀을 분류!
void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ABaseGameState* BaseGameState = Cast<ABaseGameState>(UGameplayStatics::GetGameState(this));
	if (BaseGameState)
	{
		ACharacterState* CharacterState = Cast<ACharacterState>(NewPlayer->PlayerState); // 순회 없이 그냥 중간에 들어온 놈만 체크

		if (CharacterState && CharacterState->GetTeam() == ETeam::ET_NoTeam) // 팀이 없는 플레이어만 처리
		{
			if (BaseGameState->BlueTeam.Num() >= BaseGameState->RedTeam.Num()) // 레드팀이 더 적으면 레드팀 추가
			{
				BaseGameState->RedTeam.Add(CharacterState);
				CharacterState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				BaseGameState->BlueTeam.Add(CharacterState);
				CharacterState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

// 플레이어가 강종이나 그냥 나갈 때 호출되는 함수
void ATeamsGameMode::Logout(AController* Exiting)
{
	ABaseGameState* BaseGameState = Cast<ABaseGameState>(UGameplayStatics::GetGameState(this));

	if (BaseGameState)
	{
		ACharacterState* CharacterState = Cast<ACharacterState>(Exiting->PlayerState);
		if (BaseGameState && CharacterState)
		{
			if (BaseGameState->RedTeam.Contains(CharacterState))
			{
				BaseGameState->RedTeam.Remove(CharacterState); // 레드팀에서 제거
			}
			if (BaseGameState->BlueTeam.Contains(CharacterState))
			{
				BaseGameState->BlueTeam.Remove(CharacterState); // 블루팀에서 제거
			}
		}
	}
}

// 팀킬 방지용 피해 계산 함수
float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	ACharacterState* AttackerState = Attacker->GetPlayerState<ACharacterState>();
	ACharacterState* VictimState = Victim->GetPlayerState<ACharacterState>();
	
	if (AttackerState == nullptr || VictimState == nullptr) return BaseDamage; // 플레이어 상태가 없으면 기본 피해 반환

	if (AttackerState == VictimState) return BaseDamage; // 자기가 자기한테 주는 거는 그냥 주기

	if (AttackerState->GetTeam() == VictimState->GetTeam()) return 0.f; // 팀킬 방지용. 공격자와 피해자가 같은 팀이면 피해 없음

	return BaseDamage; // 기본 피해 반환
}

// 플레이어가 제거되었을 때 호출되는 함수
void ATeamsGameMode::PlayerEliminated(ABaseCharacter* EliminatedCharacter, ABasePC* EliminatedController, ABasePC* AttackerController)
{
	Super::PlayerEliminated(EliminatedCharacter, EliminatedController, AttackerController);// 기본 게임 모드의 플레이어 제거 함수 호출

	ABaseGameState* BaseGameState = Cast<ABaseGameState>(UGameplayStatics::GetGameState(this));
	ACharacterState* AttackerPlayerState = AttackerController ? AttackerController->GetPlayerState<ACharacterState>(): nullptr;

	if (BaseGameState && AttackerPlayerState)
	{
		if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			BaseGameState->BlueTeamScoreAdd(); // 블루팀 점수 추가
		}
		else if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			BaseGameState->RedTeamScoreAdd(); // 레드팀 점수 추가
		}
	}
}
