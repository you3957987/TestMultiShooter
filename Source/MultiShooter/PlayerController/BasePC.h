#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BasePC.generated.h"

UCLASS()
class MULTISHOOTER_API ABasePC : public APlayerController
{
	GENERATED_BODY()

	class ACharacterHUD* CharacterHUD; // 캐릭터 HUD 클래스
	
protected:
	virtual void BeginPlay() override; // 플레이 시작 시 호출되는 함수


public:

	void SetHUDHealth( float Health, float MaxHealth ); // HUD의 체력을 설정하는 함수

};
