#include "TeamPlayerStart.h"

void ATeamPlayerStart::StartSpawnCooldown()
{
	bSpawned = true;
	GetWorldTimerManager().SetTimer(
		SpawnCooldownTimerHandle,
		this,
		&ATeamPlayerStart::ResetSpawnCooldown,
		3.0f, // 3ÃÊ Äð´Ù¿î
		false
	);
}

void ATeamPlayerStart::ResetSpawnCooldown()
{
	bSpawned = false;
}

