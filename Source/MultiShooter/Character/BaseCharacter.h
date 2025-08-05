#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MultiSHooter/TypeHeader/TurningInPlace.h" // 조준시 좌우로 360도 회전하는게 아니라 90도 정도로만 돌아가게 만들기 위한 헤더
#include "MultiShooter/Interface/CrosshairInterface.h" // 크로스헤어 인터페이스 헤더
#include "Components/TimelineComponent.h" // 타임라인 컴포넌트 헤더
#include "MultiShooter/TypeHeader/CombatState.h" // HUD 패키지 구조체 헤더 파일
#include "MultiShooter/TypeHeader/Team.h"
#include "BaseCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

UCLASS()
class MULTISHOOTER_API ABaseCharacter : public ACharacter, public ICrosshairInterface // ICrosshairInterface 인터페이스를 구현합니다.
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* FrontCamera;

	UPROPERTY(EditAnywhere, Category = "자체설정", BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverHeadWidget;

	UPROPERTY(EditAnywhere, Category = "자체설정", BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* TeamLogoWidget;

	// 팀별 위젯 클래스(빨강/파랑)
	UPROPERTY(EditAnywhere, Category = "자체설정", BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> RedTeamWidgetClass;
	UPROPERTY(EditAnywhere, Category = "자체설정", BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> BlueTeamWidgetClass;

	UPROPERTY()
	class AMainGameMode* MainGameMode; // 메인 게임 모드

	void UpdateWidgetForLocalView(UWidgetComponent* Widget);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* CombatComponent; // 전투 컴포넌트

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UBuffComponent* BuffComponent; // 버프 컴포넌트

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class ULagCompensationComponent* LagCompensationComponent; // 지연 보상 컴포넌트

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UNiagaraSystem* CrownSystem; // 최고 점수 플레이어에게 생성되는 크라운 이펙트

	UPROPERTY()
	class UNiagaraComponent* CrownComponent; // 크라운 이펙트를 생성하기 위한 Niagara 컴포넌트

	// --------------------------------------------------------------------------------------------------------무기 관련 ㅜ
	//
	UPROPERTY(EditAnywhere, Category = "자체설정")
	TSubclassOf<ABaseWeapon> DefaultWeaponClass; // 기본 무기 클래스

	FTimerHandle StartWeaponTimer; // 기본 무기를 장착하기 위한 타이머 핸들
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* GrenadeMesh; // 수류탄 메쉬 컴포넌트

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class ABaseWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(ABaseWeapon* LastWeapon);

	UFUNCTION(Server, Reliable) //OverlappingWeapon이 변경되었을 때 호출되는 함 이 함수는 서버에서는 실행 안하고 클라이언트에서만 실행
	void ServerEquipWeapon(); // 클라이언트에게 무기 장착하라고 지시하는 함수
	//
	// --------------------------------------------------------------------------------------------------------무기 관련 ㅗ

	// --------------------------------------------------------------------------------------------------------IA 입력 매핑 ㅜ
	//
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UInputAction* IA_Jump;
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UInputAction* IA_Move;
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UInputAction* IA_Look;
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UInputAction* IA_Equip;
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UInputAction* IA_Aim;
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UInputAction* IA_Fire;
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UInputAction* IA_Reload;
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UInputAction* IA_Grenade; // 수류탄 던지는 액션
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UInputAction* IA_Roll; // 무기 교체 액션
	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UInputAction* IA_BackDash; // 뒤로 대시 액션

	UPROPERTY(EditDefaultsOnly, Category = "자체설정")
	class UInputAction* IA_CtrlOnly; // Ctrl 키만 눌렀을 때 액션
	//
	// --------------------------------------------------------------------------------------------------------IA 입력 매핑 ㅗ

	// -------------------------------------------------------------------------------캐릭터 에임 오프셋 관련(조준시 몸 틀어지는거) ㅜ
	//
	float AO_Pitch; // 캐릭터의 AO 피치를 저장합니다. 조준 시에 사용됩니다.
	float AO_Yaw; // 캐릭터의 AO 요 를 저장합니다. 조준 시에 사용됩니다.
	float InterpAO_Yaw; // AO_Yaw를 보간하기 위한 변수입니다. 조준 시에 사용됩니다.
	FRotator StartingAimRotation; // 조준 회전을 시작할 때의 회전값을 저장합니다.

	ETurningInPlace TurningInPlace; // 캐릭터가 조준 중일 때 좌우로 회전하는 상태를 제한하기 위한 열거형 변수!
	void TurnInPlace(float DeltaTime); // 조준 중일 때 좌우로 회전하는 상태를 처리하는 함수
	//
	// -------------------------------------------------------------------------------캐릭터 에임 오프셋 관련(조준시 몸 틀어지는거) ㅗ

	// -------------------------------------------------------------------------------카메라 관련 유틸(캐릭터 너무 가까우면 카메라 숨기기) ㅜ
	//
	void HideCameraIfCharacterClose(float DeltaTime); // 캐릭터가 가까이 있을 때 카메라를 숨기는 함수
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f; // 카메라가 캐릭터와 얼마나 가까이 있을 때 숨길지 설정하는 변수
	bool bCloseWall = false; // 벽에 가까이 있을 때 true로 설정되는 변수
	//
	// --------------------------------------------------------------------------------카메라 관련 유틸(캐릭터 너무 가까우면 카메라 숨기기) ㅗ

	//-------------------------------------------------------------------------------------------------------애니메이션 몽타주ㅜ
	//
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* FireAssultMontage; // 무기를 발사할 때 사용할 애니메이션 몽타주 

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* FireRocketMontage; // 무기를 발사할 때 사용할 애니메이션 몽타주
	
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* FirePistolMontage; // 무기를 발사할 때 사용할 애니메이션 몽타주 

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* FireSMGMontage; // 무기를 발사할 때 사용할 애니메이션 몽타주
	
	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* FireShotgunMontage; // 무기를 발사할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* FireSniperMontage; // 무기를 발사할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* FireGrenadeLauncherMontage; // 유탄 발사기를 발사할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* FireMinigunMontage; // 미니건을 발사할 때 사용할 애니메이션 몽타주

	//-------------------------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* ReloadAssultMontage; // 무기를 재장전할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* ReloadRocketMontage; // 무기를 재장전할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* ReloadPistolMontage; // 무기를 재장전할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* ReloadSMGMontage; // 무기를 재장전할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* ReloadShotgunMontage; // 무기를 재장전할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* ReloadSniperMontage; // 무기를 재장전할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* ReloadGrenadeLauncherMontage; // 유탄 발사기를 재장전할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* ReloadMinigunMontage; // 미니건을 재장전할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* ThrowGrenadeMontage; // 유탄 발사기를 재장전할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* HitReactMontage; // 적에게 맞았을 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* DieMontage; // 적에게 맞았을 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* SwapMontage; // 무기를 교체할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* RollMontage; // 무기를 교체할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere, Category = "자체설정")
	class UAnimMontage* BackDashMontage; // 뒤로 대시할 때 사용할 애니메이션 몽타주
	
	//-------------------------------------------------------------------------------------------------------애니메이션 몽타주ㅗ

	//-------------------------------------------------------------------------------------------------------플레이어 상태ㅜ
	//
	UPROPERTY()
	class ACharacterState* CharacterState; // 플레이어의 상태를 저장하는 CharacterState 클래스의 인스턴스

	UPROPERTY(EditAnywhere, Category = "자체설정", meta = (AllowPrivateAccess = "true"))
	float MAxHealth = 100.f; // 최대 체력
	
	UPROPERTY( ReplicatedUsing = OnRep_Health, VisibleAnyWhere, meta = (AllowPrivateAccess = "true") )
	float Health = 100.f; // 현재 체력

	UPROPERTY(EditAnywhere, Category = "자체설정", meta = (AllowPrivateAccess = "true"))
	float MAXShield = 100.f; 
	
	UPROPERTY( ReplicatedUsing = OnRep_Shield, VisibleAnyWhere, meta = (AllowPrivateAccess = "true") )
	float Shield = 0.f; 

	UFUNCTION()
	void OnRep_Health(float LastHealth); // 체력이 변경되었을 때 호출되는 함수

	UFUNCTION()
	void OnRep_Shield(float LastShield); // 방어력이 변경되었을 때 호출되는 함수

	UPROPERTY()
	class ABasePC* PlayerController; // 플레이어 컨트롤러

	bool bDead = false;

	FTimerHandle EliminateTimer; // 플레이어가 제거되었을 때 타이머 핸들
	UPROPERTY( EditDefaultsOnly, Category = "Player Status" ) 
	float EliminateDelay = 5.f; // 플레이어가 제거되었을 때 타이머 딜레이

	void EliminateTimerFinished(); // 플레이어가 제거되었을 때 타이머가 끝났을 때 호출되는 함수
	//
	//-------------------------------------------------------------------------------------------------------플레이어 상태ㅗ

	//-------------------------------------------------------------------------------------------------------죽었을떄 관련(디졸브 이펙트)ㅜ 
	//
	UPROPERTY( VisibleAnyWhere )
	class UTimelineComponent* DissolveTimeline; // 디졸브 타임라인 컴포넌트

	class FOnTimelineFloat DissolveTrack; // 디졸브 트랙을 위한 타임라인 핸들

	UFUNCTION()
	void UpdateDissolve(float DissolveValue); // 디졸브 값을 업데이트하는 함수

	void StartDissolve(); // 디졸브를 중지하는 함수

	UPROPERTY( EditAnywhere )
	class UCurveFloat* DissolveCurve; // 디졸브를 위한 커브

	UPROPERTY( VisibleAnyWhere )
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstanceBody; // 디졸브를 위한 동적 머티리얼 인스턴스. 게임 도중 바꿀 수 있음

	UPROPERTY( EditAnywhere, Category = "디졸브" )
	UMaterialInstance* DIssolveMaterialInstanceBody; // 디졸브를 위한 머티리얼 인스턴스

	UPROPERTY( VisibleAnyWhere )
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstanceHead; // 디졸브를 위한 동적 머티리얼 인스턴스. 게임 도중 바꿀 수 있음

	UPROPERTY( EditAnywhere, Category = "디졸브" )
	UMaterialInstance* DIssolveMaterialInstanceHead; // 디졸브를 위한 머티리얼 인스턴스

	UPROPERTY(EditAnywhere, Category = "디졸브")
	TArray<UMaterialInstance*> DissolveMaterialsBodyArray;// 디졸브를 위한 머티리얼 인스턴스 배열

	UPROPERTY(EditAnywhere, Category = "디졸브")
	TArray<UMaterialInstance*> DissolveMaterialsHeadArray; // 디졸브를 위한 머티리얼 인스턴스 배열

	void EliminateBotTimerFinished(); // 제거 봇 타이머가 끝났을 때 호출되는 함수

	UPROPERTY( EditAnywhere, Category = "자체설정" )
	UParticleSystem* EliminateBotEffect; // 플레이어가 제거되었을 때 생성되는 파티클 시스템

	UPROPERTY( VisibleAnyWhere)
	UParticleSystemComponent* EliminateBotEffectComponent; // 플레이어가 제거되었을 때 생성되는 파티클 시스템 컴포넌트

	UPROPERTY( EditAnywhere, Category = "자체설정" )
	class USoundCue* EliminateSound; // 플레이어가 제거되었을 때 재생되는 사운드 큐
	//
	//-------------------------------------------------------------------------------------------------------죽었을떄 관련(디졸브 이펙트)ㅗ 

	// 이거 91 강의 무슨 애니메이션 흔들림 없앤다는데 좀 이상해서 안씀. 안써도 그리 이상하지 않음
	bool bRotateRootBone; // 루트 본을 회전할지 여부를 저장하는 변수
	float TurnThreshold = 0.5f; // 조준 중일 때 회전할 최소 각도
	FRotator ProxyRotationLastFrame; // 로컬 플레이어가 아닌 경우 회전값을 저장하는 변수
	FRotator ProxyRotation; // 로컬 플레이어가 아닌 경우 현재 회전값을 저장하는 변수
	float ProxyYaw; // 로컬 플레이어가 아닌 경우 현재 Yaw 값을 저장하는 변수
	float TimeSinceLastMovementReplicattion; // 마지막으로 이동이 복제된 시간
	// 이거 91 강의 무슨 애니메이션 흔들림 없앤다는데 좀 이상해서 안씀. 안써도 그리 이상하지 않음

protected:
    virtual void BeginPlay() override;

	// ----------------------------------------------------------------------------------------------입력 매핑 바인딩 함수들ㅜ 
	//
    UFUNCTION()
	void Move(const FInputActionValue& Value);
	UFUNCTION()
	void Look(const FInputActionValue& Value);
	virtual void Jump(); // 점프 입력을 처리하는 함수
	UFUNCTION()
	void Equip();
	UFUNCTION()
	void AimPressed();
	UFUNCTION()
	void AimReleased();
	UFUNCTION()
	void FirePressed();
	UFUNCTION()
	void FireReleased();
	UFUNCTION()
	void Reload();
	UFUNCTION()
	void Grenade(); // 수류탄 던지기 입력을 처리하는 함수
	UFUNCTION()
	void Roll(); // 굴러잇
	UFUNCTION()
	void BackDash(); // 뒤로 대시 입력을 처리하는 함수


	bool bWaitingForCtrl = false;
	FTimerHandle CtrlWaitTimerHandle;
	void OnReloadKeyPressed();
	void OnCtrlKeyPressed();
	void OnReloadTimeout();

	//
	// ----------------------------------------------------------------------------------------------입력 매핑 바인딩 함수들ㅗ

	// ---------------------------------------------------------------------------------------------조준 오프셋 관련 함수들ㅜ 
	//
    void AimOffset(float DeltaTime); // 조준 오프셋을 계산하는 함수
    void Calculate_AO_Pitch();
    void SimProxyTurn(); // 조준중 회전값 전달을 매끄럽게 하는 함수
	//
	// ---------------------------------------------------------------------------------------------조준 오프셋 관련 함수들ㅗ 

	UFUNCTION()
	void ReceiveDamage( AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser ); 

	void PollInit(float DeltaTime); // 관련 클래스를 확인해서 HUD를 세팅
	void OnPlayerStateInitialized(); // 플레이어 상태가 초기화되었을 때 호출되는 함수

	void SetSpawnPoint(); // 스폰 포인트를 설정하는 함수
	bool bFirstSpawn = true; // 첫 스폰 여부를 저장하는 변수 // 개인 게임에서 첫 스폰시 위치 랜덤하게

public:	
	ABaseCharacter();	
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override; // 이 함수는 BeginPlay() 전에 호출되며, 컴포넌트 초기화에 사용됩니다.
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; // 뱐수 복제 위한 함수 복제는 변수가 변했을떄만!
	virtual void OnRep_ReplicatedMovement() override; // ReplicatedMovement가 변경되었을 때 호출되는 함수
	virtual void Destroyed() override; // 액터가 파괴될 때 호출되는 함수
    virtual void Restart() override; // 매치 스타트 할 시 서버에서 입력이 안되는거 해결 위한 함수 

	// --------------------------------------------------------------------------------------------------------겟터 함수들ㅜ 
	
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; } // AO_Yaw를 반환하는 함수
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; } // AO_Pitch를 반환하는 함수
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; } // TurningInPlace를 반환하는 함수
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; } // FollowCamera를 반환하는 함수
	FORCEINLINE bool GetCloseWall() const { return bCloseWall; } // bCloseWall을 반환하는 함수
	FORCEINLINE bool IsDead() const { return bDead; } // bDie를 반환하는 함수
	FORCEINLINE float GetHealth() const { return Health; } // Health를 반환하는 함수
	FORCEINLINE float GetMaxHealth() const { return MAxHealth; } // MaxHealth를 반환하는 함수
	FORCEINLINE void SetHealth( float Amount ) { Health = Amount; } // Health를 설정하는 함수
	FORCEINLINE float GetShield() const { return Shield; } // Shield를 반환하는 함수
	FORCEINLINE float GetMaxShield() const { return MAXShield; } // MaxShield를 반환하는 함수
	FORCEINLINE void SetShield( float Amount ) { Shield = Amount; } // Shield를 설정하는 함수
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return CombatComponent; } // CombatComponent를 반환하는 함수
	FORCEINLINE UBuffComponent* GetBuffComponent() const { return BuffComponent; } // BuffComponent를 반환하는 함수
	FORCEINLINE UStaticMeshComponent* GetGrenadeMesh() const { return GrenadeMesh; } // GrenadeMesh를 반환하는 함수
	FORCEINLINE ULagCompensationComponent* GetLagCompensationComponent() const { return LagCompensationComponent; } // LagCompensationComponent를 반환하는 함수
	bool IsHoldingTheFlag() const;
	bool IsWeaponEquipped();
	bool IsAiming(); // CombatComponent가 존재할 때만 bIsAiming을 반환
	bool IsLocallyReloading(); // 로컬 플레이어가 리로드 중인지 확인하는 함수
	ECombatState GetCombatState() const; // CombatComponent가 존재할 때만 CombatState를 반환
	ABaseWeapon* GetEquippedWeapon() const; // CombatComponent가 존재할 때만 EquippedWeapon을 반환
	ETeam GetTeam(); // CombatComponent가 존재할 때만 Team을 반환


	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; } // bRotateRootBone을 반환하는 함수
	// 이거 91 강의 무슨 애니메이션 흔들림 없앤다는데 좀 이상해서 안씀. 안써도 그리 이상하지 않음
	
	// --------------------------------------------------------------------------------------------------------겟터 함수들ㅗ 

	// --------------------------------------------------------------------------------------------------------무기 관련 함수들ㅜ 
	
	void SetOverlappingWeapon(ABaseWeapon* Weapon);
	void SpawnDefaultWeapon(); // 기본 무기를 스폰하는 함수
	void InitWeaponAndHUD(); // 기본 무기를 장착하고 HUD를 초기화하는 함수
	
	// --------------------------------------------------------------------------------------------------------무기 관련 함수들ㅗ 

	// ---------------------------------------------------------------------------------------------애니메이션 몽타주 재생 함수들ㅜ 
	
	void PlayFireMontage(); // 무기 발사 애니메이션 몽타주를 재생하는 함수. 
	void PlayHitReactMontage(); // 적에게 맞았을 때 애니메이션 몽타주를 재생하는 함수
	void PlayDieMontage(); // 플레이어가 죽었을 때 애니메이션 몽타주를 재생하는 함수
	void PlayReloadMontage(); // 무기를 재장전할 때 애니메이션 몽타주를 재생하는 함수
	void PlayThrowGrenadeMontage(); // 유탄 발사기를 던질 때 애니메이션 몽타주를 재생하는 함수
	void PlaySwapMontage(); // 무기를 교체할 때 애니메이션 몽타주를 재생하는 함수
	void PlayRollMontage(); // 무기를 교체할 때 애니메이션 몽타주를 재생하는 함수
	void PlayBackDashMontage(); // 뒤로 대시할 때 애니메이션 몽타주를 재생하는 함수
	
	// ---------------------------------------------------------------------------------------------애니메이션 몽타주 재생 함수들ㅗ

	// ---------------------------------------------------------------------------------------------HUD 업데이트 함수들ㅜ
	//
	void UpdateHUDHealth(); // HUD에 체력을 업데이트하는 함수  버프 컴포넌트에서 사용할 예정이므로 퍼블릭
	void UpdateHUDShield(); // HUD에 방어력을 업데이트하는 함수  버프 컴포넌트에서 사용할 예정이므로 퍼블릭
	void UpdateHUDAmmo(); // HUD에 탄약을 업데이트하는 함수
	void UpdateHUDGrenade(); // HUD에 수류탄 개수를 업데이트하는 함수 
	UFUNCTION(BlueprintImplementableEvent) // 이 함수는 블루프린트에서 구현됩니다. 그러니깐 cpp에 없음,,
	void ShowScopeSniperWidget(bool bShow); // 스코프 위젯을 보이게 하는 함수
	void UpdateHUDPing(); // HUD에 핑을 업데이트하는 함수

	//-------------------------------------------------------------------------------------------- HUD 업데이트 함수들ㅗ

	// -------------------------------------------------------------------------------------게임 전체적인 게임모드 관련 함수들ㅜ
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminate(bool bPlayerLeftGame); // 플레이어가 제거되었을 때 호출되는 멀티캐스트 함수

	void Eliminate(bool bPlayerLeftGame); // 플레이어가 제거되었을 때 호출되는 서버에서만 실행하는 함수

	UPROPERTY(Replicated)
	bool bDisableGamePlay = false; // 게임 플레이를 비활성화하는 변수

	UFUNCTION(Server, Reliable)
	void ServerLeaveGame(); // 서버에서 게임을 떠나는 함수

	FOnLeftGame OnLeftGame; // 플레이어가 게임을 떠났을 때 호출되는 델리게이트
	bool bLeftGame = false; // 플레이어가 게임을 떠났는지 여부를 저장하는 변수

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead(); // 최고 점수 플레이어에게 크라운 이펙트를 생성하는 멀티캐스트 함수
	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();

	void SetTeamColor(ETeam Team); // 팀 색상을 설정하는 함수
	void SetHoldingTheFlag(bool bHolding); // 플레이어가 깃발을 들고 있는지 여부를 설정하는 함수

	void HideSniperScope();// 스나이퍼 스코프를 숨기는 함수
	
	// -------------------------------------------------------------------------------------게임 전체적인 게임모드 관련 함수들ㅗ

	//--------------------------------------------------------------------------------------------------------캐릭터 변경 관련ㅜ

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "자체설정")
	TArray<USkeletalMesh*> SelectedSkeletalMesh;

	UPROPERTY(ReplicatedUsing = OnRep_CharacterSkeletalMeshName)
	FString CharacterSkeletalMeshName; // 캐릭터 스켈레탈 메쉬 이름

	UFUNCTION()
	void OnRep_CharacterSkeletalMeshName();

	UFUNCTION(Server, Reliable)
	void ServerSetCharacterMeshName(const FString& NewMeshName);


	UFUNCTION(BlueprintCallable)
	void SetCharacterMeshOnMenu(USkeletalMesh* NewMesh); // 메뉴에서 캐릭터 메쉬를 설정하는 함수

	void SetCharacterMesh(); // 캐릭터 메쉬를 설정하는 함수

	void SetCamera(); // 비긴 플레이에서 맵에 따라서 카메라를 설정하는 함수

	bool bTriedSetCharacterMesh = false; // pollinit 에서 사용

	//--------------------------------------------------------------------------------------------------------캐릭터 변경 관련ㅗ


	// ------------------------------------------------------------------------------서버에서 렉걸릴떄 히트 박스 조정하기 위한 박스ㅜ
	
	UPROPERTY(EditAnywhere)
	class UBoxComponent* head;
	UPROPERTY(EditAnywhere)
	UBoxComponent* pelvis;
	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_02;
	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_03;
	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_r;
	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_r;
	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_r;
	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_r;
	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_r;
	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_l;
	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_r;

	UPROPERTY()
	TMap<FName, UBoxComponent* > HitCollisionBoxes; // 히트 박스 맵. 이름과 박스 컴포넌트를 매핑합니다.
	
	// ---------------------------------------------------------------------------------서버에서 렉걸릴떄 히트 박스 조정하기 위한 박스ㅗ
};
