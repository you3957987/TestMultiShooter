#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BaseGameInstance.generated.h"


UCLASS()
class MULTISHOOTER_API UBaseGameInstance : public UGameInstance
{
	GENERATED_BODY()


public:
	UPROPERTY(BlueprintReadWrite)
	FString SelectedSkeletalMesh = "BasicMale_SK";

};
