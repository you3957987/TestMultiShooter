#include "BaseGameInstance.h"
#include "Menu.h"

void UBaseGameInstance::Init()
{
	Super::Init();
}

void UBaseGameInstance::SetPlayerName(const FString& Name)
{
	PlayerName = Name;
	UE_LOG(LogTemp, Warning, TEXT( "PlayerName : %s"), *PlayerName);
	OnNameChanged.Broadcast(); // 오버 헤드 위젯한테 이름 변경됬다고 방송
}

void UBaseGameInstance::ShowMenu()
{
	if ( MenuClass )
	{
		MenuWidget = CreateWidget<UMenu>(this, MenuClass);
		if ( MenuWidget )
		{
			MenuWidget->OnPlayerNameSet.AddDynamic(this, &UBaseGameInstance::SetPlayerName);
			MenuWidget->MenuSetup(4, "FreeForAll", "/Game/Maps/Lobby");
		}
	}
}
