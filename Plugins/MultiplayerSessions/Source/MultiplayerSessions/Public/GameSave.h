#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameSave.generated.h"

namespace  SaveSlot // 네임스페이스로 저장 슬롯 이름 관리 SaveSlot::
{
	constexpr const TCHAR* ProfileSlot = TEXT("DefaultPlayerSaveSlot");
	constexpr const TCHAR* SettingSlot = TEXT("SettingSaveSlot");
	// - constexpr: 컴파일 타임에 값이 결정되는 상수를 정의할 때 사용됩니다. 이는 성능 최적화에 도움이 됩니다.
	// - const TCHAR*: TCHAR 타입의 문자열 상수를 가리키는 포인터입니다.
	// - TEXT 매크로: 유니코드와 멀티바이트 문자셋을 모두 지원하기 위해 문자열 리터럴을 적절히 처리합니다.
}	
// 최초의 세이브 데이터 생성지는 BaseGameInstance 에서 생성!

namespace NickName
{
	constexpr const TCHAR* DefaultName = TEXT("adas123213asedasdasdasdassaddasd");
}

UCLASS()
class MULTIPLAYERSESSIONS_API UGameSave : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere)
	FString SelectedSkeletalMesh = "BasicMale_SK";

	UPROPERTY(VisibleAnywhere)
	FString PlayerName = "adas123213asedasdasdasdassaddasd"; // 기본 이름 설정

	UPROPERTY(VisibleAnywhere) // 수평 감도
	float MouseLeftRight = 5.0f;

	UPROPERTY(VisibleAnywhere) // 수직 감도
	float MouseUpDown = 5.0f;

	UPROPERTY(VisibleAnywhere) // 수평 감도
	float AimMouseLeftRight = 5.0f;

	UPROPERTY(VisibleAnywhere) // 수직 감도
	float AimMouseUpDown = 5.0f;
	
};
