#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/CheckBox.h"
#include "Kismet/GameplayStatics.h"
#include "Components/Slider.h"

#include "GameSave.h"
#include "ServerList.h"

// 초기화. 비긴 플레이같은거 -> 여가서 버튼 클릭이랑 매핑
bool UMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}
	if (NumPlayerTextBox)
	{
		NumPlayerTextBox->OnTextCommitted.AddDynamic(this, &ThisClass::HandleNumPlayerTextBoxChanged);
	}
	if (HostSessionNameTextBox)
	{
		HostSessionNameTextBox->OnTextCommitted.AddDynamic(this, &ThisClass::HandleHostSessionNameTextBoxCommitted);
	}
	if (JoinSessionNameTextBox)
	{
		JoinSessionNameTextBox->OnTextCommitted.AddDynamic(this, &ThisClass::HandleJoinSessionNameTextBoxCommitted);
	}
	if (FindServerButton)
	{
		FindServerButton->OnClicked.AddDynamic(this, &ThisClass::FindServerButtonClicked);
	}
	if ( NameButton )
	{
		NameButton->OnClicked.AddDynamic(this, &ThisClass::OnCLickedNameButton);
	}
	if ( NameBox )
	{
		NameBox->OnTextChanged.AddDynamic(this, &ThisClass::UpdatePlayerName);
	}

	if ( ApplyButton )
	{
		ApplyButton->OnClicked.AddDynamic(this, &ThisClass::OnApplyButtonClicked); // 이름 설정 버튼과 동일한 함수 사용
	}
	
	if (MouseUpDownSlider)
	{
		MouseUpDownSlider->OnValueChanged.AddDynamic(this, &UMenu::OnMouseUpDownSliderChanged);
	}
	if (MouseUpDownText)
	{
		MouseUpDownText->OnTextCommitted.AddDynamic(this, &UMenu::OnMouseUpDownTextCommitted);
	}

	if (MouseLeftRightSlider)
	{
		MouseLeftRightSlider->OnValueChanged.AddDynamic(this, &UMenu::OnMouseLeftRightSliderChanged);
	}
	if (MouseLeftRightText)
	{
		MouseLeftRightText->OnTextCommitted.AddDynamic(this, &UMenu::OnMouseLeftRightTextCommitted);
	}

	if (AimMouseUpDownSlider)
	{
		AimMouseUpDownSlider->OnValueChanged.AddDynamic(this, &UMenu::OnAimMouseUpDownSliderChanged);
	}
	if (AimMouseUpDownText)
	{
		AimMouseUpDownText->OnTextCommitted.AddDynamic(this, &UMenu::OnAimMouseUpDownTextCommitted);
	}

	if (AimMouseLeftRightSlider)
	{
		AimMouseLeftRightSlider->OnValueChanged.AddDynamic(this, &UMenu::OnAimMouseLeftRightSliderChanged);
	}
	if (AimMouseLeftRightText)
	{
		AimMouseLeftRightText->OnTextCommitted.AddDynamic(this, &UMenu::OnAimMouseLeftRightTextCommitted);
	}

	return true;
}

void UMenu::NativeDestruct()
{
	MenuTearDown();
	Super::NativeDestruct();
}

// 게임 스타트 맵의 레벨 블루프린트에서 호출함!
void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	NumPublicConnections = NumberOfPublicConnections;
	HostMatchType = TypeOfMatch;
	JoinMatchType = TypeOfMatch;
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindServerListComplete.AddUObject(this, &UMenu::OnFindServerList);
	}

	MenuSetFromSaveData();// 세이브 데이터에서 설정 불러오기
}

void UMenu::SetJoinSessionName(FString SessionName, FString MatchType)
{
	if (JoinSessionNameTextBox)
	{
		JoinSessionNameTextBox->SetText(FText::FromString(SessionName));
		JoinSessionName = SessionName; // 세션 이름 저장
	}
	JoinMatchType = MatchType; // 매치 타입 저장

	if (MatchType == "FreeForAll")
	{
		if (JoinFreeForAllCheckBox)
		{
			JoinFreeForAllCheckBox->SetIsChecked(true); // UI 상태 변경
			JoinFreeForAllCheckBox->OnCheckStateChanged.Broadcast(true);
		}
	}
	else if (MatchType == "Team")
	{
		if (JoinTeamCheckBox)
		{
			JoinTeamCheckBox->SetIsChecked(true); // UI 상태 변경
			JoinTeamCheckBox->OnCheckStateChanged.Broadcast(true);
		}
	}
	else if (MatchType == "TeamFlag")
	{
		if (JoinTeamFlagCheckBox)
		{
			JoinTeamFlagCheckBox->SetIsChecked(true); // UI 상태 변경
			JoinTeamFlagCheckBox->OnCheckStateChanged.Broadcast(true);
		}
	}
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				10.f,
				FColor::Yellow,
				FString::Printf(TEXT("FindSessionName: %s, FindMatchType: %s"), *HostSessionName, *HostMatchType)
			);
		}
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(PathToLobby);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("Failed to create session!"))
			);
		}
		HostButton->SetIsEnabled(true);
	}
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (MultiplayerSessionsSubsystem == nullptr)
	{
		return;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			10.f,
			FColor::Yellow,
			FString::Printf(TEXT("MySessionName: %s, MyMatchType: %s"), *JoinSessionName, *JoinMatchType)
		);
	}

	for (auto Result : SessionResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		FString SessionName;
		Result.Session.SessionSettings.Get(FName("SessionName"), SessionName);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				10.f,
				FColor::Yellow,
				FString::Printf(TEXT("FindSessionName: %s, FindMatchType: %s"), *SessionName, *SettingsValue)
			);
		}

		if (SettingsValue == JoinMatchType && SessionName == JoinSessionName) // 클라쪽에서 매치 타입 확인
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				10.f,
				FColor::Yellow,
				FString::Printf(TEXT("Equal Session Name Adn MatchType"))
			);
			MultiplayerSessionsSubsystem->JoinSession(Result, *JoinSessionName );
			return;
		}
	}
	
	JoinButton->SetIsEnabled(true);
	
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(FName(*JoinSessionName), Address);

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}

void UMenu::OnFindServerList(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	FindServerButton->SetIsEnabled(true); // 1단 버튼 활성화
	if (MultiplayerSessionsSubsystem == nullptr)
	{
		return;
	}

	if (ServerListWidgetClass && ServerListScrollBox)
	{
		ServerListScrollBox->ClearChildren();

		for (const auto& Result : SessionResults)
		{
			UServerList* ServerListWidget = CreateWidget<UServerList>(GetWorld(), ServerListWidgetClass);
			if (ServerListWidget)
			{
				FString SessionName;
				Result.Session.SessionSettings.Get(FName("SessionName"), SessionName);
				FString MatchType;
				Result.Session.SessionSettings.Get(FName("MatchType"), MatchType);
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(
						-1,
						3.f,
						FColor::Yellow,
						FString::Printf(TEXT("SessionName: %s MatchType : %s"), *SessionName, *MatchType)
					);
				}
				int32 NumPlayers = Result.Session.NumOpenPrivateConnections;
				int32 MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;

				ServerListWidget->SetServerListInfo(SessionName, MatchType, NumPlayers, MaxPlayers);
				ServerListWidget->SetMenuWidget(this); // 메뉴 위젯 설정
				ServerListScrollBox->AddChild(ServerListWidget);
			}
		}
	}
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}

void UMenu::HostButtonClicked()
{
	HostButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, HostMatchType, HostSessionName);
	}
}

void UMenu::JoinButtonClicked()
{
	JoinButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSessions(10000);
	}
}

void UMenu::FindServerButtonClicked()
{
	FindServerButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindAllSessions(1000);
	}
}

// 메뉴가 제거될 때 호출되는 함수
void UMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}

void UMenu::HandleNumPlayerTextBoxChanged(const FText& Text, ETextCommit::Type CommitMethod)
{
	FString InputString = Text.ToString();
	FString FilteredString;

	// 숫자만 추출
	for (int32 i = 0; i < InputString.Len(); ++i)
	{
		if (FChar::IsDigit(InputString[i]))
		{
			FilteredString.AppendChar(InputString[i]);
		}
	}

	// 값이 없으면 1로 설정
	int32 Value = 1;
	if (!FilteredString.IsEmpty())
	{
		Value = FCString::Atoi(*FilteredString);
	}

	// 1~8 사이로 제한
	Value = FMath::Clamp(Value, 1, 8);

	NumPublicConnections = Value;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1, // key: -1이면 항상 새로운 메시지
			3.f, // duration: 3초 동안 표시
			FColor::Green,
			FString::Printf(TEXT("NumPublicConnections: %d"), NumPublicConnections)
		);
	}
	// 텍스트 박스에 제한된 값으로 다시 표시
	if (NumPlayerTextBox)
	{
		FString ClampedString = FString::Printf(TEXT("%d"), Value);
		if (InputString != ClampedString)
		{
			NumPlayerTextBox->SetText(FText::FromString(ClampedString));
		}
	}
}

void UMenu::HandleHostSessionNameTextBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	FString InputString = Text.ToString();
	FString FilteredString;

	auto IsHangul = [](TCHAR c)
		{
			return (c >= 0xAC00 && c <= 0xD7A3);
		};

	for (int32 i = 0; i < InputString.Len() && FilteredString.Len() < 12; ++i)
	{
		TCHAR c = InputString[i];
		if (FChar::IsAlnum(c) || IsHangul(c))
		{
			FilteredString.AppendChar(c);
		}
	}

	if (HostSessionNameTextBox)
	{
		HostSessionNameTextBox->SetText(FText::FromString(FilteredString));
	}

	// 변수에 그대로 저장
	HostSessionName = FilteredString;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.f,
			FColor::Yellow,
			FString::Printf(TEXT("HostSessionName: %s"), *HostSessionName)
		);
	}
}

void UMenu::HandleJoinSessionNameTextBoxCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	FString InputString = Text.ToString();
	FString FilteredString;

	auto IsHangul = [](TCHAR c)
		{
			return (c >= 0xAC00 && c <= 0xD7A3);
		};

	for (int32 i = 0; i < InputString.Len() && FilteredString.Len() < 12; ++i)
	{
		TCHAR c = InputString[i];
		if (FChar::IsAlnum(c) || IsHangul(c))
		{
			FilteredString.AppendChar(c);
		}
	}

	if (JoinSessionNameTextBox)
	{
		JoinSessionNameTextBox->SetText(FText::FromString(FilteredString));
	}

	// 변수에 그대로 저장
	JoinSessionName = FilteredString;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.f,
			FColor::Yellow,
			FString::Printf(TEXT("JoinSessionName: %s"), *JoinSessionName)
		);
	}
}


// --------------------------------------------------------------------------------------------세이브 데이텨 관련 ㅜ

void UMenu::MenuSetFromSaveData()
{
	SaveData = Cast<UGameSave>(UGameplayStatics::LoadGameFromSlot(SaveSlot::SettingSlot, 0));

	if ( SaveData )
	{
		if ( MouseUpDownSlider )
		{
			MouseUpDownSlider->SetValue( SaveData->MouseUpDown ); 
		}
		if ( MouseLeftRightSlider )
		{
			MouseLeftRightSlider->SetValue( SaveData->MouseLeftRight );
		}
		if ( AimMouseUpDownSlider )
		{
			AimMouseUpDownSlider->SetValue( SaveData->AimMouseUpDown );
		}
		if ( AimMouseLeftRightSlider )
		{
			AimMouseLeftRightSlider->SetValue( SaveData->AimMouseLeftRight );
		}
		
		if ( MouseUpDownText )
		{
			FString ValueString = FString::Printf(TEXT("%.1f"), SaveData->MouseUpDown);
			MouseUpDownText->SetText(FText::FromString(ValueString));
		}
		if ( MouseLeftRightText )
		{
			FString ValueString = FString::Printf(TEXT("%.1f"), SaveData->MouseLeftRight);
			MouseLeftRightText->SetText(FText::FromString(ValueString));
		}
		if ( AimMouseUpDownText )
		{
			FString ValueString = FString::Printf(TEXT("%.1f"), SaveData->AimMouseUpDown);
			AimMouseUpDownText->SetText(FText::FromString(ValueString));
		}
		if ( AimMouseLeftRightText )
		{
			FString ValueString = FString::Printf(TEXT("%.1f"), SaveData->AimMouseLeftRight);
			AimMouseLeftRightText->SetText(FText::FromString(ValueString));
		}
	}
}

void UMenu::OnCLickedNameButton()
{
	// NameBox에서 값 가져와서 게임 인스턴스에 저장
	if (NameBox)
	{
		FString PlayerName = NameBox->GetText().ToString();

		// 앞 뒤 공백 제거
		PlayerName = PlayerName.TrimStartAndEnd();

		// 이름 입력 안하고 버튼 누르면 리턴
		if (PlayerName.IsEmpty())
		{
			UE_LOG(LogTemp, Error, TEXT("PlayerName is empty!"));
			return;
		}
		OnPlayerNameSet.Broadcast(PlayerName);
	}
}

// 이름 텍스트 박스 값 변경시
void UMenu::UpdatePlayerName(const FText& Text)
{
	// 이름 길이가 8자 넘으면 자르기
	FString InputString = Text.ToString();
	if (InputString.Len() > 8)
	{
		InputString = InputString.Left(8);
		if (NameBox)
		{
			NameBox->SetText(FText::FromString(InputString));
		}
	}
}

// 설정 적용 버튼 클릭시
void UMenu::OnApplyButtonClicked()
{
	if ( SaveData )
	{
		if ( MouseUpDownSlider )
		{
			SaveData->MouseUpDown = MouseUpDownSlider->GetValue();
		}
		if ( MouseLeftRightSlider )
		{
			SaveData->MouseLeftRight = MouseLeftRightSlider->GetValue();
		}
		if ( AimMouseUpDownSlider )
		{
			SaveData->AimMouseUpDown = AimMouseUpDownSlider->GetValue();
		}
		if ( AimMouseLeftRightSlider )
		{
			SaveData->AimMouseLeftRight = AimMouseLeftRightSlider->GetValue();
		}

		UGameplayStatics::SaveGameToSlot(SaveData, SaveSlot::SettingSlot, 0);
	}
}

void UMenu::OnMouseUpDownSliderChanged(float Value)
{
    if (MouseUpDownText)
    {
        // 소수점 첫째 자리까지 표시
        FString ValueString = FString::Printf(TEXT("%.1f"), Value);
        MouseUpDownText->SetText(FText::FromString(ValueString));
    }
}

void UMenu::OnMouseUpDownTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (MouseUpDownSlider && MouseUpDownText)
    {
        // 텍스트를 float으로 변환
        float Value = FCString::Atof(*Text.ToString());
        // 값을 1.0과 10.0 사이로 제한
        Value = FMath::Clamp(Value, 1.0f, 10.0f);

        // 슬라이더 값 설정
        MouseUpDownSlider->SetValue(Value);

        // 텍스트 박스 값도 제한된 값으로 다시 설정 (UI 동기화)
        FString ValueString = FString::Printf(TEXT("%.1f"), Value);
        MouseUpDownText->SetText(FText::FromString(ValueString));
    }
}

void UMenu::OnMouseLeftRightSliderChanged(float Value)
{
    if (MouseLeftRightText)
    {
        FString ValueString = FString::Printf(TEXT("%.1f"), Value);
        MouseLeftRightText->SetText(FText::FromString(ValueString));
    }
}

void UMenu::OnMouseLeftRightTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (MouseLeftRightSlider && MouseLeftRightText)
    {
        float Value = FCString::Atof(*Text.ToString());
        Value = FMath::Clamp(Value, 1.0f, 10.0f);

        MouseLeftRightSlider->SetValue(Value);

        FString ValueString = FString::Printf(TEXT("%.1f"), Value);
        MouseLeftRightText->SetText(FText::FromString(ValueString));
    }
}

void UMenu::OnAimMouseUpDownSliderChanged(float Value)
{
    if (AimMouseUpDownText)
    {
        FString ValueString = FString::Printf(TEXT("%.1f"), Value);
        AimMouseUpDownText->SetText(FText::FromString(ValueString));
    }
}

void UMenu::OnAimMouseUpDownTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (AimMouseUpDownSlider && AimMouseUpDownText)
    {
        float Value = FCString::Atof(*Text.ToString());
        Value = FMath::Clamp(Value, 1.0f, 10.0f);

        AimMouseUpDownSlider->SetValue(Value);

        FString ValueString = FString::Printf(TEXT("%.1f"), Value);
        AimMouseUpDownText->SetText(FText::FromString(ValueString));
    }
}

void UMenu::OnAimMouseLeftRightSliderChanged(float Value)
{
    if (AimMouseLeftRightText)
    {
        FString ValueString = FString::Printf(TEXT("%.1f"), Value);
        AimMouseLeftRightText->SetText(FText::FromString(ValueString));
    }
}

void UMenu::OnAimMouseLeftRightTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (AimMouseLeftRightSlider && AimMouseLeftRightText)
    {
        float Value = FCString::Atof(*Text.ToString());
        Value = FMath::Clamp(Value, 1.0f, 10.0f);

        AimMouseLeftRightSlider->SetValue(Value);

        FString ValueString = FString::Printf(TEXT("%.1f"), Value);
        AimMouseLeftRightText->SetText(FText::FromString(ValueString));
    }
}


// --------------------------------------------------------------------------------------------세이브 데이텨 관련 ㅗ

