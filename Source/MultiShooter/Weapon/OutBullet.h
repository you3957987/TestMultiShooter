#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "OutBullet.generated.h"

UCLASS()
class MULTISHOOTER_API AOutBullet : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	class USceneComponent* Root; // 루트 컴포넌트
	
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* BulletMesh; // 총알 메쉬 컴포넌트

	UPROPERTY(EditAnywhere)
	float ImpulsePwoer;

	UPROPERTY(EditAnywhere)
	class USoundCue* BulletSound; // 총알 소리

	bool IsSoundPlayed; // 소리가 재생되었는지 여부

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:	
	AOutBullet();
	virtual void Tick(float DeltaTime) override;

};
