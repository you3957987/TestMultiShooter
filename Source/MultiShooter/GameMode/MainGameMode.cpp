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

void AMainGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld()->GetTimeSeconds(); // 레벨 시작 시간 초기화
}

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
				World->ServerTravel("/Game/Maps/GameMap?listen"); // 서버 트래블을 통해 게임 맵으로 이동
			}
		}
	}
}

void AMainGameMode::OnMatchStateSet() // 매치 상태가 설정될 때 호출되는 함수
{
	Super::OnMatchStateSet();

	for( FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++ ) // 모든 플레이어 컨트롤러를 순회
	{
		ABasePC* PlayerController = Cast<ABasePC>( *It );
		if ( PlayerController )
		{
			PlayerController->OnMatchStateSet(MatchState); // 플레이어 컨트롤러에 매치 상태를 설정
		}
	}
}

void AMainGameMode::PlayerEliminated(ABaseCharacter* EliminatedCharacter, ABasePC* EliminatedController, ABasePC* AttackerController)
{
	ACharacterState* AttackerCharacterState = AttackerController ? Cast<ACharacterState>(AttackerController->PlayerState) : nullptr; // 공격자 플레이어 상태 캐스팅
    ACharacterState* EliminatedCharacterState = EliminatedController ? Cast<ACharacterState>(EliminatedController->PlayerState) : nullptr; // 제거된 플레이어 상태 캐스팅
	ABaseGameState* LocalGameState =  GetGameState<ABaseGameState>(); // 게임 상태를 가져오기

	if (AttackerCharacterState && AttackerCharacterState != EliminatedCharacterState && LocalGameState ) // 공격자와 제거된 플레이어가 다를 때
	{
		AttackerCharacterState->AddToScore(1.0f); // 공격자에게 점수 추가
		LocalGameState->UpdateTopScore(AttackerCharacterState); // 최고 점수를 기록한 플레이어 업데이트
	}

	if( EliminatedCharacterState ) // 제거된 플레이어와 공격자가 다를 때
	{
		EliminatedCharacterState->AddToDefeat(1); // 제거된 플레이어의 죽은 횟수 증가
		EliminatedCharacterState->UpdateDeathMessage(AttackerController ? AttackerController->PlayerState->GetPlayerName() : FString("Unknown")); // 죽은 메시지 업데이트
	}

	if( EliminatedCharacter )
    {
        EliminatedCharacter->Eliminate(); // 플레이어 제거 함수 호출
    }

}

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
 
		// only really need to do this once, as the spawn points do not move (in this game) - but relies on knowing when the match has actually started (e.g. cant use BeginPlay)
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		UGameplayStatics::GetAllActorsOfClass(this, ABaseCharacter::StaticClass(), Players);
		AActor* PlayerStart = GetSpawnPointWithLargestMinimumDistance(PlayerStarts, Players);
 
		if (PlayerStart)
		{
			RestartPlayerAtPlayerStart(EliminatedController, PlayerStart);			
		}
		else
		{
			// no respawn for you!  (needs error handling)
		}
	}
}

AActor* AMainGameMode::GetSpawnPointWithLargestMinimumDistance(TArray<AActor*> PlayerStarts, TArray<AActor*> Players) const
{
	if (PlayerStarts.Num() == 0 || Players.Num() == 0) { return nullptr; }
 
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