#pragma once

#define TRACE_LENGTH 80000.f // 십자선의 길이
#define CUSTOM_DEPTH_PURPLE 250 // 커스텀 깊이 스텐실 값, 윤곽선 색상 설정을 위해 사용
#define CUSTOM_DEPTH_BLUE 251 // 커스텀 깊이 스텐실 값, 윤곽선 색상 설정을 위해 사용
#define CUSTOM_DEPTH_BROWN 252 // 커스텀 깊이 스텐실 값, 윤곽선 색상 설정을 위해 사용

UENUM(  BlueprintType ) // 블루프린트에서 사용할 수 있도록 설정
enum class EWeaponType : uint8
{
    EWT_AssaultRifle UMETA( DisplayName = "Assault Rifle" ),
    EWT_RocketLauncher UMETA( DisplayName = "Rocket Launcher" ),
    EWT_Pistol UMETA( DisplayName = "Pistol" ),
    EWT_SMG UMETA( DisplayName = "SMG" ),
    EWT_Shotgun UMETA( DisplayName = "Shotgun" ),
    EWT_Sniper UMETA( DisplayName = "Sniper" ),
    EWT_GrenadeLauncher UMETA( DisplayName = "Grenade Launcher" ),
    EWT_MiniGun UMETA( DisplayName = "Minigun" ),
    
    EWT_MAX UMETA( DisplayName = "DefaultMAX" ) // 최대값을 정의하여 열거형의 크기를 제한
};