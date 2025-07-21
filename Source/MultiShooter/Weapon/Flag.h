#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "Flag.generated.h"


UCLASS()
class MULTISHOOTER_API AFlag : public ABaseWeapon
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* FlagPole; // 플래그 폴 메쉬 컴포넌트

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* FlagMesh; // 플래그 메쉬 컴포넌트

	FTransform InitialTransform; // 플래그 메쉬의 초기 위치

protected:
	virtual void BeginPlay() override;
	virtual void OnEquipped() override;
	virtual void OnDropped() override;

public:
	AFlag();

	virtual void Dropped() override;

	FORCEINLINE FTransform GetInitialLocation() const { return InitialTransform; } // 플래그 메쉬의 초기 위치를 반환하는 함수

	void ResetFlag();

};
