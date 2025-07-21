#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BasePC.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);// 핑 경고 이벤트 델리게이트 선언

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

	// ------------------------------------------------------------------------------------------------------------------채팅 관련 ㅜ
	
	UPROPERTY()
	class UChatBox* ChatBox;

	UPROPERTY(EditAnywhere, Category = "자체설정")
	TSubclassOf<UChatBox> ChatBoxClass;

	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UInputAction* IA_EnterToChat; // 채팅 입력 액션

	void StartChat();

	UFUNCTION()
	void OnChatCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	bool bIsChatting = false; // 채팅 중인지 여부

	void OnMouseClickAnywhere(); // 채팅 중 마우스 클릭 시 채팅 종료
	
	// --------------------------------------------------------------------------------------------------------------------채팅 관련 ㅗ

	// ----------------------------------------------------------------------------------------------------------핑 관련 ㅜ
	
	// 핑 관련 == 이거 디폴트 엔진.ini 마지막에 [PacketSimulationSettings] PktLag = 100 설정 해놓아야 확인 가능해서 넣어놈!!!!!!
	float HighPingRunningTime = 0.f; // 핑 경고 실행 시간
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float HighPingDuration = 5.f; // 핑 경고 지속 시간
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float CheckPingFrequency = 20.f; // 핑 체크 빈도 (초 단위)
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float HighPingThreshold = 50.f; // 핑 경고 임계값 (ms 단위)
	float PingAnimationRunningTime = 0.f; // 핑 애니메이션 실행 시간

	UFUNCTION(Server, Reliable)
	void ServerReportPingStaus(bool bHighPing);
	
	// ----------------------------------------------------------------------------------------------------------핑 관련 ㅗ

	// --------------------------------------------------------------------------------게암 종료 관련 ㅜ
	
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UInputAction* IA_Quit; // 게임 종료 액션

	UPROPERTY(EditAnywhere, Category = "자체설정")
	TSubclassOf<class UUserWidget> ReturnToMainMenuWidget; // 메인 메뉴로 돌아가는 위젯 클래스
	UPROPERTY()
	class UReturnToMainMenu* ReturnToMainMenu; // 메인 메뉴로 돌아가는 위젯 인스턴스

	bool bReturnToMainMenuOpen = false; // 메인 메뉴로 돌아가는 위젯이 열려 있는지 여부

	void ShowReturnToMainMenu(); // 메인 메뉴로 돌아가는 함수
	
	// --------------------------------------------------------------------------------게암 종료 관련 ㅗ

protected:
	virtual void BeginPlay() override; // 플레이 시작 시 호출되는 함수
	virtual void SetupInputComponent() override; // 입력 컴포넌트를 설정하는 함수
	
	// ---------------------------------------------------------------------------------------------------------게임 모드에서 게임 상태 받아오기 함수들ㅜ
	
	// 이거는 비긴 플레이에서 처음 게임 시작할때 서버에서 매치 상태를 확인하는 함수
	UFUNCTION( Server, Reliable )
	void ServerCheckMatchState(); // 서버에서 매치 상태를 확인하는 함수
	UFUNCTION( Client, Reliable )
	void ClientJoinMidGame(FName StateOfMatch, float WarmUp, float Match, float LevelStarting, float CoolTime, bool bIsTeamMatch);
	// 클라이언트가 중간에 게임에 참여할 때 호출되는 함수
	
	// ---------------------------------------------------------------------------------------------------------게임 모드에서 게임 상태 받아오기 함수들ㅗ
	
public:
	virtual void OnPossess(APawn* InPawn) override; // 플레이어가 조종하는 폰이 변경될 때 호출되는 함수
	virtual void Tick(float DeltaTime) override; // 매 프레임마다 호출되는 함수
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; // 뱐수 복제 위한 함수 복제는 변수가 변했을떄만!
	virtual void ReceivedPlayer() override; // 플레이어가 컨트롤러를 받았을 때 호출되는 함수
	virtual void PawnLeavingGame() override; // 플레이어가 게임을 떠날 때 호출되는 함수
	
	// -----------------------------------------------------------------------------------------------------허드 세팅 관련 함수들ㅜ
	
	void SetHUDTime(); // HUD의 시간을 설정하는 함수
	void SetHUDHealth( float Health, float MaxHealth ); // HUD의 체력을 설정하는 함수
	void SetHUDShield( float Shield, float MaxShield ); // HUD의 방어력을 설정하는 함수
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
	void SetHUDGrenade( int32 Grenade ); // HUD의 수류탄 개수를 설정하는 함수
	
	// -----------------------------------------------------------------------------------------------------허드 세팅 관련 함수들ㅗ

	// -----------------------------------------------------------------------------------------------------매치 상태 관련 ㅜ
	
	void OnMatchStateSet(FName State, bool bTeamMatch = false); // 매치 상태가 변경될 때 호출되는 함수
	UFUNCTION()
	void OnRep_MatchState(); // 매치 상태가 변경되었을 때 호출되는 함수

	void HandleMatchHasStarted(bool bTeamMatch = false); // 매치가 시작되었을 때 호출되는 함수

	void HandleCoolDown(); // 쿨다운 상태를 처리하는 함수

	FString GetInfoText(const TArray<class ACharacterState*>& Players);
	FString GetTeamInfoText(class ABaseGameState* BaseGameState ); // 팀 정보 텍스트를 가져오는 함수
	
	// -----------------------------------------------------------------------------------------------------매치 상태 관련 ㅗ

	// -----------------------------------------------------------------------------------------------------서버랑 클라이언트 시간 싱크ㅜ
	
	UPROPERTY(EditAnyWhere, Category = "자체설정")
	float TimeSyncFrequency = 1.f; // 시간 동기화 빈도 (초 단위)

	float LastTimeSync = 0.f; // 마지막 시간 동기화 시간

	float ClientServerDeltaTime = 0.f; // 클라이언트와 서버 간의 시간 차이
	UFUNCTION( Server, Reliable )
	void ServerRequestServerTime( float TimeOfClientRequest ); // 서버에 시간 요청을 보내는 함수

	UFUNCTION( Client, Reliable )
	void ClientReportServerTime( float TimeOfClientRequest, float TimeServerReceivedRequest ); // 서버 시간 보고 함수

	void CheckTimeSync( float DeltaTime ); // 시간 동기화 확인 함수
	
	// ------------------------------------------------------------------------------------------------------서버랑 클라이언트 시간 싱크ㅗ

	// ------------------------------------------------------------------------------------------------------ 핑 관련 ㅜ
	
	virtual float GetServerTime();// 다른 데서 서버 시간을 가져오는 함수

	float SingleTripTime = 0.f; // 단일 왕복 시간

	FHighPingDelegate HighPingDelegate; // 핑 경고 이벤트 델리게이트

	void CheckPing(float DeltaTime);
	void HighPingWarning(); // 핑 경고 함수
	void StopHighPingWarning(); // 핑 경고 중지 함수
	
	//---------------------------------------------------------------------------------------------------------핑 관련 ㅗ

	// ------------------------------------------------------------------------------------------------------ 킬 로그 알림 ㅜ
	
	void BroadcastElim(class ACharacterState* Attacker, class ACharacterState* Victim);

	UFUNCTION(Client, Reliable)
	void CleintElimAnnouncement(class ACharacterState* Attacker, class ACharacterState* Victim); // 제거 알림을 클라이언트에 표시하는 함수
	
	// ------------------------------------------------------------------------------------------------------ 킬 로그 알림 ㅗ
	
	// ------------------------------------------------------------------------------------------------------ 채팅  ㅜ
	
	UFUNCTION(Server, Reliable)
	void ServerChatCommitted(const FText& Text, const FString& PlayerName);

	UFUNCTION(Client, Reliable)
	void ClientChatCommitted(const FText& Text, const FString& PlayerName);

	void AddChatBox(); // 채팅 박스 위젯을 추가하는 함수
	
	// ------------------------------------------------------------------------------------------------------ 채팅  ㅗ

	// ------------------------------------------------------------------------------------------------------ 팀 관련 ㅜ

	void PollInit(float DeltaTime); // 허드 관련 초기화

	void HideTeamScore(); // 팀 점수를 숨기는 함수
	void InitTeamScore(); // 팀 점수를 초기화하는 함수
	void SetHUDRedTeamScore(int32 RedScore); // 레드 팀 점수를 설정하는 함수
	void SetHUDBlueTeamScore(int32 BlueScore); // 블루 팀 점수를 설정하는 함수

	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamScore)
	bool bShowTeamScore; // 팀 점수를 표시할지 여부

	bool bInitTeamScore = false; // 팀 점수 초기화 여부

	UFUNCTION()
	void OnRep_ShowTeamScore(); // 

	//------------------------------------------------------------------------------------------------------ 팀 관련 ㅗ
};
