#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BaseGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNameChangedDelegate); // 이름 변경됬다고 방송

UCLASS()
class MULTISHOOTER_API UBaseGameInstance : public UGameInstance
{
	GENERATED_BODY()

	virtual void Init() override;
};
