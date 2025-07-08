#pragma once

UENUM(  BlueprintType ) // 블루프린트에서 사용할 수 있도록 설정
enum class EWeaponType : uint8
{
    EWT_AssaultRifle UMETA( DisplayName = "Assault Rifle" ),
    EWT_MAX UMETA( DisplayName = "DefaultMAX" ) // 최대값을 정의하여 열거형의 크기를 제한
};