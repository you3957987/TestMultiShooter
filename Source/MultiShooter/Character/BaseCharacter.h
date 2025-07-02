#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

UCLASS()
class MULTISHOOTER_API ABaseCharacter : public ACharacter
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

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class ABaseWeapon* OverlappingWeapon;
	UFUNCTION()
	void OnRep_OverlappingWeapon( ABaseWeapon* LastWeapon ); 
	// OverlappingWeapon이 변경되었을 때 호출되는 함 이 함수는 서버에서는 실행 안하고 클라이언트에서만 실행

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* CombatComponent; // 전투 컴포넌트

	UFUNCTION( Server, Reliable )
	void ServerEquipWeapon(); // 클라이언트에게 무기 장착하라고 지시하는 함수

	float AO_Pitch; // 캐릭터의 AO 피치를 저장합니다. 조준 시에 사용됩니다.
	float AO_Yaw; // 캐릭터의 AO 요 를 저장합니다. 조준 시에 사용됩니다.
	FRotator StartingAimRotation;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void Move(const FInputActionValue& Value);
	UFUNCTION()
	void Look(const FInputActionValue& Value);
	UFUNCTION()
	void Equip();
	UFUNCTION()
	void AimPressed();
	UFUNCTION()
	void AimReleased();

	void AimOffset(float DeltaTime); // 조준 오프셋을 계산하는 함수


public:	
	ABaseCharacter();	
	virtual void Tick(float DeltaTime) override;

	bool IsWeaponEquipped();
	bool IsAiming(); // CombatComponent가 존재할 때만 bIsAiming을 반환

	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; } // AO_Yaw를 반환하는 함수
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; } // AO_Pitch를 반환하는 함수

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; // 뱐수 복제 위한 함수 복제는 변수가 변했을떄만!

	void SetOverlappingWeapon(ABaseWeapon* Weapon);

	virtual void PostInitializeComponents() override; // 이 함수는 BeginPlay() 전에 호출되며, 컴포넌트 초기화에 사용됩니다.

};
