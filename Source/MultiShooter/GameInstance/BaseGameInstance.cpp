#include "BaseGameInstance.h"
#include "Kismet/GameplayStatics.h"

#include "GameSave.h"

void UBaseGameInstance::Init()
{
	Super::Init();

	// ProfileSlot에 해당하는 세이브 파일이 존재하는지 확인합니다.
	if (UGameplayStatics::DoesSaveGameExist( SaveSlot::ProfileSlot, 0 ) == false )
	{
		UGameSave* ProfileSaveData = Cast<UGameSave>(UGameplayStatics::CreateSaveGameObject(UGameSave::StaticClass()));
		if (ProfileSaveData)
		{
			ProfileSaveData->SelectedSkeletalMesh = TEXT("BasicMale_SK");
			ProfileSaveData->PlayerName = TEXT("VICE");
            
			// 설정된 기본값으로 세이브 파일을 생성합니다.
			UGameplayStatics::SaveGameToSlot(ProfileSaveData, SaveSlot::ProfileSlot, 0);
		}
	}
	if (UGameplayStatics::DoesSaveGameExist( SaveSlot::SettingSlot, 0 ) == false)
	{
		UGameSave* SettingSaveData = Cast<UGameSave>(UGameplayStatics::CreateSaveGameObject(UGameSave::StaticClass()));
		if (SettingSaveData)
		{
			SettingSaveData->MouseLeftRight = 5.0f;
			SettingSaveData->MouseUpDown = 5.0f;
			SettingSaveData->AimMouseLeftRight = 5.0f;
			SettingSaveData->AimMouseUpDown = 5.0f;
			
			// 설정된 기본값으로 세이브 파일을 생성합니다.
			UGameplayStatics::SaveGameToSlot(SettingSaveData, SaveSlot::SettingSlot, 0);
		}
	}
}
