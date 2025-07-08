#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BasePC.generated.h"

UCLASS()
class MULTISHOOTER_API ABasePC : public APlayerController
{
	GENERATED_BODY()

	UPROPERTY()
	class ACharacterHUD* CharacterHUD; // 캐릭터 HUD 클래스

	float LevelStartingTime = 0.f; // 레벨 시작 시간 (초 단위)
	float MatchTime = 0.f; // 매치 시간 (초 단위)
	float WarmUpTime = 0.f; // 준비 시간 (초 단위)
	float CooldownTime = 0.f; // 
	uint32 CountDownTimeInt = 0; // 카운트다운 시간 (초 단위)

	UPROPERTY( ReplicatedUsing = OnRep_MatchState )
	FName MatchState; // 매치 상태 이름
	
protected:
	virtual void BeginPlay() override; // 플레이 시작 시 호출되는 함수
	
	void SetHUDTime(); // HUD의 시간을 설정하는 함수
	UFUNCTION( Server, Reliable )
	void ServerCheckMatchState(); // 서버에서 매치 상태를 확인하는 함수
	UFUNCTION( Client, Reliable )
	void ClientJoinMidGame(FName StateOfMatch, float WarmUp, float Match, float LevelStarting, float CoolTime); // 클라이언트가 중간에 게임에 참여할 때 호출되는 함수

public:
	virtual void OnPossess(APawn* InPawn) override; // 플레이어가 조종하는 폰이 변경될 때 호출되는 함수
	virtual void Tick(float DeltaTime) override; // 매 프레임마다 호출되는 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; // 뱐수 복제 위한 함수 복제는 변수가 변했을떄만!
	virtual void ReceivedPlayer() override; // 플레이어가 컨트롤러를 받았을 때 호출되는 함수

	virtual float GetServerTime(); 
	
	void SetHUDHealth( float Health, float MaxHealth ); // HUD의 체력을 설정하는 함수
	void SetHUDScore( float Score ); // HUD의 점수를 설정하는 함수
	void SetHUDDefeat( int32 Defeat ); // HUD의 죽은 횟수 상태를 설정하는 함수
	void SetHUDKilledBy( FString KilledBy ); // HUD의 죽은 메시지를 설정하는 함수
	void HideHUDKilledBy(); // HUD의 죽은 메시지를 숨기는 함수
	void SetHUDAmmo( int32 Ammo ); // HUD의 총알 개수를 설정하는 함수
	void SetHUDCarriedAmmo( int32 CarriedAmmo ); // HUD의 보유 총알 개수를 설정하는 함수
	void SetHUDWeaponIcon( UTexture2D* WeaponIcon ); // HUD의 무기 아이콘을 설정하는 함수
	void SetHUDWeponIconClear(); // HUD의 무기 아이콘을 지우는 함수
	void SetHUDMatchCountDown( float CountDownTime ); // HUD의 매치 카운트다운을 설정하는 함수
	void SetHUDAnnouncementCountDown( float CountDownTime ); // HUD의 매치 카운트다운을 설정하는 함수
	
	void OnMatchStateSet(FName State); // 매치 상태가 변경될 때 호출되는 함수
	UFUNCTION()
	void OnRep_MatchState(); // 매치 상태가 변경되었을 때 호출되는 함수

	void HandleMatchHasStarted(); // 매치가 시작되었을 때 호출되는 함수

	void HandleCoolDown(); // 쿨다운 상태를 처리하는 함수

	// 서버랑 클라이언트 시간 싱크!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	UFUNCTION( Server, Reliable )
	void ServerRequestServerTime( float TimeOfClientRequest ); // 서버에 시간 요청을 보내는 함수

	UFUNCTION( Client, Reliable )
	void ClientReportServerTime( float TimeOfClientRequest, float TimeServerReceivedRequest ); // 서버 시간 보고 함수

	float ClientServerDeltaTime = 0.f; // 클라이언트와 서버 간의 시간 차이

	UPROPERTY(EditAnyWhere, Category = "Time")
	float TimeSyncFrequency = 5.f; // 시간 동기화 빈도 (초 단위)

	float LastTimeSync = 0.f; // 마지막 시간 동기화 시간

	void CheckTimeSync( float DeltaTime ); // 시간 동기화 확인 함수

	// 서버랑 클라이언트 시간 싱크!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

};
