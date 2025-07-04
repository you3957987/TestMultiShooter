#pragma once

// 캐릭터가 조준시 좌우로 360도 회전하는게 아니라 90도 정도로만 돌아가게 만들기 위한 헤더

UENUM(BlueprintType) // 블루프린트에서 사용 가능하도록 설정
enum class ETurningInPlace : uint8
{
    ETIP_Left UMETA(DisplayName = "Turning Left"), // 왼쪽으로 회전
    ETIP_Right UMETA(DisplayName = "Turning Right"), // 오른쪽으로 회전
    ETIP_NotTurning UMETA(DisplayName = "Not Turning"), // 회전하지 않음
    
    ETIP_MAX UMETA(DisplayName = "DefaultMAX") // 최대값을 정의하여 열거형의 크기를 제한
};