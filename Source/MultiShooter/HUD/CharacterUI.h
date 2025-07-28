#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterUI.generated.h"


UCLASS()
class MULTISHOOTER_API UCharacterUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override; // 위젯 틱 함수 오버라이드

	UPROPERTY(meta = (BindWidget)) // 위젯 바인딩
	class UTextBlock* HealthText; // 플레이어 이름 텍스트 블록 바인딩

	UPROPERTY(meta = (BindWidget)) // 위젯 바인딩
	class UProgressBar* HealthBar; // 체력 바 바인딩

	UPROPERTY(meta = (BindWidget)) 
	class UTextBlock* ShieldText;  // 쉴드 량

	UPROPERTY(meta = (BindWidget)) 
	class UProgressBar* ShieldBar;  // 쉴드 바 바인딩

	UPROPERTY(meta = (BindWidget)) // 위젯 바인딩
	class UTextBlock* ScoreAmount; // 플레이어 점수 텍스트 블록 바인딩

	UPROPERTY(meta = (BindWidget)) // 위젯 바인딩
	class UTextBlock* DefeatAmount; // 플레이어 죽은 점수 텍스트 블록 바인딩

	UPROPERTY(meta = (BindWidget)) 
	class UTextBlock* DeathMessageText; // 

	UPROPERTY(meta = (BindWidget)) 
	class UTextBlock* KilledByText; 

	UPROPERTY(meta = (BindWidget)) 
	class UTextBlock* AmmoAmount; // 현재 장착된 총의 탄약량 텍스트 블록 바인딩

	UPROPERTY(meta = (BindWidget)) 
	class UTextBlock* CarriedAmmoAmount; // 현재 장착된 총의 캐릭터가 보유한 예비 탄약량 텍스트 블록 바인딩

	UPROPERTY(meta = (BindWidget)) 
	class UImage* WeaponIcon; // 현재 장착된 총의 아이콘 이미지 바인딩

	UPROPERTY(meta = (BindWidget)) 
	class UTextBlock* MatchCountDownText; // 게임 남은 시간 카운트다운 텍스트 블록 바인딩

	UPROPERTY(meta = (BindWidget)) 
	class UTextBlock* GrenadeText; // 현재 장착된 수류탄의 개수 텍스트 블록 바인딩

	UPROPERTY(meta = (BindWidget))
	class UImage* WifiImage; // 핑 높으면 깜박일 수 있는 이미지 바인딩

	UPROPERTY(meta = (BindWidgetAnim), Transient) // 위젯 애니메이션 바인딩
	UWidgetAnimation* WifiAnim; // 위젯 애니메이션

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* RedTeamScore; // 레드 팀 점수 텍스트 블록 바인딩

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* BlueTeamScore; // 블루 팀 점수 텍스트 블록 바인딩

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ScoreSpacerText; // 점수 사이의 공백 텍스트 블록 바인딩

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PingText; // 팀 이름 텍스트 블록 바인딩

	//--------------------------------------------------------------------------------------구르기 관련 변수들ㅜ
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* RollBar; // 구르기 쿨타임 시간 바인딩

	// 현재 남은 쿨타임
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Roll")
	float CurrentRollCooldown = 0.0f;

	float RollCooldownTime = 5.0f; // 구르기 쿨타임 시간

	// 쿨타임 시작 함수
	UFUNCTION(BlueprintCallable, Category = "Roll")
	void StartRollCooldown(float CooldownTime);

	//--------------------------------------------------------------------------------------구르기 관련 변수들ㅗ

	//-------------------------------------------------------------------------------------- 백대쉬 관련 변수들ㅜ

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* BackDashBar; // 구르기 쿨타임 시간 바인딩

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BackDash")
	float CurrentBackDashCooldown = 0.0f; // 현재 남은 백대쉬 쿨타임

	float BackDashCooldownTime = 7.0f; // 백대쉬 쿨타임 시간

	UFUNCTION(BlueprintCallable, Category = "BackDash")
	void StartBackDashCooldown(float CooldownTime);

	//--------------------------------------------------------------------------------------- 백대쉬 관련 변수들ㅗ

};
