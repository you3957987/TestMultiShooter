#include "BasePC.h"
#include "MultiShooter/HUD/CharacterHUD.h"
#include "MultiShooter/HUD/CharacterUI.h"
#include "Components/Progressbar.h"
#include "Components/TextBlock.h"
#include "MultiShooter/Character/BaseCharacter.h"
#include "Components/Image.h"
#include "Net/UnrealNetwork.h" // 네트워크 관련 헤더 파일
#include "Kismet/GameplayStatics.h" // 게임 관련 유틸리티 함수 헤더 파일
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "Components/EditableTextBox.h"

#include "MultiShooter/GameMode/MainGameMode.h"
#include "MultiShooter/HUD/Announcement.h" // 시작전 안내 위젯 헤더 파일
#include "MultiShooter/CharacterComponent/CombatComponent.h" // 전투 컴포넌트 헤더 파일
#include "MultiShooter/GameState/BaseGameState.h" // 게임 상태 헤더 파일
#include "MultiShooter/CharacterState/CharacterState.h" // 플레이어 상태 헤더 파일
#include "MultiShooter/HUD/ReturnToMainMenu.h" // 메인 메뉴로 돌아가는 위젯 헤더 파일
#include "MultiShooter/HUD/ChatBox.h" // 채팅 박스 위젯 헤더 파일

void ABasePC::BeginPlay()
{
    Super::BeginPlay();

    CharacterHUD = Cast<ACharacterHUD>(GetHUD()); // 현재 플레이어의 HUD를 캐릭터 HUD로 캐스팅합니다.
    ServerCheckMatchState(); // 서버에서 매치 상태를 확인합니다.

    if (InputComponent)
    {
        InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ABasePC::OnMouseClickAnywhere);
        InputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &ABasePC::OnMouseClickAnywhere);
    }
}

void ABasePC::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime); // 부모 클래스의 Tick 함수를 호출합니다.

    SetHUDTime(); // HUD의 시간을 설정합니다.

    CheckTimeSync(DeltaTime); // 시간 동기화 확인 함수 호출

	CheckPing(DeltaTime); // 핑 체크 함수 호출
}

void ABasePC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps); // 부모 클래스의 GetLifetimeReplicatedProps 함수를 호출합니다.

    DOREPLIFETIME(ABasePC, MatchState); // MatchState 변수를 복제할 수 있도록 설정합니다.
}

void ABasePC::SetupInputComponent()
{
    Super::SetupInputComponent(); // 부모 클래스의 SetupInputComponent 함수를 호출합니다.
    
    UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
    if (!EnhancedInput) return;

    // IA_QuitButton_WhenInProgress는 UInputAction* 타입의 멤버 변수로 선언되어 있어야 합니다.
  
    EnhancedInput->BindAction(IA_Quit, ETriggerEvent::Started, this, &ABasePC::ShowReturnToMainMenu);

	EnhancedInput->BindAction(IA_EnterToChat, ETriggerEvent::Started, this, &ABasePC::StartChat); // 채팅 입력 액션 바인딩
    
}

// 빙의 되었을떄 허드 상태를 초기화하는 함수
void ABasePC::OnPossess(APawn* InPawn) // 초기 UI 상태 설정
{
    Super::OnPossess(InPawn); // 부모 클래스의 OnPossess 함수를 호출합니다.

    ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(InPawn); // 조종하는 폰을 ABaseCharacter로 캐스팅합니다.
    if (BaseCharacter)
    {
        SetHUDHealth(BaseCharacter->GetHealth(), BaseCharacter->GetMaxHealth()); // 캐릭터의 체력을 HUD에 설정합니다.
        SetHUDShield(BaseCharacter->GetShield(), BaseCharacter->GetMaxShield()); // 캐릭터의 방어력을 HUD에 설정합니다.
        SetHUDGrenade(BaseCharacter->GetCombatComponent()->GetGrenade());
    }
    
}

// 플레이어가 연결된 후 호출. ServerRequestServerTime() 를 호출해서 서버한테 시간을 요청
void ABasePC::ReceivedPlayer()
{
    Super::ReceivedPlayer(); // 부모 클래스의 ReceivedPlayer 함수를 호출합니다.

    if( IsLocalController() )
    {
        ServerRequestServerTime(GetWorld()->GetTimeSeconds()); // 서버에 시간 요청을 보냅니다.
    }
}

// ------------------------------------------------------------------------------------------------------------------허드 세팅 관련 함수들ㅜ

// 시간을 관리하면서 허드를 총괄 관리하는 함수. 여기서 서버랑 클라랑 시간 동기화도 일어남
void ABasePC::SetHUDTime()
{
    AMainGameMode* LocalGameMode = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this));

    float TimeLeft = 0.f;
    if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmUpTime - GetServerTime() + LevelStartingTime;
    else if (MatchState == MatchState::InProgress) TimeLeft = WarmUpTime + MatchTime - GetServerTime() + LevelStartingTime;
    else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmUpTime + MatchTime - GetServerTime() + LevelStartingTime;

    uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

    // 서버라면 매번 GameMode에서 LevelStartingTime을 가져옴
    if (HasAuthority())
    {
        if (LocalGameMode)
        {
            LevelStartingTime = LocalGameMode->LevelStartingTime;
            SecondsLeft = FMath::CeilToInt(LocalGameMode->GetCountDownTime() + LevelStartingTime);
        }
    }

    if (!HasAuthority()) // 또는 if (IsLocalController())
    {
        //UE_LOG(LogTemp, Warning, TEXT("SetHUDTime: State=%s, WarmUp=%f, Match=%f, LevelStart=%f, Cooldown=%f, GetServerTime=%f, TimeLeft=%f"),
        //*MatchState.ToString(), WarmUpTime, MatchTime, LevelStartingTime, CooldownTime, GetServerTime(), TimeLeft);
    }
    if (HasAuthority()) // 또는 if (IsLocalController())
    {
        //UE_LOG(LogTemp, Warning, TEXT("SetHUDTime: State=%s, WarmUp=%f, Match=%f, LevelStart=%f, Cooldown=%f, GetServerTime=%f, TimeLeft=%f"),
        //*MatchState.ToString(), WarmUpTime, MatchTime, LevelStartingTime, CooldownTime, GetServerTime(), TimeLeft);
    }

    if (CountDownTimeInt != SecondsLeft)
    {
        if (MatchState == MatchState::WaitingToStart)
        {
            SetHUDAnnouncementCountDown(TimeLeft);
        }
        else if (MatchState == MatchState::InProgress)
        {
            SetHUDMatchCountDown(TimeLeft);
        }
        else if (MatchState == MatchState::Cooldown)
        {
            SetHUDAnnouncementCountDown(TimeLeft);
        }
    }
    CountDownTimeInt = SecondsLeft;
}

void ABasePC::SetHUDHealth(float Health, float MaxHealth)
{
    if (CharacterHUD && CharacterHUD->CharacterUI == nullptr)
    {
        CharacterHUD->AddCharacterUI();
        if (CharacterHUD->CharacterUI == nullptr)
            return;
    }

    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD; // 캐릭터 HUD가 nullptr인 경우 현재 플레이어의 HUD를 캐스팅합니다.

    //if( CharacterHUD ) UE_LOG(LogTemp, Warning, TEXT("CharacterHUD is valid.")); // 캐릭터 HUD가 유효한 경우 로그 출력
    //if( CharacterHUD && CharacterHUD->CharacterUI ) UE_LOG(LogTemp, Warning, TEXT("CharacterUI is valid.")); // 캐릭터 UI가 유효한 경우 로그 출력

    if (CharacterHUD && CharacterHUD->CharacterUI 
        && CharacterHUD->CharacterUI->HealthBar && CharacterHUD->CharacterUI->HealthText ) // 캐릭터 HUD와 캐릭터 UI가 유효한 경우
    {
        const float HealthPercent = Health / MaxHealth; // 체력 비율 계산
        CharacterHUD->CharacterUI->HealthBar->SetPercent(HealthPercent); // 체력 바의 비율을 설정합니다.
        
        FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth) ); // 체력 텍스트 포맷팅
        CharacterHUD->CharacterUI->HealthText->SetText(FText::FromString(HealthText)); // 체력 텍스트를 설정합니다.
    }
}

void ABasePC::SetHUDShield( float Shield, float MaxShield ) // HUD의 방어력을 설정하는 함수
{
    if (CharacterHUD && CharacterHUD->CharacterUI == nullptr)
    {
        CharacterHUD->AddCharacterUI();
        if (CharacterHUD->CharacterUI == nullptr)
            return;
    }

    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD; // 캐릭터 HUD가 nullptr인 경우 현재 플레이어의 HUD를 캐스팅합니다.

    //if( CharacterHUD ) UE_LOG(LogTemp, Warning, TEXT("CharacterHUD is valid.")); // 캐릭터 HUD가 유효한 경우 로그 출력
    //if( CharacterHUD && CharacterHUD->CharacterUI ) UE_LOG(LogTemp, Warning, TEXT("CharacterUI is valid.")); // 캐릭터 UI가 유효한 경우 로그 출력

    if (CharacterHUD && CharacterHUD->CharacterUI 
        && CharacterHUD->CharacterUI->ShieldBar && CharacterHUD->CharacterUI->ShieldText ) // 캐릭터 HUD와 캐릭터 UI가 유효한 경우
    {
        const float ShieldPercent = Shield / MaxShield; // 체력 비율 계산
        CharacterHUD->CharacterUI->ShieldBar->SetPercent(ShieldPercent); // 체력 바의 비율을 설정합니다.
        
        FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield) ); // 체력 텍스트 포맷팅
        CharacterHUD->CharacterUI->ShieldText->SetText(FText::FromString(ShieldText)); // 체력 텍스트를 설정합니다.
    }
}

void ABasePC::SetHUDScore(float Score)
{
    if (CharacterHUD && CharacterHUD->CharacterUI == nullptr)
    {
        CharacterHUD->AddCharacterUI();
        if (CharacterHUD->CharacterUI == nullptr)
            return;
    }

    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD; // 캐릭터 HUD가 nullptr인 경우 현재 플레이어의 HUD를 캐스팅합니다.

    //if( CharacterHUD ) UE_LOG(LogTemp, Warning, TEXT("CharacterHUD is valid.")); // 캐릭터 HUD가 유효한 경우 로그 출력
    //if( CharacterHUD && CharacterHUD->CharacterUI ) UE_LOG(LogTemp, Warning, TEXT("CharacterUI is valid.")); // 캐릭터 UI가 유효한 경우 로그 출력

    if (CharacterHUD && CharacterHUD->CharacterUI && CharacterHUD->CharacterUI->ScoreAmount ) // 캐릭터 HUD와 캐릭터 UI가 유효한 경우
    {
        FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score)); // 점수 텍스트 포맷팅
        CharacterHUD->CharacterUI->ScoreAmount->SetText(FText::FromString(ScoreText)); // 점수 텍스트를 설정합니다.
    }
}

void ABasePC::SetHUDAmmo(int32 Ammo)
{
    if (CharacterHUD && CharacterHUD->CharacterUI == nullptr)
    {
        CharacterHUD->AddCharacterUI();
        if (CharacterHUD->CharacterUI == nullptr)
            return;
    }

    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD; // 캐릭터 HUD가 nullptr인 경우 현재 플레이어의 HUD를 캐스팅합니다.

    if (CharacterHUD && CharacterHUD->CharacterUI && CharacterHUD->CharacterUI->AmmoAmount) // 캐릭터 HUD와 캐릭터 UI가 유효한 경우
    {
        FString AmmoText = FString::Printf(TEXT("%d"), Ammo); // 총알 개수 텍스트 포맷팅
        CharacterHUD->CharacterUI->AmmoAmount->SetText(FText::FromString(AmmoText)); // 총알 개수 텍스트를 설정합니다.
    }
}

void ABasePC::SetHUDCarriedAmmo(int32 Ammo)
{
    if (CharacterHUD && CharacterHUD->CharacterUI == nullptr)
    {
        CharacterHUD->AddCharacterUI();
        if (CharacterHUD->CharacterUI == nullptr)
            return;
    }

    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD; // 캐릭터 HUD가 nullptr인 경우 현재 플레이어의 HUD를 캐스팅합니다.

    if (CharacterHUD && CharacterHUD->CharacterUI && CharacterHUD->CharacterUI->CarriedAmmoAmount) // 캐릭터 HUD와 캐릭터 UI가 유효한 경우
    {
        FString CarriedAmmoText = FString::Printf(TEXT("%d"), Ammo); // 총알 개수 텍스트 포맷팅
        CharacterHUD->CharacterUI->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoText)); // 총알 개수 텍스트를 설정합니다.
    }
}

void ABasePC::SetHUDWeaponIcon(UTexture2D* WeaponIcon)
{
    if (CharacterHUD && CharacterHUD->CharacterUI == nullptr)
    {
        CharacterHUD->AddCharacterUI();
        if (CharacterHUD->CharacterUI == nullptr)
            return;
    }

    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD; // 캐릭터 HUD가 nullptr인 경우 현재 플레이어의 HUD를 캐스팅합니다.

    if (CharacterHUD && CharacterHUD->CharacterUI && CharacterHUD->CharacterUI->WeaponIcon) // 캐릭터 HUD와 캐릭터 UI가 유효한 경우
    {
        CharacterHUD->CharacterUI->WeaponIcon->SetBrushFromTexture(WeaponIcon, true); // 무기 아이콘을 설정합니다.
        CharacterHUD->CharacterUI->WeaponIcon->SetVisibility(ESlateVisibility::Visible); // 무기 아이콘을 보이도록 설정합니다.
    }
}

void ABasePC::SetHUDWeponIconClear()
{
    if (CharacterHUD && CharacterHUD->CharacterUI == nullptr)
    {
        CharacterHUD->AddCharacterUI();
        if (CharacterHUD->CharacterUI == nullptr)
            return;
    }

    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD; // 캐릭터 HUD가 nullptr인 경우 현재 플레이어의 HUD를 캐스팅합니다.

    if (CharacterHUD && CharacterHUD->CharacterUI && CharacterHUD->CharacterUI->WeaponIcon) // 캐릭터 HUD와 캐릭터 UI가 유효한 경우
    {
        CharacterHUD->CharacterUI->WeaponIcon->SetVisibility(ESlateVisibility::Collapsed);
    }
}

void ABasePC::SetHUDMatchCountDown(float CountDownTime)
{
    if (CharacterHUD && CharacterHUD->CharacterUI == nullptr)
    {
        CharacterHUD->AddCharacterUI();
        if (CharacterHUD->CharacterUI == nullptr)
            return;
    }

    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD; // 캐릭터 HUD가 nullptr인 경우 현재 플레이어의 HUD를 캐스팅합니다.

    if (CharacterHUD && CharacterHUD->CharacterUI && CharacterHUD->CharacterUI->MatchCountDownText) // 캐릭터 HUD와 캐릭터 UI가 유효한 경우
    {
        if (CountDownTime < 0.f)
        {
            CharacterHUD->CharacterUI->MatchCountDownText->SetText(FText()); // 카운트다운 시간이 0보다 작으면 텍스트를 비웁니다.
            return; // 함수 종료
        }
        int32 Minutes = FMath::FloorToInt(CountDownTime / 60.0f); // 분 계산
        int32 Seconds = FMath::FloorToInt(CountDownTime) % 60; // 초 계산

        FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds); // 카운트다운 텍스트 포맷팅
        CharacterHUD->CharacterUI->MatchCountDownText->SetText(FText::FromString(CountDownText)); // 카운트다운 텍스트를 설정합니다.
    }
}

void ABasePC::SetHUDGrenade(int32 Grenade)
{
    if (CharacterHUD && CharacterHUD->CharacterUI == nullptr)
    {
        CharacterHUD->AddCharacterUI();
        if (CharacterHUD->CharacterUI == nullptr)
            return;
    }

    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD; // 캐릭터 HUD가 nullptr인 경우 현재 플레이어의 HUD를 캐스팅합니다.

    if (CharacterHUD && CharacterHUD->CharacterUI && CharacterHUD->CharacterUI->GrenadeText) // 캐릭터 HUD와 캐릭터 UI가 유효한 경우
    {
        FString GrenadeText = FString::Printf(TEXT("%d"), Grenade); // 총알 개수 텍스트 포맷팅
        CharacterHUD->CharacterUI->GrenadeText->SetText(FText::FromString(GrenadeText)); // 총알 개수 텍스트를 설정합니다.
    }
}

void ABasePC::SetHUDDefeat(int32 Defeat)
{
    if (CharacterHUD && CharacterHUD->CharacterUI == nullptr)
    {
        CharacterHUD->AddCharacterUI();
        if (CharacterHUD->CharacterUI == nullptr)
            return;
    }

    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD; // 캐릭터 HUD가 nullptr인 경우 현재 플레이어의 HUD를 캐스팅합니다.

    if (CharacterHUD && CharacterHUD->CharacterUI && CharacterHUD->CharacterUI->DefeatAmount) // 캐릭터 HUD와 캐릭터 UI가 유효한 경우
    {
        FString DefeatText = FString::Printf(TEXT("%d"), Defeat); // 죽은 횟수 텍스트 포맷팅
        CharacterHUD->CharacterUI->DefeatAmount->SetText(FText::FromString(DefeatText)); // 죽은 횟수 텍스트를 설정합니다.
    }

    SetHUDWeponIconClear(); // 무기 아이콘을 숨깁니다.
}
// 누구한테 죽었는지 HUD에 표시하는 함수
void ABasePC::SetHUDKilledBy(FString KilledBy)
{
    if (CharacterHUD && CharacterHUD->CharacterUI == nullptr)
    {
        CharacterHUD->AddCharacterUI();
        if (CharacterHUD->CharacterUI == nullptr)
            return;
    }

    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD; // 캐릭터 HUD가 nullptr인 경우 현재 플레이어의 HUD를 캐스팅합니다.

    if (CharacterHUD && CharacterHUD->CharacterUI && CharacterHUD->CharacterUI->KilledByText && CharacterHUD->CharacterUI->DeathMessageText )  // 캐릭터 HUD와 캐릭터 UI가 유효한 경우
    {
        UE_LOG(LogTemp, Error, TEXT("SetHUDKilledBy called with KilledBy: %s"), *KilledBy); // 디버그용 로그 출력
        CharacterHUD->CharacterUI->KilledByText->SetText(FText::FromString(KilledBy)); // 죽은 메시지를 설정합니다.
        CharacterHUD->CharacterUI->KilledByText->SetVisibility(ESlateVisibility::Visible); // 죽은 메시지 텍스트를 보이도록 설정합니다.
        CharacterHUD->CharacterUI->DeathMessageText->SetVisibility(ESlateVisibility::Visible); // 죽은 메시지 텍스트를 보이도록 설정합니다.
    }
}
// 죽은 메시지를 숨기는 함수
void ABasePC::HideHUDKilledBy()
{
    if (CharacterHUD && CharacterHUD->CharacterUI == nullptr)
    {
        CharacterHUD->AddCharacterUI();
        if (CharacterHUD->CharacterUI == nullptr)
            return;
    }

    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD; // 캐릭터 HUD가 nullptr인 경우 현재 플레이어의 HUD를 캐스팅합니다.

    if (CharacterHUD && CharacterHUD->CharacterUI && CharacterHUD->CharacterUI->KilledByText && CharacterHUD->CharacterUI->DeathMessageText) // 캐릭터 HUD와 캐릭터 UI가 유효한 경우
    {
        CharacterHUD->CharacterUI->KilledByText->SetVisibility(ESlateVisibility::Collapsed); // 죽은 메시지 텍스트를 숨깁니다.
        CharacterHUD->CharacterUI->DeathMessageText->SetVisibility(ESlateVisibility::Collapsed); // 죽은 메시지 텍스트를 숨깁니다.
    }
}

// 아나운스 먼트에서 시간 보여주고, 쿨다운 상테에서 0초일때 위젯 초기화 함!!!
void ABasePC::SetHUDAnnouncementCountDown(float CountDownTime)
{
    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD; // 캐릭터 HUD가 nullptr인 경우 현재 플레이어의 HUD를 캐스팅합니다.

    if (CharacterHUD && CharacterHUD->Announcement && CharacterHUD->Announcement->WarmUpTimeText) // 캐릭터 HUD와 캐릭터 UI가 유효한 경우
    {
        if (CountDownTime < 0.f)
        {
            CharacterHUD->Announcement->WarmUpTimeText->SetText(FText()); // 카운트다운 시간이 0보다 작으면 텍스트를 비웁니다.
            if (MatchState == MatchState::Cooldown)
            {
                CharacterHUD->Announcement->AnnouncementText->SetText(FText());
                CharacterHUD->Announcement->InfoText->SetText(FText());
                CharacterHUD->Announcement->RemoveFromParent();
                CharacterHUD->Announcement = nullptr;
                if (ChatBox)
                {
                    ChatBox->RemoveFromParent(); // 채팅 박스를 제거합니다.
					ChatBox = nullptr; // 채팅 박스 포인터를 nullptr로 설정합니다.
                }
            }
            return; // 함수 종료
        }
        int32 Minutes = FMath::FloorToInt(CountDownTime / 60.0f); // 분 계산
        int32 Seconds = FMath::FloorToInt(CountDownTime) % 60; // 초 계산
        FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds); // 카운트다운 텍스트 포맷팅
        CharacterHUD->Announcement->WarmUpTimeText->SetText(FText::FromString(CountDownText)); // 카운트다운 텍스트를 설정합니다.
    }
}

// ------------------------------------------------------------------------------------------------------------------허드 세팅 관련 함수들ㅗ

// ------------------------------------------------------------------------------------------------------------------시간 동기화 관련 함수들ㅜ

// 틱마다 실해되면서 일정 시간마다 ServerRequestServerTime()를 호출하는 함수
void ABasePC::CheckTimeSync(float DeltaTime)
{
    LastTimeSync += DeltaTime; // 마지막 시간 동기화 시간을 업데이트합니다.
    if (IsLocalController() && LastTimeSync >= TimeSyncFrequency) // 로컬 컨트롤러인 경우
    {
        LastTimeSync = 0.f; // 마지막 시간 동기화 시간을 초기화합니다.
        ServerRequestServerTime(GetWorld()->GetTimeSeconds()); // 서버에 시간 요청을 보냅니다.
    }
}

// 현재 서버 시간을 가져오고, 클라이언트는 따로 ClientReportServerTime()를 통해 서버 시간을 저장함
void ABasePC::ServerRequestServerTime_Implementation(float TimeOfClientRequest) // 클라 요청 -> 서버 실행
{
    float CurrentServerTIme = GetWorld()->GetTimeSeconds(); // 서버의 현재 시간 가져오기
    ClientReportServerTime(TimeOfClientRequest, CurrentServerTIme); // 서버 시간 보고 함수 호출
}

// 서버에 요청이 걸리는 시간과 돌아오는 시간을 계산하여 서버의 시간을 계산후 ClientServerDeltaTime에 값을 저장   
void ABasePC::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedRequest) // 서버 응답 -> 클라 실행
{
    float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest; // 왕복 시간 계산
	SingleTripTime = (RoundTripTime * 0.5f); // 단일 왕복 시간 계산
    float CurrentServerTime = TimeServerReceivedRequest + (RoundTripTime / 2.0f); // 서버 시간 계산

    ClientServerDeltaTime = CurrentServerTime - GetWorld()->GetTimeSeconds(); // 클라이언트와 서버 간의 시간 차이 계산
}

// 서버 시간을 가져오는 함수. 서버면 그냥 서버 시간을 반환하고, 클라이언트면 서버와의 ClientServerDeltaTime()값을 가져와서 서버 시간을 반환합니다.
float ABasePC::GetServerTime()
{
    if( HasAuthority() ) // 서버 권한이 있는 경우
    {
        return GetWorld()->GetTimeSeconds(); // 서버 시간을 반환합니다.
    }
    else
    {
        return GetWorld()->GetTimeSeconds() + ClientServerDeltaTime; // 서버 시간을 반환합니다.
    }
}

// ------------------------------------------------------------------------------------------------------------------시간 동기화 관련 함수들ㅗ

//--------------------------------------------------------------------------------------------------------------- 매치 상태 관련 함수들ㅜ

// 매치 상태를 변경하는 함수.
void ABasePC::OnMatchStateSet(FName State)
{
    MatchState = State; // 매치 상태를 설정합니다. -> OnRep_MatchState() 함수가 호출됩니다.

    if( MatchState == MatchState::InProgress ) // 매치 상태가 InProgress인 경우
    {
        HandleMatchHasStarted(); // 매치가 시작되었을 때 처리하는 함수를 호출합니다.
    }
    else if( MatchState == MatchState::Cooldown ) // 매치 상태가 Cooldown인 경우
    {
        HandleCoolDown(); // 쿨다운 상태를 처리하는 함수를 호출합니다.
    }
    else if( MatchState == MatchState::WaitingToStart ) // 매치 상태가 WaitingToStart인 경우
    {
        CharacterHUD = Cast<ACharacterHUD>(GetHUD()); // 현재 플레이어의 HUD를 캐릭터 HUD로 캐스팅합니다.
        if( CharacterHUD )
        {
            CharacterHUD->AddAnnouncement(); // 시작전 안내 위젯을 추가합니다.
            if( CharacterHUD->Announcement )
            {
                CharacterHUD->Announcement->SetVisibility(ESlateVisibility::Visible); // 시작전 안내 위젯을 보이도록 설정합니다
            }
        }
        AddChatBox(); // 채팅 박스를 추가합니다.
    }
}

// 매치 상태 변수 복제시 호출. 클라 쪽
void ABasePC::OnRep_MatchState()
{
    if( MatchState == MatchState::InProgress ) // 매치 상태가 InProgress인 경우
    {
        HandleMatchHasStarted(); // 매치가 시작되었을 때 처리하는 함수를 호출합니다.
    }
    else if( MatchState == MatchState::Cooldown ) // 매치 상태가 Cooldown인 경우
    {
        HandleCoolDown(); // 쿨다운 상태를 처리하는 함수를 호출합니다.
    }
    else if( MatchState == MatchState::WaitingToStart ) // 매치 상태가 WaitingToStart인 경우
    {
        if( !HasAuthority() )
        {
            ServerCheckMatchState(); // 서버에서 매치 상태를 확인합니다.
        }
        CharacterHUD = Cast<ACharacterHUD>(GetHUD()); // 현재 플레이어의 HUD를 캐릭터 HUD로 캐스팅합니다.
        if( CharacterHUD )
        {
            CharacterHUD->AddAnnouncement(); // 시작전 안내 위젯을 추가합니다.
            if( CharacterHUD->Announcement )
            {
                CharacterHUD->Announcement->SetVisibility(ESlateVisibility::Visible); // 시작전 안내 위젯을 보이도록 설정합니다
                //UE_LOG(LogTemp, Warning, TEXT("OnRep_MatchState: MatchState is WaitingToStart.")); // 디버그용 로그 출력
            }
        }
        AddChatBox(); // 채팅 박스를 추가합니다.
    }
}

// 매치가 InProgress이면 실행. 캐릭터 허트를 게임 상태로 변경
void ABasePC::HandleMatchHasStarted()
{
    if( MatchState == MatchState::InProgress )
    {
        CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD; // 캐릭터 HUD가 nullptr인 경우 현재 플레이어의 HUD를 캐스팅합니다.
        if( CharacterHUD )
        {
            if( CharacterHUD->Announcement ) // 시작전 안내 위젯이 있는 경우
            {
                // 이 코드 뺴면 안됨!!!!!!!!!!! 다른 노트북이나 pc 에서 하면 이상하게 이게 남아있음. 에디터에서는 안그러는데;;;;
                if (CharacterHUD->Announcement->AnnouncementText)
                    CharacterHUD->Announcement->AnnouncementText->SetText(FText());
                if (CharacterHUD->Announcement->InfoText)
                    CharacterHUD->Announcement->InfoText->SetText(FText());
                if (CharacterHUD->Announcement->WarmUpTimeText)
                    CharacterHUD->Announcement->WarmUpTimeText->SetText(FText());

                CharacterHUD->Announcement->SetVisibility(ESlateVisibility::Collapsed); // 시작전 안내 위젯을 숨깁니다.
            }
            CharacterHUD->AddCharacterUI(); // 캐릭터 오버레이를 추가합니다.

            CharacterHUD->CharacterUI->SetVisibility(ESlateVisibility::Visible); // 캐릭터 UI를 보이도록 설정합니다.

			if (ChatBox) ChatBox->SetVisibility(ESlateVisibility::Visible); // 채팅 박스를 보이도록 설정합니다.
        }
    }
}

// 매치 종료시 아나운스 먼트 캐릭터 UI를 보여주면서 게임 결과를 보여주는 함수
void ABasePC::HandleCoolDown()
{
    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD; // 캐릭터 HUD가 nullptr인 경우 현재 플레이어의 HUD를 캐스팅합니다.
    if( CharacterHUD )
    {
        if( CharacterHUD->CharacterUI ) 
        {
            CharacterHUD->CharacterUI->RemoveFromParent(); // 캐릭터 UI를 제거합니다.
        }
		
        if(  CharacterHUD->Announcement && CharacterHUD->Announcement->AnnouncementText && CharacterHUD->Announcement->InfoText ) // 시작전 안내 위젯이 있는 경우
        {
            CharacterHUD->Announcement->SetVisibility(ESlateVisibility::Visible); // 시작전 안내 위젯을 보이도록 설정합니다.

            FString AnnouncementText("New Match Start In:");
            CharacterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText)); // 안내 텍스트를 설정합니다.

            ABaseGameState* LocalGameState = Cast<ABaseGameState>(UGameplayStatics::GetGameState(this)); // 현재 게임 상태를 ABaseGameState로 캐스팅합니다.
            ACharacterState* LocalPlayerState = GetPlayerState<ACharacterState>();
  
            if( LocalGameState && LocalPlayerState )
            {
                TArray<ACharacterState* > TopPlayers = LocalGameState->TopScoreingPlayers; // 게임 상태에서 모든 플레이어 컨트롤러를 가져옵니다.
                FString InfoText;
                if( TopPlayers.Num() == 0 )
                {
                    InfoText = FString("No Winner"); // 플레이어가 없는 경우 정보 텍스트를 설정합니다.
                    CharacterHUD->Announcement->InfoText->SetText(FText::FromString(InfoText)); // 정보 텍스트를 설정합니다.
                }
                else if( TopPlayers.Num() == 1 && TopPlayers[0] == LocalPlayerState ) // 플레이어가 1명이고 그 플레이어가 현재 플레이어인 경우
                {
                    InfoText = FString::Printf(TEXT("You are the winner!")); // 플레이어가 승리한 경우 정보 텍스트를 설정합니다.
                    CharacterHUD->Announcement->InfoText->SetText(FText::FromString(InfoText)); // 정보 텍스트를 설정합니다.
                }
                else if( TopPlayers.Num() == 1 && TopPlayers[0] != LocalPlayerState ) // 플레이어가 1명이고 그 플레이어가 현재 플레이어가 아닌 경우
                {
                    InfoText = FString::Printf(TEXT("%s is the winner!"), *TopPlayers[0]->GetPlayerName()); // 플레이어가 승리한 경우 정보 텍스트를 설정합니다.
                    CharacterHUD->Announcement->InfoText->SetText(FText::FromString(InfoText)); // 정보 텍스트를 설정합니다.
                }
                else if( TopPlayers.Num() > 1 )
                {
                    InfoText = FString::Printf(TEXT("Players tied for the win:\n"));
                    for( auto TiedPlayer : TopPlayers ) // 모든 플레이어를 순회합니다.
                    {
                        InfoText.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName())); // 플레이어 이름을 정보 텍스트에 추가합니다.
                    }
                }
                CharacterHUD->Announcement->InfoText->SetText(FText::FromString(InfoText)); // 정보 텍스트를 설정합니다.
            }
        }
    }

    ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(GetPawn()); // 현재 조종 중인 폰을 ABaseCharacter로 캐스팅합니다.
    if( BaseCharacter )
    {
        BaseCharacter->bDisableGamePlay = true; // 게임 플레이를 비활성화합니다.
        if( BaseCharacter->GetCombatComponent() ) // 전투 컴포넌트가 있는 경우
        {
            BaseCharacter->GetCombatComponent()->FireButtonPressed(false); // 전투 컴포넌트의 FireButtonPressed를 false로 설정합니다.
        }
    }
}

// --------------------------------------------------------------------------------------------------------------- 매치 상태 관련 함수들ㅗ


// ---------------------------------------------------------------------------------------------------------------게임 모드에서 게임 상태 받아오기 함수들ㅜ

//서버가 게임모드한테 워밍업 시간, 매치 시간, 레벨 시작 시간, 쿨다운 시간 
//매치 상태를 받아와서 저장후 ClientJoinMidGame()로 클라한테 전달
void ABasePC::ServerCheckMatchState_Implementation()
{
    AMainGameMode* MainGameMode = Cast<AMainGameMode>( UGameplayStatics::GetGameMode(this) ); // 현재 게임 모드를 AMainGameMode로 캐스팅합니다.
    if( MainGameMode )
    {
        WarmUpTime = MainGameMode->WarmupTime; // 게임 모드에서 워밍업 시간을 가져옵니다.
        MatchTime = MainGameMode->MatchTime; // 게임 모드에서 매치 시간을 가져옵니다.
        LevelStartingTime = MainGameMode->LevelStartingTime; // 게임 모드에서 레벨 시작 시간을 가져옵니다.
        CooldownTime = MainGameMode->CooldownTime; // 게임 모드에서 쿨다운 시간을 가져옵니다.
        MatchState = MainGameMode->GetMatchState(); // 게임 모드에서 매치 상태를 가져옵니다.
        ClientJoinMidGame(MatchState, WarmUpTime, MatchTime, LevelStartingTime, CooldownTime); // 클라이언트에 중간 게임 참여 정보를 보냅니다.
    }
}

// 클라이언트가 중간에 게임에 참여할 때 호출되는 함수 워밍업 시간, 매치 시간, 
// 레벨 시작 시간, 쿨다운 시간, 매치 상태를 서버한테 받아서 설정
void ABasePC::ClientJoinMidGame_Implementation( FName StateOfMatch, float WarmUp, float Match, 
    float LevelStarting, float CoolTime ) // 클라이언트가 중간에 게임에 참여할 때 호출되는 함수
{
    WarmUpTime = WarmUp; // 워밍업 시간 설정
    MatchTime = Match; // 매치 시간 설정
    LevelStartingTime = LevelStarting; // 레벨 시작 시간 설정
    CooldownTime = CoolTime; // 게임 모드에서 쿨다운 시간을 가져옵니다.
    MatchState = StateOfMatch; // 매치 상태 설정
    OnMatchStateSet(MatchState); // 매치 상태 설정 함수 호출

    if( CharacterHUD && MatchState == MatchState::WaitingToStart ) // 캐릭터 HUD가 유효하고 매치 상태가 WaitingToStart인 경우
    {
        CharacterHUD->AddAnnouncement(); // 시작전 안내 위젯을 추가합니다.
    }
}

// --------------------------------------------------------------------------------------------------------------게임 모드에서 게임 상태 받아오기 함수들ㅗ

// ----------------------------------------------------------------------------------------------------------------- 핑 관련 함수들 ㅜ

// 틱마다 핑을 확인해서 경고할지 중지할지 결정하는 함수
void ABasePC::CheckPing(float DeltaTime)
{
	if (HasAuthority()) return; // 서버 권한이 있는 경우 함수 종료

    HighPingRunningTime += DeltaTime; // 핑 경고 실행 시간 증가
    if (HighPingRunningTime >= CheckPingFrequency)
    {
        PlayerState = PlayerState == nullptr ? GetPlayerState<ACharacterState>() : PlayerState; // 플레이어 상태가 nullptr인 경우 현재 플레이어의 게임 상태를 가져옵니다.
        if (PlayerState)
        {
            UE_LOG(LogTemp, Warning, TEXT("PlayerState->GetPingInMilliseconds() = %d"), FMath::RoundToInt(PlayerState->GetPingInMilliseconds()));
            if (PlayerState->GetPingInMilliseconds() > HighPingThreshold)
            {
                HighPingWarning();
                PingAnimationRunningTime = 0.f; // 핑 애니메이션 실행 시간 초기화
				ServerReportPingStaus(true); // 서버에 핑 상태를 보고합니다.
            }
            else
            {
				ServerReportPingStaus(false); // 서버에 핑 상태를 보고합니다.
            }
        }
        HighPingRunningTime = 0.f; // 핑 경고 실행 시간 초기화
    }
    if (CharacterHUD && CharacterHUD->CharacterUI && CharacterHUD->CharacterUI->WifiAnim
        && CharacterHUD->CharacterUI->IsAnimationPlaying(CharacterHUD->CharacterUI->WifiAnim))
    {
        PingAnimationRunningTime += DeltaTime; // 핑 애니메이션 실행 시간 증가
        if (PingAnimationRunningTime >= HighPingDuration)
        {
            StopHighPingWarning();
        }
    }
}

// 핑 상태 를 서버에 보고하는 함수
void ABasePC::ServerReportPingStaus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing); // 핑 상태를 브로드캐스트합니다.
}

// 핑 경고를 허드에 표시하는 함수
void ABasePC::HighPingWarning() 
{
    if (CharacterHUD && CharacterHUD->CharacterUI == nullptr)
    {
        CharacterHUD->AddCharacterUI();
        if (CharacterHUD->CharacterUI == nullptr)
            return;
    }
    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD; // 캐릭터 HUD가 nullptr인 경우 현재 플레이어의 HUD를 캐스팅합니다.

    //if( CharacterHUD ) UE_LOG(LogTemp, Warning, TEXT("CharacterHUD is valid.")); // 캐릭터 HUD가 유효한 경우 로그 출력
    //if( CharacterHUD && CharacterHUD->CharacterUI ) UE_LOG(LogTemp, Warning, TEXT("CharacterUI is valid.")); // 캐릭터 UI가 유효한 경우 로그 출력

    if (CharacterHUD && CharacterHUD->CharacterUI
        && CharacterHUD->CharacterUI->WifiImage && CharacterHUD->CharacterUI->WifiAnim) // 캐릭터 HUD와 캐릭터 UI가 유효한 경우
    {
        CharacterHUD->CharacterUI->WifiImage->SetOpacity(1.0f);
        CharacterHUD->CharacterUI->PlayAnimation(
            CharacterHUD->CharacterUI->WifiAnim,
            0.f, // 시작 시간
            5
        );
    }
}

// 핑 경고를 허드에서 중지하는 함수
void ABasePC::StopHighPingWarning() // 핑 경고 중지 함수
{
    if (CharacterHUD && CharacterHUD->CharacterUI == nullptr)
    {
        CharacterHUD->AddCharacterUI();
        if (CharacterHUD->CharacterUI == nullptr)
            return;
    }

    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD; // 캐릭터 HUD가 nullptr인 경우 현재 플레이어의 HUD를 캐스팅합니다.

    //if( CharacterHUD ) UE_LOG(LogTemp, Warning, TEXT("CharacterHUD is valid.")); // 캐릭터 HUD가 유효한 경우 로그 출력
    //if( CharacterHUD && CharacterHUD->CharacterUI ) UE_LOG(LogTemp, Warning, TEXT("CharacterUI is valid.")); // 캐릭터 UI가 유효한 경우 로그 출력

    if (CharacterHUD && CharacterHUD->CharacterUI
        && CharacterHUD->CharacterUI->WifiImage && CharacterHUD->CharacterUI->WifiAnim) // 캐릭터 HUD와 캐릭터 UI가 유효한 경우
    {
        CharacterHUD->CharacterUI->WifiImage->SetOpacity(0.0f); // WiFi 이미지를 보이도록 설정
		if (CharacterHUD->CharacterUI->IsAnimationPlaying(CharacterHUD->CharacterUI->WifiAnim) ) // WiFi 애니메이션이 재생 중인 경우
        {
            CharacterHUD->CharacterUI->StopAnimation(CharacterHUD->CharacterUI->WifiAnim); // WiFi 애니메이션을 중지합니다.
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------- 핑 관련 함수들 ㅗ

//-------------------------------------------------------------------------------------------------------------------------- 킬로그 관련 ㅜ

void ABasePC::CleintElimAnnouncement_Implementation(ACharacterState* Attacker, ACharacterState* Victim)
{
    ACharacterState* Self = GetPlayerState<ACharacterState>();
    if (Attacker && Victim && Self)
    {
		CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD; 
        // 캐릭터 HUD가 nullptr인 경우 현재 플레이어의 HUD를 캐스팅합니다.
        if (CharacterHUD)
        {
            if (Attacker == Self && Victim != Self)
            {
				CharacterHUD->AddElimAnnouncement("You", Victim->GetPlayerName()); // 자신이 제거한 경우
				return; // 함수 종료
            }
            if (Victim == Self && Attacker != Self)
            {
				CharacterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "You"); // 자신이 제거된 경우
                return;
            }
            if (Attacker == Victim && Attacker == Self) // 내가 멍청이 
            {
                CharacterHUD->AddElimAnnouncement("You", "YourSelf"); // 자신이 제거된 경우
                return;
            }
            if (Attacker == Victim && Attacker != Self) // 남이 멍청이짓
            {
                CharacterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "Self"); // 자신이 제거된 경우
                return;
            }
			CharacterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName()); // 일반적인 제거 알림
        }
    }
}

void ABasePC::BroadcastElim(ACharacterState* Attacker, ACharacterState* Victim)
{
	CleintElimAnnouncement(Attacker, Victim); // 클라이언트에게 제거 알림을 보냅니다.
}

//------------------------------------------------------------------------------------------------------------------------킬로그 관련 ㅗ

//--------------------------------------------------------------------------------------------------------------------------- 임 종료 관련 함수들ㅜ

// 메인 메뉴 버튼 누르면 실행되는 함수
void ABasePC::ShowReturnToMainMenu()
{
    if (ReturnToMainMenuWidget == nullptr) return;
    if (ReturnToMainMenu == nullptr)
    {
        ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuWidget); // ReturnToMainMenu 위젯을 생성합니다.
    }
    if (ReturnToMainMenu)
    {
        bReturnToMainMenuOpen = !bReturnToMainMenuOpen; // ReturnToMainMenu 열림 상태를 토글합니다.
        if (bReturnToMainMenuOpen == true)
        {
            ReturnToMainMenu->MenuSetUp(); // ReturnToMainMenu 위젯을 설정합니다.
        }
        else
        {
            ReturnToMainMenu->MenuTeardown(); // ReturnToMainMenu 위젯을 제거합니다.
        }
    }
}

// 게임을 떠날 때 호출되는 함수
void ABasePC::PawnLeavingGame()
{
    ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(GetPawn());
    if (BaseCharacter && !BaseCharacter->bLeftGame)
    {
        UE_LOG(LogTemp, Warning, TEXT("PawnLeavingGame"));
        BaseCharacter->Eliminate(true);
    }

    Super::PawnLeavingGame();
}

// --------------------------------------------------------------------------------------------------------------------------- 게임 종료 관련 함수들 ㅗ

//--------------------------------------------------------------------------------------------------------------------------- 채팅 관련 ㅜ

void ABasePC::AddChatBox()
{
    // 로컬 컨트롤러에서만 채팅 박스 생성
    if (IsLocalController() && ChatBoxClass )
    {
        ChatBox = CreateWidget<UChatBox>(this, ChatBoxClass);
		if (ChatBox && ChatBox->ChatInputBox ) // 채팅 박스가 유효하고 아직 뷰포트에 추가되지 않은 경우
        {
            ChatBox->AddToViewport(100);
            ChatBox->ChatInputBox->SetHintText(FText::FromString("Text.....")); // 채팅 입력 박스에 힌트 텍스트 설정
            ChatBox->SetVisibility(ESlateVisibility::Visible); // 기다리는 동안은 채팅 숨기기
            ChatBox->ChatInputBox->RevertTextOnEscape = true; // 채팅 입력 박스에서 Escape 키를 누르면 텍스트가 되돌려지도록 설정합니다.
            ChatBox->ChatInputBox->OnTextCommitted.AddDynamic(this, &ABasePC::OnChatCommitted);
        }
    }
}

void ABasePC::StartChat()
{
	bIsChatting = !bIsChatting; // 채팅 상태를 토글합니다.

    if (bIsChatting)
    {
        ChatBox->SetVisibility(ESlateVisibility::Visible); // 항상 보이게
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(ChatBox->ChatInputBox->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // 마우스 잠금 모드를 설정합니다.
        SetInputMode(InputMode);
        SetShowMouseCursor(true);
    }
    else
    {
        FInputModeGameOnly InputModeGameOnly;
        SetInputMode(InputModeGameOnly);
        SetShowMouseCursor(false); // 마우스 커서 숨기기
    }
}

void ABasePC::OnChatCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    if (CommitMethod != ETextCommit::OnEnter) return;
    
    FString PlayerName{ "" };

    APlayerState* TempPlayerState = GetPlayerState<APlayerState>();
    if (TempPlayerState)
    {
        PlayerName = TempPlayerState->GetPlayerName();
    }
	ServerChatCommitted(Text, PlayerName); // 서버에 채팅 메시지를 보냅니다.

    // 채팅 입력 후 다시 입력창에 포커스 & 마우스 커서 유지
    if (ChatBox && ChatBox->ChatInputBox)
    {
        FInputModeGameAndUI InputMode;
        InputMode.SetWidgetToFocus(ChatBox->ChatInputBox->TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        SetInputMode(InputMode);
        SetShowMouseCursor(true);
    }
}

void ABasePC::ServerChatCommitted_Implementation(const FText& Text, const FString& PlayerName)
{
    AMainGameMode* GameMode = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this));
    if (GameMode)
    {
        GameMode->SendChatMsg(Text, PlayerName);
    }
}

void ABasePC::ClientChatCommitted_Implementation(const FText& Text, const FString& PlayerName)
{
    if (ChatBox)
    {
        ChatBox->OnTextCommitted(Text, PlayerName);
    }
}

// 마우스 클릭 시 채팅 박스 외부를 클릭했을 때 채팅 종료
void ABasePC::OnMouseClickAnywhere()
{
    if (bIsChatting)
    {
        // 챗 박스 외부 클릭 시 채팅 종료
        bIsChatting = false;
        FInputModeGameOnly InputModeGameOnly;
        SetInputMode(InputModeGameOnly);
        SetShowMouseCursor(false);
    }
}

//--------------------------------------------------------------------------------------------------------------------------- 채팅 관련 ㅗ


