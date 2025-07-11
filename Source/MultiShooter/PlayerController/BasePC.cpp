#include "BasePC.h"
#include "MultiShooter/HUD/CharacterHUD.h"
#include "MultiShooter/HUD/CharacterUI.h"
#include "Components/Progressbar.h"
#include "Components/TextBlock.h"
#include "MultiShooter/Character/BaseCharacter.h"
#include "Components/Image.h"
#include "Net/UnrealNetwork.h" // 네트워크 관련 헤더 파일
#include "MultiShooter/GameMode/MainGameMode.h"
#include "MultiShooter/HUD/Announcement.h" // 시작전 안내 위젯 헤더 파일
#include "Kismet/GameplayStatics.h" // 게임 관련 유틸리티 함수 헤더 파일
#include "MultiShooter/CharacterComponent/CombatComponent.h" // 전투 컴포넌트 헤더 파일
#include "MultiShooter/GameState/BaseGameState.h" // 게임 상태 헤더 파일
#include "MultiShooter/CharacterState/CharacterState.h" // 플레이어 상태 헤더 파일

void ABasePC::BeginPlay()
{
    Super::BeginPlay();

    CharacterHUD = Cast<ACharacterHUD>(GetHUD()); // 현재 플레이어의 HUD를 캐릭터 HUD로 캐스팅합니다.
    ServerCheckMatchState(); // 서버에서 매치 상태를 확인합니다.
}

void ABasePC::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime); // 부모 클래스의 Tick 함수를 호출합니다.

    SetHUDTime(); // HUD의 시간을 설정합니다.

    CheckTimeSync(DeltaTime); // 시간 동기화 확인 함수 호출

}

void ABasePC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps); // 부모 클래스의 GetLifetimeReplicatedProps 함수를 호출합니다.

    DOREPLIFETIME(ABasePC, MatchState); // MatchState 변수를 복제할 수 있도록 설정합니다.
}

void ABasePC::OnPossess(APawn* InPawn) // 초기 UI 상태 설정
{
    Super::OnPossess(InPawn); // 부모 클래스의 OnPossess 함수를 호출합니다.

    ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(InPawn); // 조종하는 폰을 ABaseCharacter로 캐스팅합니다.
    if (BaseCharacter)
    {
        SetHUDHealth(BaseCharacter->GetHealth(), BaseCharacter->GetMaxHealth()); // 캐릭터의 체력을 HUD에 설정합니다.
       
        SetHUDGrenade(BaseCharacter->GetCombatComponent()->GetGrenade());
    }
    
}

void ABasePC::ReceivedPlayer()
{
    Super::ReceivedPlayer(); // 부모 클래스의 ReceivedPlayer 함수를 호출합니다.

    if( IsLocalController() )
    {
        ServerRequestServerTime(GetWorld()->GetTimeSeconds()); // 서버에 시간 요청을 보냅니다.
    }
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
        UE_LOG(LogTemp, Warning, TEXT("SetHUDKilledBy called with KilledBy: %s"), *KilledBy); // 디버그용 로그 출력
        CharacterHUD->CharacterUI->KilledByText->SetText(FText::FromString(KilledBy)); // 죽은 메시지를 설정합니다.
        CharacterHUD->CharacterUI->KilledByText->SetVisibility(ESlateVisibility::Visible); // 죽은 메시지 텍스트를 보이도록 설정합니다.
        CharacterHUD->CharacterUI->DeathMessageText->SetVisibility(ESlateVisibility::Visible); // 죽은 메시지 텍스트를 보이도록 설정합니다.
    }
}

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
        if( CountDownTime < 0.f )
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

void ABasePC::SetHUDAnnouncementCountDown(float CountDownTime)
{
    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD; // 캐릭터 HUD가 nullptr인 경우 현재 플레이어의 HUD를 캐스팅합니다.

    if (CharacterHUD && CharacterHUD->Announcement && CharacterHUD->Announcement->WarmUpTimeText) // 캐릭터 HUD와 캐릭터 UI가 유효한 경우
    {
        if( CountDownTime < 0.f )
        {
            CharacterHUD->Announcement->WarmUpTimeText->SetText(FText()); // 카운트다운 시간이 0보다 작으면 텍스트를 비웁니다.
            return; // 함수 종료
        }
        int32 Minutes = FMath::FloorToInt(CountDownTime / 60.0f); // 분 계산
        int32 Seconds = FMath::FloorToInt(CountDownTime) % 60; // 초 계산
        FString CountDownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds); // 카운트다운 텍스트 포맷팅
        CharacterHUD->Announcement->WarmUpTimeText->SetText(FText::FromString(CountDownText)); // 카운트다운 텍스트를 설정합니다.
    }
}

void ABasePC::SetHUDTime()
{
    AMainGameMode* LocalGameMode = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this));

    float TimeLeft = 0.f;
    if( MatchState == MatchState::WaitingToStart ) TimeLeft = WarmUpTime - GetServerTime() + LevelStartingTime;
    else if( MatchState == MatchState::InProgress ) TimeLeft = WarmUpTime + MatchTime - GetServerTime() + LevelStartingTime;
    else if( MatchState == MatchState::Cooldown ) TimeLeft = CooldownTime + WarmUpTime + MatchTime - GetServerTime() + LevelStartingTime;
    
    uint32 SecondsLeft = FMath::CeilToInt( TimeLeft );

    // 서버라면 매번 GameMode에서 LevelStartingTime을 가져옴
    if( HasAuthority() )
    {
        if( LocalGameMode )
        {
            LevelStartingTime = LocalGameMode->LevelStartingTime;
            SecondsLeft = FMath::CeilToInt( LocalGameMode->GetCountDownTime() + LevelStartingTime );
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

    if( CountDownTimeInt != SecondsLeft )
    {
        if( MatchState == MatchState::WaitingToStart )
        {
            SetHUDAnnouncementCountDown( TimeLeft );
        }
        else if( MatchState == MatchState::InProgress )
        {
            SetHUDMatchCountDown( TimeLeft );
        }
        else if( MatchState == MatchState::Cooldown )
        {
            SetHUDAnnouncementCountDown( TimeLeft );
        }
    }
    CountDownTimeInt = SecondsLeft;
}

void ABasePC::SetHUDGrenade(int32 Grenade )
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

void ABasePC::ServerRequestServerTime_Implementation(float TimeOfClientRequest) // 클라 요청 -> 서버 실행
{
    float CurrentServerTIme = GetWorld()->GetTimeSeconds(); // 서버의 현재 시간 가져오기
    ClientReportServerTime(TimeOfClientRequest, CurrentServerTIme); // 서버 시간 보고 함수 호출
}

void ABasePC::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedRequest) // 서버 응답 -> 클라 실행
{
    float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest; // 왕복 시간 계산
    float CurrentServerTime = TimeServerReceivedRequest + (RoundTripTime / 2.0f); // 서버 시간 계산

    ClientServerDeltaTime = CurrentServerTime - GetWorld()->GetTimeSeconds(); // 클라이언트와 서버 간의 시간 차이 계산
}

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

void ABasePC::CheckTimeSync( float DeltaTime )
{
    LastTimeSync += DeltaTime; // 마지막 시간 동기화 시간을 업데이트합니다.
    if( IsLocalController() && LastTimeSync >= TimeSyncFrequency ) // 로컬 컨트롤러인 경우
    {
        LastTimeSync = 0.f; // 마지막 시간 동기화 시간을 초기화합니다.
        ServerRequestServerTime(GetWorld()->GetTimeSeconds()); // 서버에 시간 요청을 보냅니다.
    }
}

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
    }
}

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
    }
}

void ABasePC::HandleMatchHasStarted()
{
    if( MatchState == MatchState::InProgress )
    {
        CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD; // 캐릭터 HUD가 nullptr인 경우 현재 플레이어의 HUD를 캐스팅합니다.
        if( CharacterHUD )
        {
            if( CharacterHUD->Announcement ) // 시작전 안내 위젯이 있는 경우
            {
                CharacterHUD->Announcement->SetVisibility(ESlateVisibility::Collapsed); // 시작전 안내 위젯을 숨깁니다.
            }
            CharacterHUD->AddCharacterUI(); // 캐릭터 오버레이를 추가합니다.

            CharacterHUD->CharacterUI->SetVisibility(ESlateVisibility::Visible); // 캐릭터 UI를 보이도록 설정합니다.
        }
    }
}

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

void ABasePC::ClientJoinMidGame_Implementation( FName StateOfMatch, float WarmUp, float Match, float LevelStarting, float CoolTime ) // 클라이언트가 중간에 게임에 참여할 때 호출되는 함수
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


