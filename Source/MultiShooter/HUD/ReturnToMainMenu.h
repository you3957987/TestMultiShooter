#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMainMenu.generated.h"

UCLASS()
class MULTISHOOTER_API UReturnToMainMenu : public UUserWidget
{
	GENERATED_BODY()

	UFUNCTION()
	void ReturnButtonClicked(); // 메인 메뉴로 돌아가는 버튼 클릭 이벤트 핸들러
	
	UPROPERTY(meta = (BindWidget))
	class UButton* ReturnButton; // 메인 메뉴로 돌아가는 버튼

	UPROPERTY()
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem; // 멀티플레이어 세션 서브시스템

	UPROPERTY()
	class APlayerController* PlayerController; // 플레이어 컨트롤러

protected:
	virtual bool Initialize() override; // 위젯 초기화 함수

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful); // 세션 파괴 후 호출되는 함수

	UFUNCTION()
	void OnPlayerLeftGame(); // 플레이어가 게임을 떠났을 때 호출되는 함수

public:
	void MenuSetUp(); // 메뉴 설정 함수
	void MenuTeardown(); // 메뉴 설정 및 해제 함수
};
