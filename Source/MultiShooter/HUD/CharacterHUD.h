#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CharacterHUD.generated.h"

USTRUCT(BlueprintType) // 십자선 HUD를 위한 구조체 블루프린트에서 사용 가능하게 설정
struct FHUDPackage
{
	GENERATED_BODY()
public:
	UPROPERTY()
	class UTexture2D* CrosshairCenter; // 십자선 중앙 텍스처
	UPROPERTY()
	class UTexture2D* CrosshairLeft;   // 십자선 왼쪽
	UPROPERTY()
	class UTexture2D* CrosshairRight;  // 십자선 오른쪽
	UPROPERTY()
	class UTexture2D* CrosshairTop;    // 십자선 위쪽
	UPROPERTY()
	class UTexture2D* CrosshairBottom; // 십자선 아래쪽
	FHUDPackage()
        : CrosshairCenter(nullptr)
        , CrosshairLeft(nullptr)
        , CrosshairRight(nullptr)
        , CrosshairTop(nullptr)
        , CrosshairBottom(nullptr)
    {}
	float CrosshairSpread; // 십자선 확산 정도
	FLinearColor CrosshairColor;
};

UCLASS()
class MULTISHOOTER_API ACharacterHUD : public AHUD
{
	GENERATED_BODY()

	FHUDPackage HUDPackage; // 십자선 HUD 패키지

	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	float CrosshairSpreadMax = 16.f; // 십자선 최대 확산 정도

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);

	UPROPERTY(EditAnyWhere, Category = "자체설정")
	TSubclassOf<class UElimAnnouncement> ElimAnnouncementClass; // 제거 알림 위젯 클래스

	UPROPERTY(EditAnyWhere, Category = "자체설정")
	float ElimAnnouncementTime = 1.5f; // 제거 알림 실행 시간

	class APlayerController* OwningPlayerController; // 플레이어 컨트롤러

	UFUNCTION()
	void ElimAnouncementTimerFinished(UElimAnnouncement* MessageToRemove); // 제거 알림 타이머가 끝났을 때 호출되는 함수

	UPROPERTY()
	TArray<UElimAnnouncement*> ElimMessages; // 제거 알림 위젯 배열

protected:
	virtual void BeginPlay() override;

public:
	virtual void DrawHUD() override;

	FORCEINLINE void SetHUDPackage(const FHUDPackage& NewHUDPackage) { HUDPackage = NewHUDPackage; }

	UPROPERTY()
	class UCharacterUI* CharacterUI; // 캐릭터 UI 위젯

	UPROPERTY(EditAnywhere, Category = "자체설정")
	TSubclassOf<class UUserWidget> CharacterUIClass; // 캐릭터 UI 클래스

	void AddCharacterUI(); // 캐릭터 UI 위젯을 추가하는 함수

 	UPROPERTY()
	class UAnnouncement* Announcement; // 캐릭터 UI 위젯

	UPROPERTY(EditAnywhere, Category = "자체설정")
	TSubclassOf<class UUserWidget> AnnouncementClass; // 시작전 안내 클래스

	void AddAnnouncement(); // 시작전 안내 위젯을 추가하는 함수

	void AddElimAnnouncement(FString Attacker, FString Victim); // 제거 알림 위젯을 추가하는 함수
	
};
