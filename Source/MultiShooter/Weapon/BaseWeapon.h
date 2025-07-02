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
	
protected:
	virtual void BeginPlay() override;

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

	void SetWeaponState(EWeaponState NewState); // 무기 상태를 설정하는 함수

	UFUNCTION(BlueprintCallable)
    FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; } // 무기 메쉬 컴포넌트를 반환하는 함수
	FORCEINLINE USphereComponent* GetCollisionSphere() const { return CollisionSphere; } // 충돌 구 컴포넌트를 반환하는 함수

};