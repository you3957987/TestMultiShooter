// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Components/EditableTextBox.h"
#include "Components/ScrollBox.h"
#include "Components/CheckBox.h"

#include "ServerList.h"

// 초기화. 비긴 플레이같은거
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

