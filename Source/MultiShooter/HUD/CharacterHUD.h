#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CharacterHUD.generated.h"

USTRUCT(BlueprintType) // 십자선 HUD를 위한 구조체 블루프린트에서 사용 가능하게 설정
struct FHUDPackage
{
	GENERATED_BODY()
public:
	class UTexture2D* CrosshairCenter; // 십자선 중앙 텍스처
	class UTexture2D* CrosshairLeft;   // 십자선 왼쪽
	class UTexture2D* CrosshairRight;  // 십자선 오른쪽
	class UTexture2D* CrosshairTop;    // 십자선 위쪽
	class UTexture2D* CrosshairBottom; // 십자선 아래쪽
	float CrosshairSpread; // 십자선 확산 정도
	FLinearColor CrosshairColor;
};


UCLASS()
class MULTISHOOTER_API ACharacterHUD : public AHUD
{
	GENERATED_BODY()

	FHUDPackage HUDPackage; // 십자선 HUD 패키지

	UPROPERTY(EditDefaultsOnly)
	float CrosshairSpreadMax = 16.f; // 십자선 최대 확산 정도

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);

protected:
	virtual void BeginPlay() override;
	void AddCharacterUI(); // 캐릭터 UI 위젯을 추가하는 함수

public:
	virtual void DrawHUD() override;

	FORCEINLINE void SetHUDPackage(const FHUDPackage& NewHUDPackage) { HUDPackage = NewHUDPackage; }

	class UCharacterUI* CharacterUI; // 캐릭터 UI 위젯

	UPROPERTY(EditAnywhere, Category = "PlayerStat")
	TSubclassOf<class UUserWidget> CharacterUIClass; // 캐릭터 UI 클래스
	
};
