#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterUI.generated.h"


UCLASS()
class MULTISHOOTER_API UCharacterUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget)) // 위젯 바인딩
	class UTextBlock* HealthText; // 플레이어 이름 텍스트 블록 바인딩

	UPROPERTY(meta = (BindWidget)) // 위젯 바인딩
	class UProgressBar* HealthBar; // 체력 바 바인딩

	UPROPERTY(meta = (BindWidget)) 
	class UTextBlock* ShieldText; 

	UPROPERTY(meta = (BindWidget)) 
	class UProgressBar* ShieldBar; 

	UPROPERTY(meta = (BindWidget)) // 위젯 바인딩
	class UTextBlock* ScoreAmount; // 플레이어 점수 텍스트 블록 바인딩

	UPROPERTY(meta = (BindWidget)) // 위젯 바인딩
	class UTextBlock* DefeatAmount; // 플레이어 죽은 점수 텍스트 블록 바인딩

	UPROPERTY(meta = (BindWidget)) 
	class UTextBlock* DeathMessageText; 

	UPROPERTY(meta = (BindWidget)) 
	class UTextBlock* KilledByText; 

	UPROPERTY(meta = (BindWidget)) 
	class UTextBlock* AmmoAmount;

	UPROPERTY(meta = (BindWidget)) 
	class UTextBlock* CarriedAmmoAmount;

	UPROPERTY(meta = (BindWidget)) 
	class UImage* WeaponIcon;

	UPROPERTY(meta = (BindWidget)) 
	class UTextBlock* MatchCountDownText;

	UPROPERTY(meta = (BindWidget)) 
	class UTextBlock* GrenadeText;

	UPROPERTY(meta = (BindWidget))
	class UImage* WifiImage;

	UPROPERTY(meta = (BindWidgetAnim), Transient) // 위젯 애니메이션 바인딩
	UWidgetAnimation* WifiAnim; // 위젯 애니메이션

};
