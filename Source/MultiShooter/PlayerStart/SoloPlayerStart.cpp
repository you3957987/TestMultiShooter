#include "SoloPlayerStart.h"

void ASoloPlayerStart::StartSpawnCooldown()
{
	bSpawned = true;
	GetWorldTimerManager().SetTimer(
		SpawnCooldownTimerHandle,
		this,
		&ASoloPlayerStart::ResetSpawnCooldown,
		3.0f, // 3ÃÊ Äð´Ù¿î
		false
	);
}

void ASoloPlayerStart::ResetSpawnCooldown()
{
	bSpawned = false;
}

