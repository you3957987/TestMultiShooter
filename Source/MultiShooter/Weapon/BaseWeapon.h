#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseWeapon.generated.h"

UENUM(BlueprintType) // 무기 타입을 정의하는 열거형 블루프린트에서도 사용 가능하게
enum class EWeaponState : uint8
{
	EWS_Intital UMETA(DisplayName = "Initial State"),
	EWS_Equiped UMETA(DisplayName = "Equiped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_MAX UMETA(DisplayName = "DefaultMAX") // 최대값을 정의하여 열거형의 크기를 제한,
};

UCLASS()
class MULTISHOOTER_API ABaseWeapon : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	class USkeletalMeshComponent* WeaponMesh; // 무기 메쉬 컴포넌트

	UPROPERTY(VisibleAnywhere)
	class USphereComponent* CollisionSphere; // 충돌 구 컴포넌트

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_WeaponState)
	EWeaponState WeaponState; // 무기 상태, 초기 상태로 설정

	UFUNCTION()
	void OnRep_WeaponState(); // 무기 상태가 변경될 때 호출되는 함수

	UPROPERTY(EditAnywhere)
	class UWidgetComponent* PickUpWidget; // 무기 위젯 컴포넌트

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AOutBullet> OutBulletClass; // 발사할 총알 클래스

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash; // 총구 플래시 이펙트

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	class USceneComponent* MuzzlePoint; // 총구 위치를 나타내는 컴포넌트

	UPROPERTY(EditAnywhere)
	class USceneComponent* BulletOutPoint; // 탄피 배출 위치를 나타내는 컴포넌트

	UFUNCTION()
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex); // 오버랩 종료 함수 

public:	
	ABaseWeapon();

	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; // 뱐수 복제 위한 함수 복제는 변수가 변했을떄만!

	void ShowPickUpWidget(bool bShow); // 위젯을 보이거나 숨기는 함수

	virtual void Fire( const FVector& HitTarget ); // 발사 함수, 타겟 위치를 인자로 받음

	void SetWeaponState(EWeaponState NewState); // 무기 상태를 설정하는 함수

	UFUNCTION(BlueprintCallable)
    FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; } // 무기 메쉬 컴포넌트를 반환하는 함수
	FORCEINLINE USphereComponent* GetCollisionSphere() const { return CollisionSphere; } // 충돌 구 컴포넌트를 반환하는 함수
	FORCEINLINE float GetZoomedFOV() const { return ZoomFOV; } // 줌 FOV를 반환하는 함수
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; } // 줌 인터폴레이션 속도를 반환하는 함수

	// 십자선 HUD------------------------------------------------------- 팔로우 카메라 focal distance 초점 거리 10000으로 설정, camere->조리개 값 4-> 32로 설정!
	UPROPERTY(EditAnywhere, Category = "Crosshair" )
	class UTexture2D* CrosshairCenter; // 십자선 텍스처

	UPROPERTY(EditAnywhere ,Category = "Crosshair")
	class UTexture2D* CrosshairLeft; // 십자선 텍스처

	UPROPERTY(EditAnywhere ,Category = "Crosshair")
	class UTexture2D* CrosshairRight; // 십자선 텍스처

	UPROPERTY(EditAnywhere, Category = "Crosshair")
	class UTexture2D* CrosshairTop; // 십자선 텍스처

	UPROPERTY(EditAnywhere, Category = "Crosshair")
	class UTexture2D* CrosshairBottom; // 십자선 텍스처

	UPROPERTY(EditAnywhere, Category = "Zoom")
	float ZoomFOV = 30.f; // 줌 FOV (Field of View) 기본값

	UPROPERTY(EditAnywhere, Category = "Zoom")
	float ZoomInterpSpeed = 20.f; // 줌 인터폴레이션 속도

	// 십자선 HUD------------------------------------------------------- 팔로우 카메라 focal distance 초점 거리 10000으로 설정, camere->조리개 값 4-> 32로 설정!

	// 자동 발사----------------------------------------------------------------------------------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, Category = "Combat")
	float FireDelay = 0.15f; // 발사 딜레이

	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bAutoFire = true; // 자동 발사 여부
	
	// 자동 발사----------------------------------------------------------------------------------------------------------------------------------------------------------

};