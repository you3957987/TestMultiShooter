#include "BaseGameInstance.h"
#include "Kismet/GameplayStatics.h"

#include "MultiShooter/SaveData/SaveData.h"

void UBaseGameInstance::Init()
{
	Super::Init();

	// ProfileSlot에 해당하는 세이브 파일이 존재하는지 확인합니다.
	if (UGameplayStatics::DoesSaveGameExist( SaveSlot::ProfileSlot, 0 ) == false )
	{
		// 세이브 파일이 없으면 새로운 SaveData 객체를 생성합니다.
		USaveData* ProfileSaveData = Cast<USaveData>(UGameplayStatics::CreateSaveGameObject(USaveData::StaticClass()));
		if (ProfileSaveData)
		{
			// 여기에 기본값을 설정합니다. 예를 들어, 기본 캐릭터 메쉬 이름 등
			ProfileSaveData->SelectedSkeletalMesh = TEXT("BasicMale_SK");
			ProfileSaveData->PlayerName = TEXT("VICE");
            
			// 설정된 기본값으로 세이브 파일을 생성합니다.
			UGameplayStatics::SaveGameToSlot(ProfileSaveData, SaveSlot::ProfileSlot, 0);
		}
	}
	if (UGameplayStatics::DoesSaveGameExist( SaveSlot::SettingSlot, 0 ) == false)
	{
		USaveData* SettingSaveData = Cast<USaveData>(UGameplayStatics::CreateSaveGameObject(USaveData::StaticClass()));
		if (SettingSaveData)
		{
			// 여기에 기본값을 설정합니다. 예를 들어, 기본 캐릭터 메쉬 이름 등
			SettingSaveData->MouseLeftRight = 1.0f;
			SettingSaveData->MouseUpDown = 1.0f;
			
			// 설정된 기본값으로 세이브 파일을 생성합니다.
			UGameplayStatics::SaveGameToSlot(SettingSaveData, SaveSlot::SettingSlot, 0);
		}
	}
}
