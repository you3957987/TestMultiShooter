#include "MainGameMode.h"
#include "MultiShooter/Character/BaseCharacter.h"
#include "MultiShooter/PlayerController/BasePC.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h" // 플레이어 시작 위치를 가져오기 위한 헤더

void AMainGameMode::PlayerEliminated(ABaseCharacter* EliminatedCharacter, ABasePC* EliminatedController, ABasePC* AttackerController)
{
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