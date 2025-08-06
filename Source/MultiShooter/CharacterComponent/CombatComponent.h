#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MultiShooter/HUD/CharacterHUD.h" // HUD 관련 헤더 파일
#include "MultiShooter/Weapon/WeaponType.h" // 무기 타입 열거형 헤더 파일
#include "MultiShooter/TypeHeader/CombatState.h" // HUD 패키지 구조체 헤더 파일
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTISHOOTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY()
	class ABaseCharacter* Character; // 이 컴포넌트를 소유한 캐릭터
	UPROPERTY()
	class ABasePC* PlayerController; // 플레이어 컨트롤러
	UPROPERTY()
	class ACharacterHUD* CharacterHUD; // 캐릭터 HUD
	UPROPERTY( ReplicatedUsing = OnRep_EquippedWeapon )
	class ABaseWeapon* EquippedWeapon; // 현재 장착된 무기

	UPROPERTY( ReplicatedUsing = OnRep_SecondaryWeapon )
	class ABaseWeapon* SecondaryWeapon; // 보조 무기

	UPROPERTY( ReplicatedUsing = OnRep_Aiming )
	bool bIsAiming = false; // Aim 상태를 나타내는 변수 이거는 캐릭터한테 받아옴

	UFUNCTION()
	void OnRep_Aiming(); // Aim 상태가 변경되었을 때 호출되는 함수

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied; // 전투 상태를 나타내는 변수

	UFUNCTION()
	void OnRep_CombatState(); // 전투 상태가 변경되었을 때 호출되는 함수

	bool bAimButtonPressed = false; // Aim 버튼이 눌렸는지 여부

	bool bFireButtonPressed; // 발사 버튼이 눌렸는지 여부

	UPROPERTY()
	ABaseWeapon* TheFlag; // 깃발 무기

	UPROPERTY(ReplicatedUsing = OnRep_bHoldingFlag)
	bool bHoldingFlag = false; // 깃발을 들고 있는지 여부

	UFUNCTION()
	void OnRep_bHoldingFlag();

	// -----------------------------------------------------------------------------십자선ㅜ
	//
	FHUDPackage HUDPackage; // HUD 패키지, 십자선 관련 정보
	FVector HitTarget;

	float CrosshairVelocityFactor; // 십자선 속도 계수
	float CrosshairInAirFactor; // 십자선 공중 계수
	float CrosshairAimFactor; // 십자선 Aim 계수
	float CrosshairShootingFactor; // 십자선 사격 계수

	float DefaultFOV; // 기본 FOV (Field of View)

	float CurrentFOV; // 현재 FOV

	UPROPERTY(EditAnywhere, Category = "자체설정")
	float ZoomingFOV = 30.f; // Aim 상태에서의 FOV

	UPROPERTY(EditAnywhere, Category = "자체설정")
	float ZoomInterpSpeed = 20.f; // 줌 인터폴레이션 속도

	void InterpFOV(float DeltaTime); // FOV를 인터폴레이션하는 함수
	//
	// -----------------------------------------------------------------------------십자선ㅗ

	// -----------------------------------------------------------------------------자동 발사ㅜ
	//
	FTimerHandle FireTimer; // 발사 타이머 핸들

	bool bCanFire = true; // 발사 가능 여부

	void FireTimerFinished(); // 발사 타이머가 끝났을 때 호출되는 함수
	void StartFireTimer(); // 발사 타이머를 시작하는 함수

	bool CanFire();
	//
	// -----------------------------------------------------------------------------자동 발사ㅗ

	// -----------------------------------------------------------------------------총알ㅜ
	//
	UPROPERTY(  ReplicatedUsing = OnRep_CarriedAmmo )
	int32 CarriedAmmo; // 보유 총알 개수

	UPROPERTY( EditAnywhere, Category = "자체설정")
	int32 MaxCarriedAmmo = 500; // 최대 보유 총알 개수

	UFUNCTION()
	void OnRep_CarriedAmmo(); // 보유 총알 개수가 변경되었을 때 호출되는 함수

	TMap<EWeaponType, int32> CarriedAmmoMap; // 무기 타입별 총알 개수 맵

	void InitCarriedAmmo(); // 보유 총알 개수  초기화하는 함수

	UFUNCTION( BlueprintCallable )
	void UpdateAmmoValues(); // HUD에 총알 개수를 업데이트하는 함수

	UPROPERTY( ReplicatedUsing = OnRep_Grenade )
	int32 Grenade = 4; // 소총 시작 총알 개수

	UPROPERTY( EditAnywhere, Category = "자체설정")
	int32 MaxGrenade = 4;

	UFUNCTION()
	void OnRep_Grenade(); // 수류탄 개수가 변경되었을 때 호출되는 함수

	void UpdateGrenade(); // HUD에 수류탄 개수를 업데이트하는 함수
	// //----------------------------------------------------------------------무기 추가시 총알도 추가!!!
	UPROPERTY(  EditAnywhere, Category = "자체설정")
	int32 StartingARAmmo = 30; // 소총 시작 총알 개수

	UPROPERTY(  EditAnywhere, Category = "자체설정")
	int32 StartingRocketAmmo = 0; // 소총 시작 총알 개수

	UPROPERTY(  EditAnywhere, Category = "자체설정")
	int32 StartingPistolAmmo = 0; // 권총 시작 총알 개수

	UPROPERTY(  EditAnywhere, Category = "자체설정")
	int32 StartingSMGAmmo = 0; // 권총 시작 총알 개수

	UPROPERTY(  EditAnywhere, Category = "자체설정")
	int32 StartingShotgunAmmo = 0; // 샷건 시작 총알 개수

	UPROPERTY(  EditAnywhere, Category = "자체설정")
	int32 StartingSniperAmmo = 0; // 스나 시작 총알 개수

	UPROPERTY(  EditAnywhere, Category = "자체설정")
	int32 StartingGrenadeLauncherAmmo = 0; // 유탄 발사기 시작 총

	UPROPERTY(  EditAnywhere, Category = "자체설정")
	int32 StartingMinigunAmmo = 0; // 유탄 발사기 시작 총

	UPROPERTY(EditAnywhere, Category = "자체설정")
	int32 StartingCrossbowAmmo = 0; // 크로스보우 시작 총알 개수

	UPROPERTY(EditAnywhere, Category = "자체설정")
	int32 StartingHuntingRifleAmmo = 0; // 헌팅 라이플 시작 총알 개수
	
	//-----------------------------------------------------------------------------총알ㅗ

protected:
	virtual void BeginPlay() override;

	UFUNCTION( Server, Reliable )
	void ServerSetAiming(bool IsAiming); // 서버에서 Aim 상태를 설정하는 함수

	UFUNCTION()
	void OnRep_EquippedWeapon(); // 무기가 변경되었을 때 호출되는 함수

	UFUNCTION()
	void OnRep_SecondaryWeapon(); // 무기가 변경되었을 때 호출되는 함수

	//------------------------------------------------------------------------------------------------------------발사ㅜ
	//
    void Fire(); // 발사 버튼이 눌렸을 때 호출되는 함수
	void FireProjectileWeapon();
	void FireHitscanWeapon(); // 히트스캔 무기를 발사하는 함수
	void FireShotgun(); // 샷건 무기를 발사하는 함수
	void LocalFire(const FVector_NetQuantize& TraceHitTarget); // 로컬에서 발사하는 함수
	void LocalShoutgunFire(const TArray<FVector_NetQuantize>& TraceHitTarget); // 로컬에서 샷건을 발사하는 함수

    UFUNCTION( Server, Reliable, WithValidation )
	void Server_Fire( const FVector_NetQuantize& TraceHitTarget, float FireDelay ); // 서버에서 발사 버튼이 눌렸을 때 호출되는 함수
	UFUNCTION( NetMulticast, Reliable )
	void Multicast_Fire( const FVector_NetQuantize& TraceHitTarget ); // 멀티캐스트로 발사 버튼이 눌렸을 때 호출되는 함수
	//  FVector_NetQuantize 네트워크 전송용 벡터 타입
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTarget, float FireDelay);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTarget);
	//
	//------------------------------------------------------------------------------------------------------------발사ㅗ

	// ------------------------------------------------------------------------------------------------------------리로드ㅜ
	//
	UFUNCTION(Server, Reliable)
	void ServerReload(); // 서버에서 리로드를 수행하는 함수
	void HandleReload(); // 리로드를 처리하는 함수
	int32 AmountToReload(); // 리로드할 총알 개수
	void UpdateCarriedAmmo();
	void ReloadEmptyWeapon(); // 빈 무기를 리로드하는 함수
	//
	// ------------------------------------------------------------------------------------------------------------리로드ㅗ

	// ------------------------------------------------------------------------------------------------------------십자선 유틸ㅜ
	//
	void TraceUnderCrosshair(FHitResult& TraceHitResult); // 십자선 아래의 물체를 추적하는 함수
	void SetHUDCrosshair(float Deltatime); // 십자선 HUD를 설정하는 함수
	//
	// ------------------------------------------------------------------------------------------------------------십자선 유틸ㅗ

	// ------------------------------------------------------------------------------------------------------------무기 유틸	ㅜ
	//
	void AttachWeaponToSocket(ABaseWeapon* WeaponToAttach); // 무기를 소켓에 부착하는 함수
	void PlayEquipSound(ABaseWeapon* WeaponToEquip);
	void UpdateWeaponIcon(); // HUD에 무기 아이콘을 업데이트하는 함수
	void EquipFirstWeapon(ABaseWeapon* WeaponToEquip); // 기본 무기를 장착하는 함수
	void EquipSecondaryWeapon(ABaseWeapon* WeaponToEquip); // 보조 무기를 장착하는 함수
	void AttachWeaponToBackpack(ABaseWeapon* WeaponToAttach); // 무기를 백팩에 부착하는 함수
	void AttachFlagToSocket(ABaseWeapon* Flag); // 깃발을 소켓에 부착하는 함수
	void DropEquippedWeapon(); // 현재 장착된 무기를 드롭하는 함수
	//
	// ------------------------------------------------------------------------------------------------------------무기 유틸	ㅗ

	//------------------------------------------------------------------------------------------------------------수류탄ㅜ
	//
	UPROPERTY(EditAnywhere, Category = "자체설정")
	TSubclassOf<class AProjectile> GrenadeClass; // 수류탄 클래스

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade(); // 서버에서 수류탄을 던지는 함수

	void ThrowGrenade(); // 수류탄을 던지는 함수

	UFUNCTION( BlueprintCallable ) // 수류탄 몽타주 노티파이로 연결
	void FireGrenade(); // 수류탄을 발사하는 함수

	UFUNCTION( Server, Reliable )
	void Server_FireGrenade( const FVector_NetQuantize& Target ); // 서버에서 수류탄을 발사하는 함수

	void SetWeaponMesh(bool bShow); // 수류탄 던질 떄 무기 메쉬를 숨기는 함수
	void SetGrenadeMesh(bool bShow); // 수류탄 메쉬를 보이거나 숨기는 함수
	//
	//------------------------------------------------------------------------------------------------------------수류탄ㅗ

public:	
	UCombatComponent();

	friend class ABaseCharacter; // ABaseCharacter 클래스가 이 컴포넌트의 private 멤버에 접근할 수 있도록 허용

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; // 뱐수 복제 위한 함수 복제는 변수가 변했을떄만!

	bool bLocallyReloading = false; // 로컬 플레이어가 리로드 중인지 여부 -- 이거 안쓰는거 같음

	//------------------------------------------------------------------------------------------------------------겟터ㅜ
	//
	FORCEINLINE class ABaseWeapon* GetEquippedWeapon() const { return EquippedWeapon; } // 현재 장착된 무기를 반환하는 함수
	FORCEINLINE int32 GetGrenade() const { return Grenade; } // 현재 보유한 수류탄 개수를 반환하는 함수
	bool ShouldSwapWeapon();
	//
	//------------------------------------------------------------------------------------------------------------겟터ㅗ

	void SetAiming(bool IsAiming); // Aim 상태를 설정하는 함수

	void EquipWeapon(class ABaseWeapon* WeaponToEquip); // 무기를 장착하는 함수
	void SwapWeapon(); // 무기를 교체하는 함수

	void Reload();

	void FireButtonPressed(bool bPressed); // 발사 버튼이 눌렸을 때 호출되는 함수

	UFUNCTION( BlueprintCallable )
	void FinshedReload(); // 리로드가 완료되었을 때 호출되는 함수 -> 애님 몽타주 노타퍼아에서 블루프린트로 연결후 거기서 사용!

	UFUNCTION( BlueprintCallable )
	void FinishedThrowGrenade(); // 수류탄 던지기가 완료되었을 때 호출되는 함수 -> 애님 몽타주 노타퍼아에서 블루프린트로 연결후 거기서 사용!

	UFUNCTION(BlueprintCallable)
	void FinishedSwap();

	UFUNCTION(BlueprintCallable)
	void FinishedSwapAttachWeapon();
	
	void ClearEquippedWeapon();

	void PickUpAmmo( EWeaponType WeaponType, int32 AmmoAmount ); // 무기 타입에 따라 총알을 획득하는 함수

	void PickUpGrenade(int32 Amount); // 수류탄을 획득하는 함수


	// ------------------------------------------------------------------------------------------------------------구르기 ㅜ

	UPROPERTY(EditAnywhere, Category = "자체설정")
	float RollCollDownTime = 5.f; // 구르기 쿨타임

	void Roll(); // 구르기 함수

	UFUNCTION(Server, Reliable)
	void ServerRoll(); // 서버에서 구르기를 수행하는 함수

	UFUNCTION(BlueprintCallable)
	void FinishedRoll(); // 구르기가 완료되었을 때 호출되는 함수 -> 애님 몽타주 노타퍼아에서 블루프린트로 연결후 거기서 사용!

	// ------------------------------------------------------------------------------------------------------------구르기 ㅗ

	// ------------------------------------------------------------------------------------------------------------뒤로 대시 ㅜ

	UPROPERTY(EditAnywhere, Category = "자체설정")
	float BackDashCollDownTime = 7.f; // 뒤로 대시 쿨타임

	void BackDash(); // 뒤로 대시 함수

	UFUNCTION(Server, Reliable)
	void ServerBackDash(); // 서버에서 뒤로 대시를 수행하는 함수

	UFUNCTION(BlueprintCallable)
	void FinishedBackDash(); // 뒤로 대시가 완료되었을 때 호출되는 함수 -> 애님 몽타주 노타퍼아에서 블루프린트로 연결후 거기서 사용!

	// ------------------------------------------------------------------------------------------------------------뒤로 대시 ㅗ
};
