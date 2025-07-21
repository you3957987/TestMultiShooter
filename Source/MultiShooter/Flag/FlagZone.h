#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MultiShooter/TypeHeader/Team.h"
#include "FlagZone.generated.h"

UCLASS()
class MULTISHOOTER_API AFlagZone : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere)
	class UBoxComponent* ZoneBox; // 플래그 존의 사각형 콜리전 컴포넌트


	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* ZoneMesh; // 플래그 존의 메쉬 컴포넌트

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnBoxOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


public:	
	AFlagZone();

	UPROPERTY(EditAnywhere, Category = "자체설정")
	ETeam Team; // 이 플래그 존이 속한 팀

};
