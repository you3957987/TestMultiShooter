#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyController.generated.h"


UCLASS()
class MULTISHOOTER_API ALobbyController : public APlayerController
{
	GENERATED_BODY()
	

protected:
	virtual void BeginPlay() override; // 플레이어 컨트롤러가 시작될 때 호출되는 함수
	virtual void SetupInputComponent() override; // 입력 컴포넌트를 설정하는 함수

public:
	bool bIsChatting = false; // 채팅 중인지 여부

	void OnMouseClickAnywhere(); // 채팅 중 마우스 클릭 시 채팅 종료

	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UInputAction* IA_EnterToChat; // 채팅 입력 액션

	void StartChat();

	void AddChatBox(); // 채팅 박스 위젯을 추가하는 함수

	UPROPERTY()
	class UChatBox* ChatBox;

	UPROPERTY(EditAnywhere, Category = "자체설정")
	TSubclassOf<UChatBox> ChatBoxClass;

	UFUNCTION()
	void OnChatCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION(Server, Reliable)
	void ServerChatCommitted(const FText& Text, const FString& PlayerName);

	UFUNCTION(Client, Reliable)
	void ClientChatCommitted(const FText& Text, const FString& PlayerName);
};
