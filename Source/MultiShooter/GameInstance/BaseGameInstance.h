#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BaseGameInstance.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnNameChangedDelegate); // 이름 변경됬다고 방송

UCLASS()
class MULTISHOOTER_API UBaseGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:

	virtual void Init() override; // 게임 인스턴스가 초기화될 때 호출되는 함수
	
	UPROPERTY(BlueprintReadWrite)
	FString SelectedSkeletalMesh = "BasicMale_SK";

	UFUNCTION()
	void SetPlayerName(const FString& Name); // 플레이어 이름 설정 함수

	UFUNCTION(BlueprintCallable)
	void ShowMenu(); // 메뉴 표시 함수

	UPROPERTY()
	FString PlayerName = "VICE"; // 기본 이름 설정

	UPROPERTY()
	FOnNameChangedDelegate OnNameChanged;
	
private:

	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	TSubclassOf<class UMenu> MenuClass;

	UPROPERTY()
	class UMenu* MenuWidget;
	
};
