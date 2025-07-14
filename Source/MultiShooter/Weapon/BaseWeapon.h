#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponType.h" // 무기 타입 열거형 헤더 파일
#include "BaseWeapon.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquippedDelegate);

UENUM(BlueprintType) // 무기 타입을 정의하는 열거형 블루프린트에서도 사용 가능하게
enum class EWeaponState : uint8
{
	EWS_Intital UMETA(DisplayName = "Initial State"),
	EWS_Equiped UMETA(DisplayName = "Equiped"),
	EWS_EquippedSecondary UMETA(DisplayName = "EquippedSecondary"), // 보조 무기 장착 상태
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	EWS_MAX UMETA(DisplayName = "DefaultMAX") // 최대값을 정의하여 열거형의 크기를 제한,
};
UENUM(BlueprintType) // 무기 타입을 정의하는 열거형 블루프린트에서도 사용 가능하게
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "HitScan"), // 히트 스캔 발사
	EFT_Projectile UMETA(DisplayName = "Projectile"), // 발사체 발사
	EFT_Shotgun UMETA(DisplayName = "Shotgun"), // 산탄총 발사
	EFT_MAX UMETA(DisplayName = "DefaultMAX") // 최대값을 정의하여 열거형의 크기를 제한
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

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UWidgetComponent* PickUpWidget; // 무기 위젯 컴포넌트

	UPROPERTY(EditAnywhere, Category = "자체설정")
	TSubclassOf<class AOutBullet> OutBulletClass; // 발사할 총알 클래스

	UPROPERTY(EditAnywhere, Category = "자체설정")
	UParticleSystem* MuzzleFlash; // 총구 플래시 이펙트

	//총알 받아라 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	UPROPERTY(EditAnywhere, Category = "자체설정")
	int32 Ammo; // 총알 개수
	
	UPROPERTY(EditAnywhere, Category = "자체설정")
	int32 MaxAmmo; // 최대 총알 개수

	int32 SequnceAmmo = 0; // 처리되지 않은 서버 탄약 요청의 개수
	
    void SpendAmmo(); // 총알을 소모하는 함수

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo); // 클라이언트에서 서버의 총알 개수를 업데이트하는 함수
	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd); // 클라이언트에서 총알을 추가하는 함수

	UPROPERTY()
	class ABaseCharacter* Character; // 캐릭터 클래스 포인터
	UPROPERTY()
	class ABasePC* PlayerController; // 플레이어 컨트롤러 클래스 포인터

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType; // 무기 타입

	//총알 받아라 !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

protected:
	virtual void BeginPlay() override;
	virtual void OnWeaponStateSet();
	virtual void OnEquipped(); // 무기가 장착될 때 호출되는 함수
	virtual void OnDropped(); // 무기가 드롭될 때 호출되는 함수
	virtual void OnEquippedSecondary(); // 보조 무기가 장착될 때 호출되는 함수

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class USceneComponent* MuzzlePoint; // 총구 위치를 나타내는 컴포넌트

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class USceneComponent* BulletOutPoint; // 탄피 배출 위치를 나타내는 컴포넌트

	UFUNCTION()
	virtual void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex); // 오버랩 종료 함수 

	// 산탄 관련
	UPROPERTY(EditAnywhere, Category = "자체설정")
	float DinstanceToSphere = 800.f; // 타겟과의 거리

	UPROPERTY(EditAnywhere, Category = "자체설정")
	float SphereRadius = 75.f; // 타겟과의 거리

public:	
	ABaseWeapon();

	virtual void Tick(float DeltaTime) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; // 뱐수 복제 위한 함수 복제는 변수가 변했을떄만!

	virtual void OnRep_Owner() override; // 소유자가 변경될 때 호출되는 함수

	virtual void Fire( const FVector& HitTarget ); // 발사 함수, 타겟 위치를 인자로 받음

	void ShowPickUpWidget(bool bShow); // 위젯을 보이거나 숨기는 함수

	void SetWeaponState(EWeaponState NewState); // 무기 상태를 설정하는 함수

	void SetHUDAmmo();

	void AddAmmo(int32 Ammo); // 총알을 추가하는 함수

	UFUNCTION(BlueprintCallable)
    FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; } // 무기 메쉬 컴포넌트를 반환하는 함수
	FORCEINLINE USphereComponent* GetCollisionSphere() const { return CollisionSphere; } // 충돌 구 컴포넌트를 반환하는 함수
	FORCEINLINE float GetZoomedFOV() const { return ZoomFOV; } // 줌 FOV를 반환하는 함수
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; } // 줌 인터폴레이션 속도를 반환하는 함수
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; } // 무기 타입을 반환하는 함수
	FORCEINLINE int32 GetAmmo() const { return Ammo; } // 현재 총알 개수를 반환하는 함수
	FORCEINLINE int32 GetMaxAmmo() const { return MaxAmmo; } // 최대 총알 개수를 반환하는 함수
	FORCEINLINE UTexture2D* GetWeaponIcon() const { return WeaponIcon; } // 무기 상태를 반환하는 함수

	// 십자선 HUD------------------------------------------------------- 팔로우 카메라 focal distance 초점 거리 10000으로 설정, camere->조리개 값 4-> 32로 설정!
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UTexture2D* CrosshairCenter; // 십자선 텍스처

	UPROPERTY(EditAnywhere ,Category = "자체설정")
	class UTexture2D* CrosshairLeft; // 십자선 텍스처

	UPROPERTY(EditAnywhere ,Category = "자체설정")
	class UTexture2D* CrosshairRight; // 십자선 텍스처

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UTexture2D* CrosshairTop; // 십자선 텍스처

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UTexture2D* CrosshairBottom; // 십자선 텍스처

	UPROPERTY(EditAnywhere, Category = "자체설정")
	float ZoomFOV = 30.f; // 줌 FOV (Field of View) 기본값

	UPROPERTY(EditAnywhere, Category = "자체설정")
	float ZoomInterpSpeed = 20.f; // 줌 인터폴레이션 속도

	UPROPERTY(EditAnywhere)
	UTexture2D* WeaponIcon; // 무기 아이콘 텍스처

	// 십자선 HUD------------------------------------------------------- 팔로우 카메라 focal distance 초점 거리 10000으로 설정, camere->조리개 값 4-> 32로 설정!

	// 자동 발사----------------------------------------------------------------------------------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, Category = "자체설정")
	float FireDelay = 0.15f; // 발사 딜레이

	UPROPERTY(EditAnywhere, Category = "자체설정")
	bool bAutoFire = true; // 자동 발사 여부

	bool IsEmptyAmmo(); // 총알이 없는지 확인하는 함수
	bool IsFull(); // 총알이 가득 찼는지 확인하는 함수
	
	// 자동 발사----------------------------------------------------------------------------------------------------------------------------------------------------------

	// 플레이어 죽음----------------------------------------------------------------------------------------------------------------------------------------------------------
	void Dropped();
	// 플레이어 죽음----------------------------------------------------------------------------------------------------------------------------------------------------------

	// 장착 사운드----------------------------------------------------------------------------------------------------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class USoundCue* EquipSound; // 장착 사운드
	// 장착 사운드----------------------------------------------------------------------------------------------------------------------------------------------------------

	// 무기 윤곽선----------------------------------------------------------------------------------------------------------------------------------------------------------
	void EnableCustomDepth(bool bEnable); // 윤곽선을 활성화하거나 비활성화하는 함수
	// 무기 윤곽선----------------------------------------------------------------------------------------------------------------------------------------------------------

	bool bDestroyWeapon = false;

	UPROPERTY(BlueprintAssignable)
	FOnEquippedDelegate OnEquippedDelegate;

	UPROPERTY(EditAnywhere, Category = "자체설정")
	EFireType FireType;

	FVector TraceEndWithScatter(const FVector& HitTarget); // 산란을 고려한 트레이스 끝점 계산 함수

	UPROPERTY(EditAnywhere, Category = "자체설정")
	bool bUseScatter = false; // 산탄 사용 여부
};