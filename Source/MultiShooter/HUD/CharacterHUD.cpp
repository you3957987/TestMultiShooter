#include "CharacterHUD.h"

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

