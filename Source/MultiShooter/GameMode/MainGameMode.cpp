#include "MainGameMode.h"
#include "MultiShooter/Character/BaseCharacter.h"
#include "MultiShooter/PlayerController/BasePC.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h" // 플레이어 시작 위치를 가져오기 위한 헤더
#include "MultiShooter/CharacterState/CharacterState.h"
#include "MultiShooter/GameState/BaseGameState.h" // 게임 상태를 가져오기 위한 헤더
#include "MultiShooter/CharacterComponent/CombatComponent.h" // 전투 컴포넌트를 가져오기 위한 헤더

namespace MatchState
{
	const FName Cooldown = FName("Cooldown"); // 매치 상태: 쿨다운 == 엔드 게임 이후 누가 승자인지 확인하는 시간
}

AMainGameMode::AMainGameMode()
{
	bDelayedStart = true; // 게임 모드가 시작될 때 지연 시작을 활성화합니다. -> 자유 시점으로 플레이어가 시작할 수 있도록 설정
}

// LevelStartingTime 변수를 초기화하고 레벨 시작 시간을 저장합니다.
void AMainGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds(); // 레벨 시작 시간 초기화
}

// 매치 상태에 따라 카운트 다운 시간 재고 매치 상태 관리
void AMainGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if ( MatchState == MatchState::WaitingToStart )
	{
		CountDownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime; // 현재 시간에서 레벨 시작 시간을 빼서 카운트다운 시간 계산
		if ( CountDownTime <= 0.0f ) // 카운트다운 시간이 0 이하가 되면
		{
			StartMatch(); // 매치 시작 -> 매치 상태 InProgress로 변경
		}
	}
	else if( MatchState == MatchState::InProgress )
	{
		CountDownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime; // 현재 시간에서 레벨 시작 시간을 빼서 카운트다운 시간 계산
		if ( CountDownTime <= 0.0f ) // 카운트다운 시간이 0 이하가 되면
		{
			SetMatchState(MatchState::Cooldown); // 매치 상태를 쿨다운으로 변경 == 엔드 게임 이후 누가 승자인지 확인하는 시간
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountDownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if( CountDownTime < 0.f )
		{
			for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				ABasePC* PC = Cast<ABasePC>(*It);
				if (PC)
				{
					ABaseCharacter* Char = Cast<ABaseCharacter>(PC->GetCharacter());
					if (Char && Char->GetCombatComponent())
					{
						Char->GetCombatComponent()->ClearEquippedWeapon();
					}
				}
			}

			UWorld* World = GetWorld();
			if (World)
			{
				bUseSeamlessTravel = true; // 필요시
				FString MapName = UGameplayStatics::GetCurrentLevelName(World, true); // 현재 맵 이름만 추출
				FString TravelURL = FString::Printf(TEXT("/Game/Maps/%s?listen"), *MapName);
				World->ServerTravel(TravelURL);
			}
		}
	}
}

// 게임 모드에서 매치 스테이트가 바뀔 떄 호출. 모든 플레이어 컨트롤러를 순회하며 OnMatchStateSet 함수를 호출하여 매치 상태를 설정
void AMainGameMode::OnMatchStateSet() // 매치 상태가 설정될 때 호출되는 함수
{
	Super::OnMatchStateSet();

	for( FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++ ) // 모든 플레이어 컨트롤러를 순회
	{
		ABasePC* PlayerController = Cast<ABasePC>( *It );
		if ( PlayerController )
		{
			PlayerController->OnMatchStateSet(MatchState, bTeamMatch); // 플레이어 컨트롤러에 매치 상태를 설정
		}
	}
}

// 팀킬 방지용. 이거는 팀 게임모드에서 재정의
float AMainGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	return BaseDamage;
}

// 플레이가 죽으면 캐릭터 스테이트 점수 및 죽은 횟수 업데이트 및 최고 점수 기록한 플레이어 업데이트
void AMainGameMode::PlayerEliminated(ABaseCharacter* EliminatedCharacter, ABasePC* EliminatedController, ABasePC* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return; 
	if (EliminatedController == nullptr || EliminatedController->PlayerState == nullptr) return; 

	ACharacterState* AttackerCharacterState = AttackerController ? Cast<ACharacterState>(AttackerController->PlayerState) : nullptr; 
    ACharacterState* EliminatedCharacterState = EliminatedController ? Cast<ACharacterState>(EliminatedController->PlayerState) : nullptr; 
	ABaseGameState* LocalGameState =  GetGameState<ABaseGameState>(); // 게임 상태를 가져오기

	if (AttackerCharacterState && AttackerCharacterState != EliminatedCharacterState && LocalGameState ) // 공격자와 제거된 플레이어가 다를 때
	{
		TArray<ACharacterState*> PlayersCurerrentlyInTheLead;
		for (auto LeadPlayer : LocalGameState->TopScoreingPlayers) // 현재 최고 점수를 기록한 플레이어들을 순회
		{
			PlayersCurerrentlyInTheLead.Add(LeadPlayer); // 최고 점수를 기록한 플레이어를 배열에 추가
		}

		AttackerCharacterState->AddToScore(1.0f); // 공격자에게 점수 추가
		LocalGameState->UpdateTopScore(AttackerCharacterState); // 최고 점수를 기록한 플레이어 업데이트
		if (LocalGameState->TopScoreingPlayers.Contains(AttackerCharacterState))
		{
			ABaseCharacter* Leader = Cast<ABaseCharacter>(AttackerCharacterState->GetPawn()); // 최고 점수를 기록한 플레이어의 캐릭터를 가져오기
			if (Leader)
			{
				Leader->MulticastGainedTheLead(); // 최고 점수를 기록한 플레이어에게 알림
			}
		}

		for (int32 i = 0; i < PlayersCurerrentlyInTheLead.Num(); i++)
		{
			if (LocalGameState->TopScoreingPlayers.Contains(PlayersCurerrentlyInTheLead[i]) == false)
			{
				ABaseCharacter* Loser = Cast<ABaseCharacter>(PlayersCurerrentlyInTheLead[i]->GetPawn()); // 최고 점수를 기록한 플레이어의 캐릭터를 가져오기
				if (Loser)
				{
					Loser->MulticastLostTheLead(); // 최고 점수를 잃은 플레이어에게 알림
				}
			}
		}
	}

	if( EliminatedCharacterState ) // 제거된 플레이어와 공격자가 다를 때
	{
		EliminatedCharacterState->AddToDefeat(1); // 제거된 플레이어의 죽은 횟수 증가
		EliminatedCharacterState->UpdateDeathMessage(AttackerController ? AttackerController->PlayerState->GetPlayerName() : FString("Unknown")); 
		// 죽은 메시지 업데이트
	}

	if( EliminatedCharacter )
    {
        EliminatedCharacter->Eliminate(false); // 플레이어 제거 함수 호출
    }

	for( FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++ ) // 모든 플레이어 컨트롤러를 순회
	{
		ABasePC* PlayerController = Cast<ABasePC>(*It);
		if (PlayerController && AttackerCharacterState && EliminatedCharacterState)
		{
			PlayerController->BroadcastElim(AttackerCharacterState, EliminatedCharacterState); 
			// 플레이어 컨트롤러에 제거된 플레이어와 공격자 정보를 브로드캐스트
		}
	}
}

// 플레이어가 제거되었을 때 리스폰 요청하는 함수. 여기서 캐릭터간 부활 거리 계산 후 부활 시킴
void AMainGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Reset();
		EliminatedCharacter->Destroy();
	}
 
	if (EliminatedController)
	{
		TArray<AActor*> PlayerStarts;
		TArray<AActor*> Players;
 
		// 이 게임에서는 스폰 포인트(플레이어 시작 위치)가 움직이지 않으므로 사실상 한 번만 이 작업을 해주면 되긴 함
		// 하지만 실제로 매치가 시작된 시점을 알아야 하므로그 타이밍을 잘 맞춰야 함
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts); // 플레이어 시작 위치를 가져오기
		UGameplayStatics::GetAllActorsOfClass(this, ABaseCharacter::StaticClass(), Players); // 현재 게임에 있는 모든 플레이어 캐릭터를 가져오기
		AActor* PlayerStart = GetSpawnPointWithLargestMinimumDistance(PlayerStarts, Players);
 
		if (PlayerStart)
		{
			RestartPlayerAtPlayerStart(EliminatedController, PlayerStart);			
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Visang!!!!!!!!!!!!!"));
			// 비상 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		}
	}
}


// ------------------------------------------------------------------------------------플레이어가 게임을 떠났을 때 호출되는 함수
//
void AMainGameMode::PlayerLeftGame(ACharacterState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return; // 플레이어가 nullptr인 경우 함수 종료

	ABaseGameState* BasePlayerState = Cast<ABaseGameState>(UGameplayStatics::GetGameState(this));

	if (BasePlayerState && BasePlayerState->TopScoreingPlayers.Contains(PlayerLeaving))
	{
		BasePlayerState->TopScoreingPlayers.Remove(PlayerLeaving); // 게임 상태에서 플레이어 제거
	}
	ABaseCharacter* CharacterLeaving = Cast<ABaseCharacter>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Eliminate(true); // 플레이어 제거 함수 호출
	}
}

// 이거는 플레이어가 강종으로 나갈 때 호출되는 함수. 원래 게임 모드에 존재하는 함수 오버라이드
void AMainGameMode::Logout(AController* Exiting)
{
	if (Exiting == nullptr) { return; }

	ABaseGameState* BaseGameState = GetGameState<ABaseGameState>();
	ACharacterState* BasePlayerState = Cast<ACharacterState>(Exiting->PlayerState);
	if (BaseGameState && BaseGameState->TopScoreingPlayers.Contains(BasePlayerState))
	{
		UE_LOG(LogTemp, Warning, TEXT("Logout"));
		BaseGameState->TopScoreingPlayers.Remove(BasePlayerState);
	}

	Super::Logout(Exiting);
}
//
// // ------------------------------------------------------------------------------------플레이어가 게임을 떠났을 때 호출되는 함수

// 플레이어가 채팅 쓰면 다른 플레이어 컨틀롤러 전부 순회 후 ClientChatCommitted 함수를 호출하여 채팅 메시지를 전송
void AMainGameMode::SendChatMsg(const FText& Text, const FString& PlayerName)
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABasePC* PC = Cast<ABasePC>(*It);
		if (PC)
		{
			PC->ClientChatCommitted(Text, PlayerName);
		}
	}
}

// 아래 두줄은 캐릭터 부활시 캐릭터간 거리 가장 먼 곳에서 부활하게 해주는 함수 2개
AActor* AMainGameMode::GetSpawnPointWithLargestMinimumDistance(TArray<AActor*> PlayerStarts, TArray<AActor*> Players) const
{
	if (PlayerStarts.Num() == 0) { return nullptr; }

	if (Players.Num() == 0)
	{
		// 플레이어가 없으면 랜덤으로 하나 선택
		int32 RandIdx = FMath::RandRange(0, PlayerStarts.Num() - 1);
		return PlayerStarts[RandIdx];
	}

	float LargestMinimumDistance = -1 * std::numeric_limits<float>::infinity();
	AActor* Result = nullptr;

	for (AActor* PlayerStart : PlayerStarts)
	{
		const float MinimumDistance = GetMinimumDistance(PlayerStart, Players);

		if (MinimumDistance > LargestMinimumDistance)
		{
			LargestMinimumDistance = MinimumDistance;
			Result = PlayerStart;
		}
	}

	return Result;
}

float AMainGameMode::GetMinimumDistance(const AActor* PlayerStart, const TArray<AActor*> Players) const
{
	float MinimumDistanceSquared = std::numeric_limits<float>::infinity();

	for (AActor* Player : Players)
	{
		const float DistanceSquared = FVector::DistSquared(PlayerStart->GetActorLocation(), Player->GetActorLocation());
		MinimumDistanceSquared = FMath::Min(MinimumDistanceSquared, DistanceSquared);
	}

	return FMath::Sqrt(MinimumDistanceSquared);
}
//------------------------------------------------------------------------------------------------------------------
