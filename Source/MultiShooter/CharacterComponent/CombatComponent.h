#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTISHOOTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

	UPROPERTY( ReplicatedUsing = OnRep_EquippedWeapon )
	class ABaseWeapon* EquippedWeapon; // 현재 장착된 무기
	class ABaseCharacter* Character; // 이 컴포넌트를 소유한 캐릭터

	UPROPERTY( Replicated )
	bool bIsAiming; // Aim 상태를 나타내는 변수 이거는 캐릭터한테 받아옴

protected:
	virtual void BeginPlay() override;

	void SetAiming(bool IsAiming); // Aim 상태를 설정하는 함수
	UFUNCTION( Server, Reliable )
	void ServerSetAiming(bool IsAiming); // 서버에서 Aim 상태를 설정하는 함수

	UFUNCTION()
	void OnRep_EquippedWeapon(); // 무기가 변경되었을 때 호출되는 함수

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	friend class ABaseCharacter; // ABaseCharacter 클래스가 이 컴포넌트의 private 멤버에 접근할 수 있도록 허용

	void EquipWeapon(class ABaseWeapon* WeaponToEquip); // 무기를 장착하는 함수

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override; // 뱐수 복제 위한 함수 복제는 변수가 변했을떄만!
};
