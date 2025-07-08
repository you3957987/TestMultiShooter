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
	class APlayerController* PlayerController; // 플레이어 컨트롤러
	UPROPERTY()
	class ACharacterHUD* CharacterHUD; // 캐릭터 HUD
	UPROPERTY( ReplicatedUsing = OnRep_EquippedWeapon )
	class ABaseWeapon* EquippedWeapon; // 현재 장착된 무기

	UPROPERTY( Replicated )
	bool bIsAiming; // Aim 상태를 나타내는 변수 이거는 캐릭터한테 받아옴

	bool bFireButtonPressed; // 발사 버튼이 눌렸는지 여부

	// 십자선-----------------------------------------------------------------------------
	FHUDPackage HUDPackage; // HUD 패키지, 십자선 관련 정보
	FVector HitTarget;

	float CrosshairVelocityFactor; // 십자선 속도 계수
	float CrosshairInAirFactor; // 십자선 공중 계수
	float CrosshairAimFactor; // 십자선 Aim 계수
	float CrosshairShootingFactor; // 십자선 사격 계수

	float DefaultFOV; // 기본 FOV (Field of View)

	float CurrentFOV; // 현재 FOV

	UPROPERTY(EditAnywhere, Category = "Zoom")
	float ZoomingFOV = 30.f; // Aim 상태에서의 FOV

	UPROPERTY(EditAnywhere, Category = "Zoom")
	float ZoomInterpSpeed = 20.f; // 줌 인터폴레이션 속도

	void InterpFOV(float DeltaTime); // FOV를 인터폴레이션하는 함수
	// 십자선-----------------------------------------------------------------------------

	// 자동 발사-----------------------------------------------------------------------------
	FTimerHandle FireTimer; // 발사 타이머 핸들

	bool bCanFire = true; // 발사 가능 여부

	void FireTimerFinished(); // 발사 타이머가 끝났을 때 호출되는 함수
	void StartFireTimer(); // 발사 타이머를 시작하는 함수

	bool CanFire();

	UPROPERTY(  ReplicatedUsing = OnRep_CarriedAmmo )
	int32 CarriedAmmo; // 보유 총알 개수

	UFUNCTION()
	void OnRep_CarriedAmmo(); // 보유 총알 개수가 변경되었을 때 호출되는 함수

	TMap<EWeaponType, int32> CarriedAmmoMap; // 무기 타입별 총알 개수 맵

	UPROPERTY(  EditAnywhere)
	int32 StartingARAmmo = 30; // 소총 시작 총알 개수

	void InitCarriedAmmo(); // 보유 총알 개수  초기화하는 함수

	UFUNCTION( BlueprintCallable )
	void UpdateAmmoValues(); // HUD에 총알 개수를 업데이트하는 함수

	// 자동 발사-----------------------------------------------------------------------------

	UPROPERTY( ReplicatedUsing = OnRep_CombatState )
	ECombatState CombatState = ECombatState::ECS_Unoccupied; // 전투 상태를 나타내는 변수

	UFUNCTION(  )
	void OnRep_CombatState(); // 전투 상태가 변경되었을 때 호출되는 함수

protected:
	virtual void BeginPlay() override;

	void SetAiming(bool IsAiming); // Aim 상태를 설정하는 함수
	UFUNCTION( Server, Reliable )
	void ServerSetAiming(bool IsAiming); // 서버에서 Aim 상태를 설정하는 함수

	UFUNCTION()
	void OnRep_EquippedWeapon(); // 무기가 변경되었을 때 호출되는 함수

    void Fire(); // 발사 버튼이 눌렸을 때 호출되는 함수

    UFUNCTION( Server, Reliable )
	void Server_Fire( const FVector_NetQuantize& TraceHitTarget ); // 서버에서 발사 버튼이 눌렸을 때 호출되는 함수
	UFUNCTION( NetMulticast, Reliable )
	void Multicast_Fire( const FVector_NetQuantize& TraceHitTarget ); // 멀티캐스트로 발사 버튼이 눌렸을 때 호출되는 함수
	//  FVector_NetQuantize 네트워크 전송용 벡터 타입

	void TraceUnderCrosshair(FHitResult& TraceHitResult); // 십자선 아래의 물체를 추적하는 함수

	void SetHUDCrosshair(float Deltatime); // 십자선 HUD를 설정하는 함수

	UFUNCTION( Server, Reliable )
	void ServerReload(); // 서버에서 리로드를 수행하는 함수

	void HandleReload(); // 리로드를 처리하는 함수

	int32 AmountToReload(); // 리로드할 총알 개수

public:	
	UCombatComponent();

	friend class ABaseCharacter; // ABaseCharacter 클래스가 이 컴포넌트의 private 멤버에 접근할 수 있도록 허용

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; // 뱐수 복제 위한 함수 복제는 변수가 변했을떄만!

	void EquipWeapon(class ABaseWeapon* WeaponToEquip); // 무기를 장착하는 함수

	void Reload();

	void FireButtonPressed(bool bPressed); // 발사 버튼이 눌렸을 때 호출되는 함수

	UFUNCTION( BlueprintCallable )
	void FinshedReload(); // 리로드가 완료되었을 때 호출되는 함수 -> 애님 몽타주 노타퍼아에서 블루프린트로 연결후 거기서 사용!

	void ClearEquippedWeapon();

};
