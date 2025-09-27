#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "Menu.generated.h"

// 이름값을 브로드 캐스트 함
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerNameSet, const FString&, PlayerName);

UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")), 
		FString LobbyPath = FString(TEXT("/Game/ThirdPersonCPP/Maps/Lobby")));

	void SetJoinSessionName(FString SessionName, FString MatchType);

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UCheckBox* JoinFreeForAllCheckBox; // 매치 타입 체크박스

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UEditableTextBox* NameBox; // 이름 입력 박스

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* NameButton; // 이름 설정 버튼

	UPROPERTY(BlueprintAssignable)
	FOnPlayerNameSet OnPlayerNameSet;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UCheckBox* JoinTeamCheckBox; // 매치 타입 체크박스

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UCheckBox* JoinTeamFlagCheckBox; // 매치 타입 체크박스

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	TArray<USkeletalMesh*> SelectedSkeletalMesh;

protected:

	virtual bool Initialize() override;
	virtual void NativeDestruct() override;

	//
	// Callbacks for the custom delegates on the MultiplayerSessionsSubsystem
	//
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

	void OnFindServerList(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);


private:

	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton; // 세션 생성 버튼

	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton; // 세션 참여 버튼

	UPROPERTY(meta = (BindWidget))
	UButton* FindServerButton; // 세션 찾기 버튼

	UPROPERTY(meta = (BindWidget))
	class UScrollBox* ServerListScrollBox; // 서버 목록 스크롤 박스

	UPROPERTY(meta = (BindWidget)) // 에디터블 텍스트 블럭
	class UEditableTextBox* HostSessionNameTextBox; // 서버 이름 입력 박스

	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* NumPlayerTextBox; // 플레이어 수 입력 박스

	UPROPERTY(meta = (BindWidget)) // 에디터블 텍스트 블럭
	class UEditableTextBox* JoinSessionNameTextBox; // 클라이언트 세션 이름 박스

	// 서버 리스트 위젯 클래스 (블루프린트에서 할당 가능)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class UServerList> ServerListWidgetClass;


	UFUNCTION()
	void HostButtonClicked();

	UFUNCTION()
	void JoinButtonClicked();

	UFUNCTION()
	void FindServerButtonClicked();

	void MenuTearDown();

	UFUNCTION()
	void HandleNumPlayerTextBoxChanged(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void HandleHostSessionNameTextBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	UFUNCTION()
	void HandleJoinSessionNameTextBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	
	UFUNCTION(BlueprintCallable)
	void OnCLickedNameButton(); // 이름 설정 버튼 클릭해서 이름 바꾸는 함수

	UFUNCTION()
	void UpdatePlayerName(const FText& Text); // 이름 입력 박스에서 텍스트가 변경될 때 호출되는 함수


	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FString HostSessionName{ TEXT("DefaultSession") }; // 세션 이름(기본값: MySession)

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FString JoinSessionName{ TEXT("DefaultSession") }; // 세션 이름(기본값: MySession)

	// The subsystem designed to handle all online session functionality
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 NumPublicConnections{4}; // 최대 플레이어 수

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FString HostMatchType{TEXT("FreeForAll")};

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FString JoinMatchType{ TEXT("FreeForAll") };

	FString PathToLobby{TEXT("")};



	// ----------------------------------------------------------------------------------------설정 ㅜ

	void MenuSetFromSaveData(); // 세이브 데이터에서 설정 불러오기

	class UGameSave* SaveData;

	UPROPERTY(meta = (BindWidget))
	class UButton* ApplyButton; // 설정 적용 버튼
	UFUNCTION()
	void OnApplyButtonClicked();

	UPROPERTY(meta = (BindWidget))
	class USlider* MouseUpDownSlider;
	UFUNCTION()
	void OnMouseUpDownSliderChanged(float Value);
	
	UPROPERTY(meta = (BindWidget))
	class USlider* MouseLeftRightSlider;
	UFUNCTION()
	void OnMouseLeftRightSliderChanged(float Value);
	
	UPROPERTY(meta = (BindWidget))
	class USlider* AimMouseUpDownSlider;
	UFUNCTION()
	void OnAimMouseUpDownSliderChanged(float Value);
	
	UPROPERTY(meta = (BindWidget))
	class USlider* AimMouseLeftRightSlider;
	UFUNCTION()
	void OnAimMouseLeftRightSliderChanged(float Value);

	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* MouseUpDownText;UFUNCTION()
	void OnMouseUpDownTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	
	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* MouseLeftRightText;
	UFUNCTION()
	void OnMouseLeftRightTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	
	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* AimMouseUpDownText;
	UFUNCTION()
	void OnAimMouseUpDownTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	
	UPROPERTY(meta = (BindWidget))
	class UEditableTextBox* AimMouseLeftRightText;
	UFUNCTION()
	void OnAimMouseLeftRightTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);
	
	
	// ----------------------------------------------------------------------------------------설정 ㅗ

};
