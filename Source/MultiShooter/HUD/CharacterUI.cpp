#include "CharacterUI.h"
#include "Components/ProgressBar.h"



void UCharacterUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (CurrentRollCooldown > 0.0f)
	{
		CurrentRollCooldown -= InDeltaTime;
		if (CurrentRollCooldown < 0.0f)
		{
			CurrentRollCooldown = 0.0f;
		}
		if (RollBar)
		{
			RollBar->SetPercent(CurrentRollCooldown / RollCooldownTime);
		}
	}

	if (CurrentBackDashCooldown > 0.0f)
	{
		CurrentBackDashCooldown -= InDeltaTime;
		if (CurrentBackDashCooldown < 0.0f)
		{
			CurrentBackDashCooldown = 0.0f;
		}
		if (BackDashBar)
		{
			BackDashBar->SetPercent(CurrentBackDashCooldown / BackDashCooldownTime);
		}
	}
}

void UCharacterUI::StartRollCooldown(float CooldownTime)
{
	RollCooldownTime = CooldownTime;
	CurrentRollCooldown = RollCooldownTime;
	if (RollBar)
	{
		RollBar->SetPercent(1.0f);
	}
}

void UCharacterUI::StartBackDashCooldown(float CooldownTime)
{
	BackDashCooldownTime = CooldownTime;
	CurrentBackDashCooldown = BackDashCooldownTime;
	if (BackDashBar)
	{
		BackDashBar->SetPercent(1.0f);
	}
}
