#include "BaseGameState.h"
#include "Net/UnrealNetwork.h" // 네트워크 복제를 위한 헤더 파일
#include "MultiShooter/CharacterState/CharacterState.h" // 플레이어 상태 클래스 헤더 파일
#include "MultiShooter/PlayerController/BasePC.h" // 플레이어 컨트롤러 클래스 헤더 파일

void ABaseGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // 최고 점수를 기록한 플레이어들의 배열을 네트워크에서 복제합니다.
    DOREPLIFETIME(ABaseGameState, TopScoreingPlayers);
    DOREPLIFETIME(ABaseGameState, RedTeamScore);
    DOREPLIFETIME(ABaseGameState, BlueTeamScore);
   
}

void ABaseGameState::UpdateTopScore(class ACharacterState* ScoringPlayer)
{
    if( TopScoreingPlayers.Num() == 0 )
    {
        TopScoreingPlayers.Add(ScoringPlayer); // 최고 점수를 기록한 플레이어가 없으면 추가합니다.
        TopScore = ScoringPlayer->GetScore(); // 해당 플레이어의 점수를 최고 점수로 설정합니다.
    }
    else if( ScoringPlayer->GetScore() > TopScore )
    {
        TopScoreingPlayers.Empty(); // 최고 점수를 기록한 플레이어들을 초기화합니다.
        TopScoreingPlayers.AddUnique(ScoringPlayer); // 새로운 최고 점수를 기록한 플레이어를 추가합니다.
        TopScore = ScoringPlayer->GetScore(); // 해당 플레이어의 점수를 최고 점수로 설정합니다.
    }
    else if( ScoringPlayer->GetScore() == TopScore )
    {
        TopScoreingPlayers.AddUnique(ScoringPlayer); // 동일한 최고 점수를 기록한 플레이어를 추가합니다.
    }
}

//---------------------------------------------------------------------------------------------------------------------팀 게임 관련 ㅜ

void ABaseGameState::RedTeamScoreAdd()
{
    RedTeamScore++;

    ABasePC* BasePlayerController = Cast<ABasePC>(GetWorld()->GetFirstPlayerController()); // 로컬 플레이어 컨트롤러를 가져옵니다.
    if (BasePlayerController)
    {
        BasePlayerController->SetHUDRedTeamScore(RedTeamScore); // 로컬 플레이어 컨트롤러의 HUD에 레드 팀 점수를 설정합니다.
    }
}

void ABaseGameState::BlueTeamScoreAdd()
{
    BlueTeamScore++;

    ABasePC* BasePlayerController = Cast<ABasePC>(GetWorld()->GetFirstPlayerController()); // 로컬 플레이어 컨트롤러를 가져옵니다.
    if (BasePlayerController)
    {
        BasePlayerController->SetHUDBlueTeamScore(BlueTeamScore); // 로컬 플레이어 컨트롤러의 HUD에 레드 팀 점수를 설정합니다.
    }
}

void ABaseGameState::OnRep_RedTeamScore()
{
	ABasePC* BasePlayerController = Cast<ABasePC>(GetWorld()->GetFirstPlayerController()); // 로컬 플레이어 컨트롤러를 가져옵니다.
    if (BasePlayerController)
    {
		BasePlayerController->SetHUDRedTeamScore(RedTeamScore); // 로컬 플레이어 컨트롤러의 HUD에 레드 팀 점수를 설정합니다.
    }
}

void ABaseGameState::OnRep_BlueTeamScore()
{
    ABasePC* BasePlayerController = Cast<ABasePC>(GetWorld()->GetFirstPlayerController()); // 로컬 플레이어 컨트롤러를 가져옵니다.
    if (BasePlayerController)
    {
        BasePlayerController->SetHUDBlueTeamScore(BlueTeamScore); // 로컬 플레이어 컨트롤러의 HUD에 레드 팀 점수를 설정합니다.
    }
}

//---------------------------------------------------------------------------------------------------------------------팀 게임 관련 ㅗ