#include "ServerList.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

#include "Menu.h"

void UServerList::SetServerListInfo(FString SessionName, FString MatchType, int32 NumPlayer, int32 MaxPlayer)
{
	StoredSessionName = SessionName;
	StoredMatchType = MatchType;
	StoredNumPlayer = NumPlayer;
	StoredMaxPlayer = MaxPlayer;

	if (ServerTextBlock)
	{
		ServerTextBlock->SetText(FText::FromString(FString::Printf(TEXT("%s :: %s   [%d/%d]"), 
			*SessionName, *MatchType, 1 + NumPlayer, MaxPlayer)));
	}

	if (NumPlayer + 1 >= MaxPlayer) // 서버가 꽉 차면 버튼 비활성화
	{
		if (ServerSelectButton)
		{
			ServerSelectButton->SetIsEnabled(false);
		}
	}
	else // 아니면 버튼 활성화
	{
		if (ServerSelectButton)
		{
			ServerSelectButton->SetIsEnabled(true);
			ServerSelectButton->OnClicked.AddDynamic(this, &UServerList::OnServerSelected);
		}
	}
}

void UServerList::OnServerSelected()
{
	if (MenuWidget)
	{
		MenuWidget->SetJoinSessionName(StoredSessionName, StoredMatchType);
	}
}
