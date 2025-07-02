#include "CombatComponent.h"
#include "MultiShooter/Weapon/BaseWeapon.h"
#include "MultiShooter/Character/BaseCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h" // 네트워크 복제를 위한 헤더 파일
#include "GameFramework/CharacterMovementComponent.h" // 캐릭터 이동 컴포넌트 헤더 파일

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;


}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UCombatComponent::EquipWeapon(ABaseWeapon* WeaponToEquip) // 이 함수는 서버에서만 실행됨
{
    if (Character == nullptr || WeaponToEquip == nullptr) return;

    EquippedWeapon = WeaponToEquip;
    EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equiped); 

    USkeletalMeshComponent* WeaponMesh = WeaponToEquip->GetWeaponMesh();
    if (WeaponMesh == nullptr) return;

    if (WeaponMesh && WeaponMesh->GetSkeletalMeshAsset() &&
        WeaponMesh->GetSkeletalMeshAsset()->GetName() == TEXT("AssaultRifle_SK"))
    {
        WeaponToEquip->AttachToComponent(
            Character->GetMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            FName("hand_rSocket") // 소켓 지정
        );
        WeaponMesh->SetRelativeLocation(FVector(-6.796413f, 4.446436f, 1.126907f));
        WeaponMesh->SetRelativeRotation(FRotator(0.f, 90.000206f, 0.000007f) ); // (Pitch=0.000007,Yaw=90.000206,Roll=0.000000)
    }

    EquippedWeapon->SetOwner(Character); // 복사 자동으로 됨
    Character->GetCharacterMovement()->bOrientRotationToMovement = false; // 캐릭터가 이동 방향을 바라보지 않도록 설정
    Character->bUseControllerRotationYaw = true; // 캐릭터

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UCombatComponent, EquippedWeapon); // EquippedWeapon 변수를 네트워크에서 복제하도록 설정
    DOREPLIFETIME(UCombatComponent, bIsAiming); // bIsAiming 변수를 네트워크에서 복제하도록 설정
}

void UCombatComponent::SetAiming(bool IsAiming)
{
    bIsAiming = IsAiming; // Aim 상태를 설정합니다.
    ServerSetAiming(IsAiming); // 서버에 Aim 상태를 설정합니다.
}

void UCombatComponent::ServerSetAiming_Implementation(bool IsAiming)
{
    bIsAiming = IsAiming; // Aim 상태를 설정합니다.
}

void UCombatComponent::OnRep_EquippedWeapon()
{
    if (EquippedWeapon && Character )
    {
        Character->GetCharacterMovement()->bOrientRotationToMovement = false; // 캐릭터가 이동 방향을 바라보지 않도록 설정
        Character->bUseControllerRotationYaw = true; // 캐릭터
    }
}

