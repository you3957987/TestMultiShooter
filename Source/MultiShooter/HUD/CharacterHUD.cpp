#include "CharacterHUD.h"
#include "GameFramework/PlayerController.h"
#include "CharacterUI.h"
#include "Announcement.h"
#include "ElimAnnouncement.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/HorizontalBox.h"
#include "Components/CanvasPanelSlot.h"
#include "ChatBox.h"

void ACharacterHUD::BeginPlay()
{
    Super::BeginPlay();

    //AddCharacterUI(); // 캐릭터 UI 위젯을 추가합니다.
    //AddAnnouncement(); // 시작전 안내 위젯을 추가합니다.

    APlayerController* PlayerController = GetOwningPlayerController();
	if(PlayerController && CharacterUIClass)
	{
		CharacterUI = CreateWidget<UCharacterUI>(PlayerController,CharacterUIClass); // 캐릭터 UI 위젯 생성
	}
}

// 십자선 HUD를 그리는 함수
void ACharacterHUD::DrawHUD()
{
    Super::DrawHUD();

    FVector2D ViewportSize;
    if (GEngine && GEngine->GameViewport)
    {
        GEngine->GameViewport->GetViewportSize(ViewportSize);
        const FVector2D ViewportCenter(  ViewportSize.X / 2.f, ViewportSize.Y / 2.f); // 화면 중앙 좌표 계산

        float SpreadScaled = HUDPackage.CrosshairSpread * CrosshairSpreadMax; // 십자선 확산 정도를 최대 확산 정도로 스케일링

        if(  HUDPackage.CrosshairCenter )
        {
            FVector2D Spread(0.f, 0.f); // 중앙은 스프레드 없음
            DrawCrosshair(HUDPackage.CrosshairCenter, ViewportCenter, Spread, HUDPackage.CrosshairColor); // 십자선 중앙 텍스처 그리기
        }
   
        if( HUDPackage.CrosshairLeft )
        {
            FVector2D Spread(-SpreadScaled, 0.f); // 왼쪽은 스프레드가 왼쪽으로 확산됨
            DrawCrosshair(HUDPackage.CrosshairLeft, ViewportCenter, Spread, HUDPackage.CrosshairColor); // 십자선 왼쪽 텍스처 그리기
        }

        if( HUDPackage.CrosshairRight )
        {
            FVector2D Spread(SpreadScaled, 0.f); // 오른쪽은 스프레드가 오른쪽으로 확산됨
            DrawCrosshair(HUDPackage.CrosshairRight, ViewportCenter, Spread, HUDPackage.CrosshairColor); // 십자선 오른쪽 텍스처 그리기
        }

        if( HUDPackage.CrosshairTop )
        {
            FVector2D Spread(0.f, SpreadScaled); // 위쪽은 스프레드가 위쪽으로 확산됨
            DrawCrosshair(HUDPackage.CrosshairTop, ViewportCenter, Spread, HUDPackage.CrosshairColor); // 십자선 위쪽 텍스처 그리기
        }
   
        if( HUDPackage.CrosshairBottom )
        {
            FVector2D Spread(0.f, -SpreadScaled); // 아래쪽은 스프레드가 아래쪽으로 확산됨
            DrawCrosshair(HUDPackage.CrosshairBottom, ViewportCenter, Spread, HUDPackage.CrosshairColor); // 십자선 아래쪽 텍스처 그리기
        }

    }

}

void ACharacterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor )
{
    const float TextureWidth = Texture->GetSizeX();
    const float TextureHeight = Texture->GetSizeY();
    const FVector2D TexturDrawPoint(
        ViewportCenter.X - (TextureWidth / 2.f) + Spread.X, // 십자선 중앙 좌표에서 텍스처 너비의 절반을 빼고 확산 정도를 더함
        ViewportCenter.Y - (TextureHeight / 2.f + Spread.Y) // 십자선 중앙 좌표에서 텍스처 높이의 절반을 빼고 확산 정도를 더함
    ); // 십자선 중앙 좌표를 기준으로 텍스처 그리기

    DrawTexture(
        Texture, 
        TexturDrawPoint.X, 
        TexturDrawPoint.Y, 
        TextureWidth, 
        TextureHeight, 
        0.f, 
        0.f, 
        1.f, 
        1.f, 
        CrosshairColor // 원래 색깔로
    ); // 십자선 텍스처 그리기
}

void ACharacterHUD::AddCharacterUI()
{
    /*
    APlayerController* PlayerController = GetOwningPlayerController();
    if (PlayerController && CharacterUIClass )
    {
        CharacterUI = CreateWidget<UCharacterUI>(PlayerController, CharacterUIClass); // 캐릭터 UI 위젯 생성
        CharacterUI->AddToViewport(); // 뷰포트에 추가
        CharacterUI->SetVisibility(ESlateVisibility::Hidden); // 가시성 설정
        UE_LOG(LogTemp, Warning, TEXT("CharacterUI added to viewport.")); // 디버그용 로그 출력
    }
    */
    if(CharacterUI && !CharacterUI->IsInViewport() )
    {
        CharacterUI->AddToViewport();
    }
}

void ACharacterHUD::AddAnnouncement()
{
    APlayerController* PlayerController = GetOwningPlayerController();
    if (PlayerController && AnnouncementClass)
    {
        if( !Announcement )
        {
            Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass); // 시작전 안내 위젯 생성
        }

        if( Announcement && !Announcement->IsInViewport() ) // 위젯이 뷰포트에 없다면
        {
            Announcement->AddToViewport(); // 뷰포트에 추가
        }

        //Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass); // 시작전 안내 위젯 생성
        //Announcement->AddToViewport(); // 뷰포트에 추가
        //UE_LOG(LogTemp, Warning, TEXT("Announcement added to viewport.")); // 디버그용 로그 출력
    }
}

// 킬로그 알림 추가하는 함수
void ACharacterHUD::AddElimAnnouncement(FString Attacker, FString Victim)
{
	OwningPlayerController = OwningPlayerController == nullptr ? GetOwningPlayerController() : OwningPlayerController; // 플레이어 컨트롤러를 가져옵니다.
    if (OwningPlayerController && ElimAnnouncementClass)
    {
       
        UElimAnnouncement* ElimAnnouncement = CreateWidget<UElimAnnouncement>(OwningPlayerController, ElimAnnouncementClass); // 제거 알림 위젯 생성
        
        if (ElimAnnouncement)
        {
			ElimAnnouncement->SetElimAnnouncementText(Attacker, Victim); // 제거 알림 텍스트 설정
      
			ElimAnnouncement->AddToViewport(); // 뷰포트에 추가

			for (UElimAnnouncement* Msg : ElimMessages) // 이전에 존재하던 킬로그 메시지 위치를 아래로 이동시키기
            {
                if (Msg && Msg->AnnouncementBox)
                {
					UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox); // 제거 알림 위젯의 캔버스 슬롯을 가져옵니다.
                    if (CanvasSlot)
                    {
                        FVector2D Position = CanvasSlot->GetPosition(); // 현재 위치를 가져옵니다.
                        FVector2D NewPosition(
                            CanvasSlot->GetPosition().X,
                            Position.Y + CanvasSlot->GetSize().Y
                        );
						CanvasSlot->SetPosition(NewPosition); // 새로운 위치로 설정합니다.
                    }
                }
            }

			ElimMessages.Add(ElimAnnouncement); // 제거 알림 위젯을 배열에 추가

			FTimerHandle ElimAnnouncementTimerHandle; // 제거 알림 타이머 핸들
			FTimerDelegate TimerDelegate; // 타이머 델리게이트
			TimerDelegate.BindUFunction(this, FName("ElimAnouncementTimerFinished"), ElimAnnouncement); // 타이머가 끝났을 때 호출할 함수 바인딩
			GetWorld()->GetTimerManager().SetTimer(
				ElimAnnouncementTimerHandle, // 타이머 핸들
				TimerDelegate, // 타이머 델리게이트
				ElimAnnouncementTime, // 제거 알림 실행 시간
				false // 반복하지 않음
			); // 타이머 설정
        }
	}
}

// 제거 알림 타이머가 끝났을 킬로그 지우는 함수
void ACharacterHUD::ElimAnouncementTimerFinished(UElimAnnouncement* MessageToRemove)
{
    if (MessageToRemove)
    {
		MessageToRemove->RemoveFromParent(); // 제거 알림 위젯을 뷰포트에서 제거합니다.
    }
}