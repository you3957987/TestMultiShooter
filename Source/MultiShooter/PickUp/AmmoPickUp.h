#pragma once

#include "CoreMinimal.h"
#include "PickUp.h"
#include "MultiShooter/Weapon/WeaponType.h" // 무기 타입 열거형 헤더 파일
#include "AmmoPickUp.generated.h"

UCLASS()
class MULTISHOOTER_API AAmmoPickUp : public APickUp
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	int32 AmmoAmount = 30; // 획득할 총알 개수

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType; // 무기 타입
	
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult); // 오버랩 시작 함수

public:
	AAmmoPickUp();
};
