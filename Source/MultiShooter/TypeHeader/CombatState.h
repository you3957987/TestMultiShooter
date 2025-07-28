#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
    ECS_Unoccupied UMETA(DisplayName = "Unoccupied"), // 전투 상태가 아닐 때
    ECS_Reloading UMETA(DisplayName = "Reloading"), // 리로드 중일 때
    ECS_ThrowingGrenade UMETA(DisplayName = "ThrowingGrenade"), // 수류탄 던지는 중
	ECS_SwappingWeapons UMETA(DisplayName = "SwappingWeapons"), // 무기 교체 중
	ECS_Rolling UMETA(DisplayName = "Rolling"), // 구르는 중
	ECS_BackDashing UMETA(DisplayName = "BackDashing"), // 뒤로 대시 중

    ECS_MAX UMETA(DisplayName = "DefaultMAX") // 최대값을 정의하여 열거형의 크기를 제한
};