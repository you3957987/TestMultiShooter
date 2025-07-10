#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MultiSHooter/TypeHeader/TurningInPlace.h" // 조준시 좌우로 360도 회전하는게 아니라 90도 정도로만 돌아가게 만들기 위한 헤더
#include "MultiShooter/Interface/CrosshairInterface.h" // 크로스헤어 인터페이스 헤더
#include "Components/TimelineComponent.h" // 타임라인 컴포넌트 헤더
#include "MultiShooter/TypeHeader/CombatState.h" // HUD 패키지 구조체 헤더 파일
#include "BaseCharacter.generated.h"

UCLASS()
class MULTISHOOTER_API ABaseCharacter : public ACharacter, public ICrosshairInterface // ICrosshairInterface 인터페이스를 구현합니다.
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* FollowCamera;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverHeadWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Jump;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Move;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Look;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Equip;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Aim;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Fire;
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* IA_Reload;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class ABaseWeapon* OverlappingWeapon;
	UFUNCTION()
	void OnRep_OverlappingWeapon( ABaseWeapon* LastWeapon ); 
	// OverlappingWeapon이 변경되었을 때 호출되는 함 이 함수는 서버에서는 실행 안하고 클라이언트에서만 실행

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* CombatComponent; // 전투 컴포넌트

	UFUNCTION( Server, Reliable )
	void ServerEquipWeapon(); // 클라이언트에게 무기 장착하라고 지시하는 함수

	float AO_Pitch; // 캐릭터의 AO 피치를 저장합니다. 조준 시에 사용됩니다.
	float AO_Yaw; // 캐릭터의 AO 요 를 저장합니다. 조준 시에 사용됩니다.
	float InterpAO_Yaw; // AO_Yaw를 보간하기 위한 변수입니다. 조준 시에 사용됩니다.
	FRotator StartingAimRotation; // 조준 회전을 시작할 때의 회전값을 저장합니다.

	ETurningInPlace TurningInPlace; // 캐릭터가 조준 중일 때 좌우로 회전하는 상태를 제한하기 위한 열거형 변수!
	void TurnInPlace(float DeltaTime); // 조준 중일 때 좌우로 회전하는 상태를 처리하는 함수

	void HideCameraIfCharacterClose(); // 캐릭터가 가까이 있을 때 카메라를 숨기는 함수
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f; // 카메라가 캐릭터와 얼마나 가까이 있을 때 숨길지 설정하는 변수
	bool bCloseWall = false; // 벽에 가까이 있을 때 true로 설정되는 변수

	// 이거 91 강의 무슨 애니메이션 흔들림 없앤다는데 좀 이상해서 안씀. 안써도 그리 이상하지 않음
	bool bRotateRootBone; // 루트 본을 회전할지 여부를 저장하는 변수
	float TurnThreshold = 0.5f; // 조준 중일 때 회전할 최소 각도
	FRotator ProxyRotationLastFrame; // 로컬 플레이어가 아닌 경우 회전값을 저장하는 변수
	FRotator ProxyRotation; // 로컬 플레이어가 아닌 경우 현재 회전값을 저장하는 변수
	float ProxyYaw; // 로컬 플레이어가 아닌 경우 현재 Yaw 값을 저장하는 변수
	float TimeSinceLastMovementReplicattion; // 마지막으로 이동이 복제된 시간
	// 이거 91 강의 무슨 애니메이션 흔들림 없앤다는데 좀 이상해서 안씀. 안써도 그리 이상하지 않음

	//애니메이션 몽타주-------------------------------------------------------------------------------------------------------
	UPROPERTY(EditAnywhere)
	class UAnimMontage* FireAssultMontage; // 무기를 발사할 때 사용할 애니메이션 몽타주 

	UPROPERTY(EditAnywhere)
	class UAnimMontage* FireRocketMontage; // 무기를 발사할 때 사용할 애니메이션 몽타주
	
	UPROPERTY(EditAnywhere)
	class UAnimMontage* FirePistolMontage; // 무기를 발사할 때 사용할 애니메이션 몽타주 

	UPROPERTY(EditAnywhere)
	class UAnimMontage* FireSMGMontage; // 무기를 발사할 때 사용할 애니메이션 몽타주
	
	UPROPERTY(EditAnywhere)
	class UAnimMontage* FireShotgunMontage; // 무기를 발사할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere)
	class UAnimMontage* FireSniperMontage; // 무기를 발사할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere)
	class UAnimMontage* FireGrenadeLauncherMontage; // 유탄 발사기를 발사할 때 사용할 애니메이션 몽타주

	//-------------------------------------------------------------------------------------------------

	UPROPERTY(EditAnywhere)
	class UAnimMontage* ReloadAssultMontage; // 무기를 재장전할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere)
	class UAnimMontage* ReloadRocketMontage; // 무기를 재장전할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere)
	class UAnimMontage* ReloadPistolMontage; // 무기를 재장전할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere)
	class UAnimMontage* ReloadSMGMontage; // 무기를 재장전할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere)
	class UAnimMontage* ReloadShotgunMontage; // 무기를 재장전할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere)
	class UAnimMontage* ReloadSniperMontage; // 무기를 재장전할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere)
	class UAnimMontage* ReloadGrenadeLauncherMontage; // 유탄 발사기를 재장전할 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere)
	class UAnimMontage* HitReactMontage; // 적에게 맞았을 때 사용할 애니메이션 몽타주

	UPROPERTY(EditAnywhere)
	class UAnimMontage* DieMontage; // 적에게 맞았을 때 사용할 애니메이션 몽타주

	//애니메이션 몽타주-------------------------------------------------------------------------------------------------------

	//플레이어 상태-------------------------------------------------------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Player Status", meta = (AllowPrivateAccess = "true"))
	float MAxHealth = 100.f; // 최대 체력
	
	UPROPERTY( ReplicatedUsing = OnRep_Health, VisibleAnyWhere, Category = "Player Status", meta = (AllowPrivateAccess = "true") )
	float Health = 100.f; // 현재 체력

	UFUNCTION()
	void OnRep_Health(); // 체력이 변경되었을 때 호출되는 함수

	UPROPERTY()
	class ABasePC* PlayerController; // 플레이어 컨트롤러

	bool bDead = false;

	FTimerHandle EliminateTimer; // 플레이어가 제거되었을 때 타이머 핸들
	UPROPERTY( EditDefaultsOnly, Category = "Player Status" ) 
	float EliminateDelay = 5.f; // 플레이어가 제거되었을 때 타이머 딜레이

	void EliminateTimerFinished(); // 플레이어가 제거되었을 때 타이머가 끝났을 때 호출되는 함수
	//플레이어 상태-------------------------------------------------------------------------------------------------------

	//디졸브 및 죽으면 나오는 봇 -------------------------------------------------------------------------------------------------------

	UPROPERTY( VisibleAnyWhere )
	class UTimelineComponent* DissolveTimeline; // 디졸브 타임라인 컴포넌트

	class FOnTimelineFloat DissolveTrack; // 디졸브 트랙을 위한 타임라인 핸들

	UFUNCTION()
	void UpdateDissolve(float DissolveValue); // 디졸브 값을 업데이트하는 함수

	void StartDissolve(); // 디졸브를 중지하는 함수

	UPROPERTY( EditAnywhere )
	class UCurveFloat* DissolveCurve; // 디졸브를 위한 커브

	UPROPERTY( VisibleAnyWhere, Category = "Dissolve" )
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstanceBody; // 디졸브를 위한 동적 머티리얼 인스턴스. 게임 도중 바꿀 수 있음

	UPROPERTY( EditAnywhere, Category = "Dissolve" )
	UMaterialInstance* DIssolveMaterialInstanceBody; // 디졸브를 위한 머티리얼 인스턴스

	UPROPERTY( VisibleAnyWhere, Category = "Dissolve" )
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstanceHead; // 디졸브를 위한 동적 머티리얼 인스턴스. 게임 도중 바꿀 수 있음

	UPROPERTY( EditAnywhere, Category = "Dissolve" )
	UMaterialInstance* DIssolveMaterialInstanceHead; // 디졸브를 위한 머티리얼 인스턴스

	void EliminateBotTimerFinished(); // 제거 봇 타이머가 끝났을 때 호출되는 함수

	UPROPERTY( EditAnywhere, Category = "Dissolve" )
	UParticleSystem* EliminateBotEffect; // 플레이어가 제거되었을 때 생성되는 파티클 시스템

	UPROPERTY( VisibleAnyWhere)
	UParticleSystemComponent* EliminateBotEffectComponent; // 플레이어가 제거되었을 때 생성되는 파티클 시스템 컴포넌트

	UPROPERTY( EditAnywhere, Category = "Dissolve" )
	class USoundCue* EliminateSound; // 플레이어가 제거되었을 때 재생되는 사운드 큐

	//디졸브 및 죽으면 나오는 봇 -------------------------------------------------------------------------------------------------------

	UPROPERTY()
	class ACharacterState* CharacterState; // 플레이어의 상태를 저장하는 CharacterState 클래스의 인스턴스

protected:
    virtual void BeginPlay() override;

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


    void AimOffset(float DeltaTime); // 조준 오프셋을 계산하는 함수
    void Calculate_AO_Pitch();
    void SimProxyTurn(); // 조준중 회전값 전달을 매끄럽게 하는 함수

	UFUNCTION()
	void ReceiveDamage( AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser ); 

	void UpdateHUDHealth(); 

	void PollInit(); // 관련 클래스를 확인해서 HUD를 세팅

public:	
	ABaseCharacter();	
	virtual void Tick(float DeltaTime) override;
	virtual void PostInitializeComponents() override; // 이 함수는 BeginPlay() 전에 호출되며, 컴포넌트 초기화에 사용됩니다.
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; // 뱐수 복제 위한 함수 복제는 변수가 변했을떄만!
	virtual void OnRep_ReplicatedMovement() override; // ReplicatedMovement가 변경되었을 때 호출되는 함수
	virtual void Destroyed() override; // 액터가 파괴될 때 호출되는 함수

    virtual void Restart() override; // 매치 스타트 할 시 서버에서 입력이 안되는거 해결 위한 함수 

	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; } // AO_Yaw를 반환하는 함수
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; } // AO_Pitch를 반환하는 함수
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; } // TurningInPlace를 반환하는 함수
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; } // FollowCamera를 반환하는 함수
	// 이거 91 강의 무슨 애니메이션 흔들림 없앤다는데 좀 이상해서 안씀. 안써도 그리 이상하지 않음
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; } // bRotateRootBone을 반환하는 함수
	// 이거 91 강의 무슨 애니메이션 흔들림 없앤다는데 좀 이상해서 안씀. 안써도 그리 이상하지 않음
	FORCEINLINE bool GetCloseWall() const { return bCloseWall; } // bCloseWall을 반환하는 함수
	FORCEINLINE bool IsDead() const { return bDead; } // bDie를 반환하는 함수
	FORCEINLINE float GetHealth() const { return Health; } // Health를 반환하는 함수
	FORCEINLINE float GetMaxHealth() const { return MAxHealth; } // MaxHealth를 반환하는 함수
	FORCEINLINE UCombatComponent* GetCombatComponent() const { return CombatComponent; } // CombatComponent를 반환하는 함수

	bool IsWeaponEquipped();
	bool IsAiming(); // CombatComponent가 존재할 때만 bIsAiming을 반환
	void SetOverlappingWeapon(ABaseWeapon* Weapon);

	void PlayFireMontage(); // 무기 발사 애니메이션 몽타주를 재생하는 함수. 
	void PlayHitReactMontage(); // 적에게 맞았을 때 애니메이션 몽타주를 재생하는 함수
	void PlayDieMontage(); // 플레이어가 죽었을 때 애니메이션 몽타주를 재생하는 함수
	void PlayReloadMontage(); // 무기를 재장전할 때 애니메이션 몽타주를 재생하는 함수

	UFUNCTION( NetMulticast, Reliable )
	void MulticastEliminate(); // 플레이어가 제거되었을 때 호출되는 멀티캐스트 함수

	void Eliminate(); // 플레이어가 제거되었을 때 호출되는 서버에서만 실행하는 함수

	UPROPERTY( Replicated )
	bool bDisableGamePlay = false; // 게임 플레이를 비활성화하는 변수

	ECombatState GetCombatState() const;

	//UFUNCTION( NetMulticast, Unreliable ) // 이거는 전달 안되도 상관 없음
	//void MulticastPlayHitReactMontage(); // 적에게 맞았을 때 애니메이션 몽타주를 멀티캐스트로 재생하는 함수
	//void ABaseCharacter::MulticastPlayHitReactMontage_Implementation() { PlayHitReactMontage();} // 멀티캐스트로 애니메이션 몽타주를 재생합니다. 

	UFUNCTION( BlueprintImplementableEvent) // 이 함수는 블루프린트에서 구현됩니다.
	void ShowScopeSniperWidget( bool bShow ); // 스코프 위젯을 보이게 하는 함수

};
