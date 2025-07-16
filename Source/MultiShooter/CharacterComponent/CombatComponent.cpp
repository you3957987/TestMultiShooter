#include "CombatComponent.h"
#include "MultiShooter/Weapon/BaseWeapon.h"
#include "MultiShooter/Character/BaseCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h" // 네트워크 복제를 위한 헤더 파일
#include "GameFramework/CharacterMovementComponent.h" // 캐릭터 이동 컴포넌트 헤더 파일
#include "Kismet/GameplayStatics.h" // 게임 관련 유틸리티 함수들을 사용하기 위한 헤더 파일
#include "DrawDebugHelpers.h" // 디버그용 선 그리기 함수들을 사용하기 위한 헤더 파일
#include "MultiShooter/PlayerController/BasePC.h" // 플레이어 컨트롤러 헤더 파일
//#include "MultiShooter/HUD/CharacterHUD.h" // HUD 관련 헤더 파일
#include "Camera/CameraComponent.h" // 카메라 컴포넌트 헤더 파일
#include "TimerManager.h" // 타이머 매니저 헤더 파일
#include "Sound/SoundCue.h" // 사운드 큐 헤더 파일
#include "MultiShooter/Weapon/Projectile.h" // 수류탄 클래스 헤더 파일
#include "Components/BoxComponent.h" // 박스 컴포넌트 헤더 파일
#include "MultiShooter/Weapon/ShotGun.h" // 산탄총 클래스 헤더 파일

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;


}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if( Character ) // 캐릭터가 존재하고, 캐릭터의 카메라가 존재하는 경우
    {
        if( Character->GetFollowCamera() )
        {
            DefaultFOV = Character->GetFollowCamera()->FieldOfView; // 기본 FOV를 캐릭터의 카메라에서 가져옵니다.
            CurrentFOV = DefaultFOV; // 현재 FOV를 기본 FOV로 설정
        }
        if( Character->HasAuthority() ) // 서버에서만 초기화
        {
            InitCarriedAmmo(); // 보유 총알 개수를 초기화합니다.
        }
    }
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if( Character && Character->IsLocallyControlled() ) // 로컬 플레이어만 FOV를 업데이트합니다.
    {
        FHitResult HitResult; // 십자선 아래의 물체를 추적하기 위한 히트 결과 변수
        TraceUnderCrosshair(HitResult); // 십자선 아래의 물체를 추적하는 함수 호출 이때 힛 리절트 값이 HitResult에 저장됩니다.
        HitTarget = HitResult.ImpactPoint; // 또는 HitResult.Location

        SetHUDCrosshair(DeltaTime); // 십자선 HUD를 설정합니다.
        InterpFOV(DeltaTime); // FOV를 업데이트합니다.
    }

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UCombatComponent, EquippedWeapon); // EquippedWeapon 변수를 네트워크에서 복제하도록 설정
    DOREPLIFETIME(UCombatComponent, bIsAiming); // bIsAiming 변수를 네트워크에서 복제하도록 설정
    DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);// CarriedAmmo 변수를 네트워크에서 복제하도록 설정
    DOREPLIFETIME(UCombatComponent, CombatState); // CombatState 변수를 네트워크에서 복제하도록 설정
    DOREPLIFETIME(UCombatComponent, Grenade); // Grenade 변수를 네트워크에서 복제하도록 설정
    DOREPLIFETIME(UCombatComponent, SecondaryWeapon); // SecondaryWeapon 변수를 네트워크에서 복제하도록 설정
}

void UCombatComponent::OnRep_EquippedWeapon()
{
    if (EquippedWeapon && Character )
    {
        EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equiped);
        EquippedWeapon->SetOwner(Character); // 복사 자동으로 됨
        EquippedWeapon->SetHUDAmmo(); // 무기의 HUD에 총알 수를 설정합니다.
        UpdateCarriedAmmo();
        UpdateWeaponIcon(); // HUD에 무기 아이콘을 업데이트합니다.
        if (CombatState != ECombatState::ECS_SwappingWeapons) // 무기 교체 중이 아닐 때만
        {
            PlayEquipSound(EquippedWeapon); // 무기 장착 사운드를 재생합니다.
            AttachWeaponToSocket(EquippedWeapon); // 무기를 소켓에 부착합니다. 
        }
        ReloadEmptyWeapon(); // 빈 무기를 리로드합니다.
        EquippedWeapon->EnableCustomDepth(false); // 윤곽선을 활성화합니다.

        Character->GetCharacterMovement()->bOrientRotationToMovement = false; // 캐릭터가 이동 방향을 바라보지 않도록 설정
        Character->bUseControllerRotationYaw = true; // 캐릭터
    }
}

void UCombatComponent::OnRep_SecondaryWeapon()
{
    if (SecondaryWeapon && Character )
    {
        SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
        SecondaryWeapon->SetOwner(Character); // 복사 자동으로 됨
        if (CombatState != ECombatState::ECS_SwappingWeapons) // 무기 교체 중이 아닐 때만
        {
            AttachWeaponToBackpack(SecondaryWeapon); // 무기를 백팩에 부착합니다.
            PlayEquipSound(SecondaryWeapon); // 무기 장착 사운드를 재생합니다.
        } 
    }
}

void UCombatComponent::OnRep_CarriedAmmo()
{
    ABasePC* Controller = Cast<ABasePC>(Character->GetController()); // 캐릭터의 플레이어 컨트롤러를 가져옵니다.
    if(Controller)
    {
        Controller->SetHUDCarriedAmmo( CarriedAmmo );
    }
}

void UCombatComponent::OnRep_CombatState()
{
    switch( CombatState )
    {
        case ECombatState::ECS_Unoccupied:
            if( bFireButtonPressed ) // 발사 버튼이 눌렸다면
            {
                Fire(); // 발사 함수를 호출합니다.
            }
            SetWeaponMesh(true); // 무기 메시 숨김
            break;
        case ECombatState::ECS_Reloading:
            // 리로드 중일 때 처리할 로직
            if( Character && !Character->IsLocallyControlled() )HandleReload(); // 리로드를 처리하는 함수를 호출합니다.
            break;
        case ECombatState::ECS_ThrowingGrenade:
            if(  Character && !Character->IsLocallyControlled() )// 
            {
                Character->PlayThrowGrenadeMontage(); // 수류탄을 던지는 함수를 호출합니다.
                SetWeaponMesh(false); // 무기 메시 숨김
                SetGrenadeMesh(true); // 수류탄 메시 보이게 (서버/클라 모두)
            }
        case ECombatState::ECS_SwappingWeapons:
            if( Character && Character->IsLocallyControlled() == false ) // 무기를 교체하는 중일 때
            {
                Character->PlaySwapMontage(); // 무기 교체 애니메이션을 재생합니다.
            }
			break;
        default:
            break;
    }
}

void UCombatComponent::OnRep_Grenade()
{
    UpdateGrenade(); // HUD에 수류탄 개수를 업데이트합니다.
}

void UCombatComponent::OnRep_Aiming() // 렉걸릴 시 클라 쪽에서 줌 광클 시 지연되는거 방지
{
    if (Character && Character->IsLocallyControlled())
    {
        bIsAiming = bAimButtonPressed; // Aim 버튼이 눌렸는지 여부를 설정합니다.
    }
}

void UCombatComponent::InitCarriedAmmo() // 무기 추가시 탄약도 추가
{
    // 무기 타입별로 초기 보유 총알 개수를 설정합니다.
    CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);   // 소총 시작 총알 개수
    CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo); // 로켓 런처 시작 총알 개수
    CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo); // 권총 시작 총알 개수
    CarriedAmmoMap.Emplace(EWeaponType::EWT_SMG, StartingSMGAmmo); // SMG 시작 총알 개수
    CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo); // 샷건 시작 총알 개수
    CarriedAmmoMap.Emplace(EWeaponType::EWT_Sniper, StartingSniperAmmo); // 스나이퍼 시작 총알 개수
    CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo); // 유탄 발사기 시작 총알 개수
    CarriedAmmoMap.Emplace(EWeaponType::EWT_MiniGun, StartingMinigunAmmo); // 미니건 시작 총알 개수
}

void UCombatComponent::PickUpAmmo(EWeaponType WeaponType, int32 Amount)
{
    if( CarriedAmmoMap.Contains(WeaponType) ) // 무기 타입에 해당하는 총알 개수가 있는지 확인
    {
        CarriedAmmoMap[WeaponType]  = FMath::Clamp(CarriedAmmoMap[WeaponType] + Amount, 0, MaxCarriedAmmo); // 총알 개수를 증가시키고 최대값을 초과하지 않도록 합니다.
        UpdateCarriedAmmo(); // HUD에 총알 개수를 업데이트합니다.
    }
    if( EquippedWeapon && EquippedWeapon->IsEmptyAmmo() && EquippedWeapon->GetWeaponType() == WeaponType ) // 장착된 무기가 있고, 총알이 비어있는 경우
    {
        Reload(); // 무기를 리로드합니다.
    }
}

void UCombatComponent::PickUpGrenade(int32 Amount)
{
    Grenade = FMath::Clamp(Grenade + Amount, 0, MaxGrenade); // 수류탄 개수를 증가시키고 최대값을 초과하지 않도록 합니다.
    UpdateGrenade(); // HUD에 수류탄 개수를 업데이트합니다.
}

void UCombatComponent::EquipWeapon(ABaseWeapon* WeaponToEquip) // 이 함수는 서버에서만 실행됨
{
    if (Character == nullptr || WeaponToEquip == nullptr) return;
    if( CombatState != ECombatState::ECS_Unoccupied ) return; // 무기를 장착할 수 없습니다. // 전투 상태가 비활성화 상태가 아닐 때

    if( EquippedWeapon != nullptr && SecondaryWeapon == nullptr ) // 이미 장착된 무기가 있고, 보조 무기가 없는 경우
    {
        EquipSecondaryWeapon(WeaponToEquip); // 보조 무기를 장착합니다.
    }
    else
    {
        EquipFirstWeapon(WeaponToEquip); // 기본 무기를 장착합니다.
    }

    Character->GetCharacterMovement()->bOrientRotationToMovement = false; // 캐릭터가 이동 방향을 바라보지 않도록 설정
    Character->bUseControllerRotationYaw = true; // 캐릭터 가 컨트롤러의 회전을 사용하도록 설정
}

void UCombatComponent::EquipFirstWeapon(ABaseWeapon* WeaponToEquip) // 기본 무기를 장착하는 함수
{
    if(WeaponToEquip == nullptr) return;

    DropEquippedWeapon(); // 현재 장착된 무기가 있다면 드롭합니다.
    EquippedWeapon = WeaponToEquip; // OnRep_EquippedWeapon 함수 실행

    EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equiped); 
    AttachWeaponToSocket(WeaponToEquip); // 무기를 소켓에 부착합니다.
    EquippedWeapon->SetOwner(Character); // 복사 자동으로 됨
    EquippedWeapon->SetHUDAmmo(); // 무기의 HUD에 총알 수를 설정합니다.
    UpdateCarriedAmmo(); // 보유 총알 개수를 업데이트합니다.
    UpdateWeaponIcon();
    PlayEquipSound(WeaponToEquip); // 무기 장착 사운드를 재생합니다.
    ReloadEmptyWeapon(); // 빈 무기를 리로드합니다.
    EquippedWeapon->EnableCustomDepth(false); // 윤곽선을 활성화합니다.
}
	
void UCombatComponent::EquipSecondaryWeapon(ABaseWeapon* WeaponToEquip) // 보조 무기를 장착하는 함수
{
    if(WeaponToEquip == nullptr) return;

    SecondaryWeapon = WeaponToEquip; // 보조 무기를 설정합니다. OnRep_SecondaryWeapon 함수가 호출됩니다.

    SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary); // 보조 무기의 상태를 장착됨으로 설정합니다.
    AttachWeaponToBackpack(WeaponToEquip); // 현재 장착된 무기를 백팩에 부착합니다.
    PlayEquipSound(WeaponToEquip); // 보조 무기 장착 사운드를 재생합니다.
    SecondaryWeapon->SetOwner(Character); // 보조 무기의 소유자를 캐릭터로 설정합니다.
}

void UCombatComponent::AttachWeaponToBackpack(ABaseWeapon* WeaponToAttach) // 무기를 백팩에 부착하는 함수
{
    if( Character == nullptr || WeaponToAttach == nullptr || Character->GetMesh() == nullptr ) return;

    const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName("BackpackSocket")); // 백팩 소켓을 가져옵니다.
    if(BackpackSocket)
    {
        BackpackSocket->AttachActor(WeaponToAttach, Character->GetMesh()); // 백팩 소켓에 무기를 부착합니다.
    }
}

void UCombatComponent::SwapWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied || Character == nullptr) return; 
    // 전투 상태가 비활성화 상태가 아닐 때 무기를 교체할 수 없습니다.

	Character->PlaySwapMontage(); // 무기 교체 애니메이션 몽타주를 재생합니다.
	CombatState = ECombatState::ECS_SwappingWeapons; // 전투 상태를 무기 교체 상태로 설정합니다.

    ABaseWeapon* TempWeapon = EquippedWeapon; // 현재 장착된 무기를 임시 변수에 저장합니다.
    EquippedWeapon = SecondaryWeapon; // 보조 무기를 현재 장착된 무기로 설정합니다.
    SecondaryWeapon = TempWeapon; // 임시 변수에 저장된 무기를 보조 무기로 설정합니다.
	if (SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(false); // 보조 무기의 윤곽선을 비활성화합니다.
}

bool UCombatComponent::ShouldSwapWeapon()
{
    return( EquippedWeapon != nullptr && SecondaryWeapon != nullptr ); // 현재 장착된 무기와 보조 무기가 다를 때 true를 반환합니다.
}

void UCombatComponent::UpdateCarriedAmmo()
{
    if (EquippedWeapon == nullptr) return;

    if( CarriedAmmoMap.Contains( EquippedWeapon->GetWeaponType() ) )
    {
        CarriedAmmo = CarriedAmmoMap[ EquippedWeapon->GetWeaponType() ]; // 무기 타입에 해당하는 보유 총알 개수를 설정합니다.
    }

    ABasePC* Controller = Cast<ABasePC>(Character->GetController()); // 캐릭터의 플레이어 컨트롤러를 가져옵니다.
    if(Controller)
    {
        Controller->SetHUDCarriedAmmo( CarriedAmmo );
    }
}

void UCombatComponent::UpdateWeaponIcon() // HUD에 무기 아이콘을 업데이트하는 함수
{
    ABasePC* Controller = Cast<ABasePC>(Character->GetController()); // 캐릭터의 플레이어 컨트롤러를 가져옵니다.
    if( Controller )
    {
        if( EquippedWeapon->WeaponIcon)
        {
            Controller->SetHUDWeaponIcon( EquippedWeapon->WeaponIcon ); // HUD에 무기 아이콘을 설정합니다.
        }
        else
        {
            Controller->SetHUDWeponIconClear(); // 무기 아이콘이 없으면 HUD에서 무기 아이콘을 지웁니다.
        }
    }
}

void UCombatComponent::DropEquippedWeapon()
{
    if( EquippedWeapon ) EquippedWeapon->Dropped(); // 이미 장착된 무기가 있다면 드롭합니다.
}

void UCombatComponent::AttachWeaponToSocket(ABaseWeapon* WeaponToEquip ) // 무기 추가시 각 무기마다 다르게 위치 지정해주어야 함!!!!!
{
    USkeletalMeshComponent* WeaponMesh = EquippedWeapon->GetWeaponMesh();
    if (WeaponMesh == nullptr) return;

    if (WeaponMesh && WeaponMesh->GetSkeletalMeshAsset() &&
        WeaponMesh->GetSkeletalMeshAsset()->GetName() == TEXT("AssaultRifle_SK"))
    {
        EquippedWeapon->AttachToComponent(
            Character->GetMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            FName("hand_rSocket") // 소켓 지정
        );
        WeaponMesh->SetRelativeLocation(FVector(-6.796413f, 4.446436f, 1.126907f));
        WeaponMesh->SetRelativeRotation(FRotator(0.f, 90.000206f, 0.000007f) ); // (Pitch=0.000007,Yaw=90.000206,Roll=0.000000)
    }
    else if (WeaponMesh && WeaponMesh->GetSkeletalMeshAsset() &&
        WeaponMesh->GetSkeletalMeshAsset()->GetName() == TEXT("Bazooka_SK"))
    {
        EquippedWeapon->AttachToComponent(
            Character->GetMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            FName("hand_rSocket") // 소켓 지정
        );
        WeaponMesh->SetRelativeLocation(FVector(-7.935278f, 2.721333f, 19.447058f));
        WeaponMesh->SetRelativeRotation(FRotator(0.f, 90.00016f, 0.f) ); // (Pitch=0.000007,Yaw=90.000206,Roll=0.000000)
    }
    else if (WeaponMesh && WeaponMesh->GetSkeletalMeshAsset() &&
        WeaponMesh->GetSkeletalMeshAsset()->GetName() == TEXT("Pistol_SK"))
    {
        EquippedWeapon->AttachToComponent(
            Character->GetMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            FName("hand_rSocket") // 소켓 지정
        );
        WeaponMesh->SetRelativeLocation(FVector(-5.796565f, 3.547575f, -0.468514f));
        WeaponMesh->SetRelativeRotation(FRotator(0.000046f, 90.000198f, 0.000034f) );
    }
    else if (WeaponMesh && WeaponMesh->GetSkeletalMeshAsset() &&
        WeaponMesh->GetSkeletalMeshAsset()->GetName() == TEXT("SMG_SK"))
    {
        EquippedWeapon->AttachToComponent(
            Character->GetMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            FName("hand_rSocket") // 소켓 지정
        );
        WeaponMesh->SetRelativeLocation(FVector(-8.003382f, 3.45388f, -3.464114f));
        WeaponMesh->SetRelativeRotation(FRotator(0.000046f, 90.000198f, 0.000034f) );
    }
    else if (WeaponMesh && WeaponMesh->GetSkeletalMeshAsset() &&
        WeaponMesh->GetSkeletalMeshAsset()->GetName() == TEXT("Shotgun_SK"))
    {
        EquippedWeapon->AttachToComponent(
            Character->GetMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            FName("hand_rSocket") // 소켓 지정
        );
        WeaponMesh->SetRelativeLocation(FVector(-6.928853f, 5.209436f, 10.59279f));
        WeaponMesh->SetRelativeRotation(FRotator(0.00002f, 90.000191f, 0.00002f) );
    }
    else if (WeaponMesh && WeaponMesh->GetSkeletalMeshAsset() &&
        WeaponMesh->GetSkeletalMeshAsset()->GetName() == TEXT("SniperRifle_SK"))
    {
        EquippedWeapon->AttachToComponent(
            Character->GetMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            FName("hand_rSocket") // 소켓 지정
        );
        WeaponMesh->SetRelativeLocation(FVector(-3.726387, 2.377795, -0.399829));
        WeaponMesh->SetRelativeRotation(FRotator(0.00002f, 90.000191f, 0.00002f) );
    }
    else if (WeaponMesh && WeaponMesh->GetSkeletalMeshAsset() &&
        WeaponMesh->GetSkeletalMeshAsset()->GetName() == TEXT("GravityGun_SK"))
    {
        EquippedWeapon->AttachToComponent(
            Character->GetMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            FName("hand_rSocket") // 소켓 지정
        );
        WeaponMesh->SetRelativeLocation(FVector(-8.923258f, 5.280822f, 2.778548f));
        WeaponMesh->SetRelativeRotation(FRotator(0.f, 90.000206f, 0.000007f) );
    }
    else if (WeaponMesh && WeaponMesh->GetSkeletalMeshAsset() &&
        WeaponMesh->GetSkeletalMeshAsset()->GetName() == TEXT("MiniGun_SK"))
    {
        EquippedWeapon->AttachToComponent(
            Character->GetMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            FName("hand_rSocket") // 소켓 지정
        );
        WeaponMesh->SetRelativeLocation(FVector(-10.5811f, 25.454687, -9.787731f));
        WeaponMesh->SetRelativeRotation(FRotator(0.f, 90.000206f, 0.f) );
    }
}

void UCombatComponent::PlayEquipSound( ABaseWeapon* WeaponToEquip  )
{
    if( WeaponToEquip && Character && WeaponToEquip->EquipSound ) // 무기 장착 사운드가 있고 캐릭터가 존재하는 경우
    {
        UGameplayStatics::PlaySoundAtLocation(
            this,
            WeaponToEquip->EquipSound,
            Character->GetActorLocation()
        ); // 무기 장착 사운드를 재생합니다.
    }
}

void UCombatComponent::ReloadEmptyWeapon()
{
    if( EquippedWeapon && EquippedWeapon->IsEmptyAmmo() )
    {
        Reload(); // 무기가 비어있으면 리로드 함수를 호출합니다.
    }
}

void UCombatComponent::Reload()
{
    if( CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && 
        EquippedWeapon && !EquippedWeapon->IsFull() && !bLocallyReloading) // 보유 총알이 
    {
        ServerReload(); // 서버에 리로드 요청을 보냅니다.
		HandleReload(); // 리로드를 처리하는 함수를 호출합니다.
		bLocallyReloading = true; // 로컬에서 리로드 중임을 표시합니다.
    }
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return; // 캐릭터나 장착된 무기가 없으면 함수 종료

    CombatState = ECombatState::ECS_Reloading; // 전투 상태를 리로드 중으로 설정합니다. -> on_rep_CombatState() 함수가 호출됩니다.
	if (!Character->IsLocallyControlled()) HandleReload(); // 서버에서 리로드를 처리합니다.
}

int32 UCombatComponent::AmountToReload()
{
    if( EquippedWeapon == nullptr ) return 0; // EquippedWeapon이 없으면 리로드할 총알 개수는 0

    int32 NeedAmmo = EquippedWeapon->GetMaxAmmo() - EquippedWeapon->GetAmmo(); // 무기가 최대 총알 개수에서 현재 총알 개수를 뺀 값이 필요합니다.

    if( CarriedAmmoMap.Contains( EquippedWeapon->GetWeaponType() ) )
    {
        int32 AmountCarried = CarriedAmmoMap[ EquippedWeapon->GetWeaponType() ]; // 무기 타입에 해당하는 보유 총알 개수를 가져옵니다.
        int32 LeastAmmo = FMath::Min(NeedAmmo, AmountCarried); // 필요한 총알 개수와 보유 총알 개수 중 최소값을 선택합니다.
        return FMath::Clamp(NeedAmmo, 0, LeastAmmo); // 최소값을 0과 필요한 총알 개수 사이로 제한합니다.
    }
    return 0;
}

void UCombatComponent::HandleReload()
{
    if (Character)
    {
        Character->PlayReloadMontage(); // 캐릭터의 리로드 애니메이션 몽타주를 재생합니다.
    }
}

void UCombatComponent::UpdateAmmoValues()
{
    if( Character == nullptr || EquippedWeapon == nullptr ) return;

    int32 ReloadAmount = AmountToReload(); // 리로드할 총알 개수를 계산합니다.
    if( CarriedAmmoMap.Contains( EquippedWeapon->GetWeaponType() ) )
    {
        CarriedAmmoMap[ EquippedWeapon->GetWeaponType() ] -= ReloadAmount; // 보유 총알 개수를 감소시킵니다.
        CarriedAmmo = CarriedAmmoMap[ EquippedWeapon->GetWeaponType() ]; // 현재 보유 총알 개수를 업데이트합니다.
    }

    ABasePC* Controller = Cast<ABasePC>(Character->GetController()); // 캐릭터의 플레이어 컨트롤러를 가져옵니다.
    if(Controller)
    {
        Controller->SetHUDCarriedAmmo( CarriedAmmo );
    }

    EquippedWeapon->AddAmmo( ReloadAmount ); // 무기에 리로드할 총알 개수를 추가합니다.
}

void UCombatComponent::SetAiming(bool IsAiming)
{
    if( Character == nullptr || EquippedWeapon == nullptr ) return; // 캐릭터가 없으면 함수 종료
    bIsAiming = IsAiming; // Aim 상태를 설정합니다.
    ServerSetAiming(IsAiming); // 서버에 Aim 상태를 설정합니다.

    if( Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Sniper ) // 로컬 플레이어만 FOV를 업데이트합니다.
    {
        Character->ShowScopeSniperWidget( bIsAiming ); // 스코프 위젯을 보이게 합니다.
    }
	if( Character->IsLocallyControlled() ) bAimButtonPressed = IsAiming; // Aim 버튼이 눌렸는지 여부를 설정합니다.
}

void UCombatComponent::ServerSetAiming_Implementation(bool IsAiming)
{
    bIsAiming = IsAiming; // Aim 상태를 설정합니다.
}

void UCombatComponent::Server_Fire_Implementation( const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
    Multicast_Fire( TraceHitTarget ); // 서버에서 발사 버튼이 눌렸을 때 멀티캐스트로 호출합니다.
}
// 치트 방지용으로 발사 속도 딜레이 검사 후 이상하면 킥하기
bool UCombatComponent::Server_Fire_Validate(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
    if (EquippedWeapon)
    {
		bool bNealyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f); // 발사 딜레이가 거의 같은지 확인합니다.
		return bNealyEqual; // 발사 딜레이가 거의 같으면 true를 반환합니다.
    }
	return true; // EquippedWeapon이 nullptr인 경우에도 true를 반환합니다.
}

void UCombatComponent::Multicast_Fire_Implementation( const FVector_NetQuantize& TraceHitTarget )
{
    if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return; // 로컬 플레이어가 아닌 경우 함수 종료

	LocalFire(TraceHitTarget); // 로컬 플레이어에서 발사 함수를 호출합니다.
}

void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTarget, float FireDelay)
{
    MulticastShotgunFire(TraceHitTarget);
}
//치트 방지용으로 발사 속도 딜레이 검사 후 이상하면 킥하기
bool UCombatComponent::ServerShotgunFire_Validate(const TArray<FVector_NetQuantize>& TraceHitTarget, float FireDelay)
{
    if (EquippedWeapon)
    {
        bool bNealyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f); // 발사 딜레이가 거의 같은지 확인합니다.
        return bNealyEqual; // 발사 딜레이가 거의 같으면 true를 반환합니다.
    }
    return true; // EquippedWeapon이 nullptr인 경우에도 true를 반환합니다.
}

void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTarget)
{
    if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return; // 로컬 플레이어가 아닌 경우 함수 종료

    LocalShoutgunFire(TraceHitTarget); // 로컬 플레이어에서 샷건 발사 함수를 호출합니다.
}

void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
    if (EquippedWeapon == nullptr) return; // EquippedWeapon이 없으면 함수 종료
    if (Character && CombatState == ECombatState::ECS_Unoccupied) // 캐릭터가 존재하고 전투 상태가 비활성화 상태인 경우
    {
        Character->PlayFireMontage(); // 캐릭터의 무기 발사 애니메이션 몽타주를 재생합니다.

        EquippedWeapon->Fire(TraceHitTarget); // EquippedWeapon의 Fire 함수를 호출합니다.
    }
}

void UCombatComponent::LocalShoutgunFire(const TArray<FVector_NetQuantize>& TraceHitTarget)
{
    if (EquippedWeapon == nullptr || Character == nullptr ) return; // EquippedWeapon이 없거나 캐릭터가 없는 경우 함수 종료

    if (CombatState == ECombatState::ECS_Unoccupied) // 캐릭터가 존재하고 전투 상태가 비활성화 상태인 경우
    {
        Character->PlayFireMontage(); // 캐릭터의 무기 발사 애니메이션 몽타주를 재생합니다.

        AShotGun* Shotgun = Cast<AShotGun>(EquippedWeapon); // 장착된 무기가 샷건인지 확인합니다.
        if (Shotgun)
        {
            Shotgun->FireShotgun(TraceHitTarget);
        }
    }
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
    bFireButtonPressed = bPressed; // 발사 버튼이 눌렸는지 여부를 설정합니다. 
    if( bFireButtonPressed && bIsAiming ) 
    {
        Fire();
    }
}

void UCombatComponent::Fire()
{
    if( CanFire() )
    {
        bCanFire = false;
        // FHitResult HitResult; // 십자선 아래의 물체를 추적하기 위한 히트 결과 변수
        // TraceUnderCrosshair(HitResult); // 십자선 아래의 물체를 추적하는 함수 호출 이때 힛 리절트 값이 HitResult에 저장됩니다.
        if (EquippedWeapon)
        {
            CrosshairShootingFactor = 0.75f;

            switch (EquippedWeapon->FireType)
            {
                case EFireType::EFT_Projectile: // 발사체 무기
                    FireProjectileWeapon(); // 발사체 무기를 발사하는 함수를 호출합니다.
                    break;
                case EFireType::EFT_HitScan: // 히트스캔 무기
                    FireHitscanWeapon(); // 히트스캔 무기를 발사하는 함수를 호출합니다.
                    break;
                case EFireType::EFT_Shotgun: // 샷건 무기
                    FireShotgun(); // 샷건 무기를 발사하는 함수를 호출합니다.
					break;
            }
        }
        StartFireTimer(); // 발사 타이머를 시작합니다.
    }

}

void UCombatComponent::FireProjectileWeapon()
{
    if (EquippedWeapon && Character)
    {
        HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget; // 산란을 사용할 경우 십자선 아래의 물체를 추적합니다.
        Server_Fire(HitTarget, EquippedWeapon->FireDelay); // 서버에 발사 버튼이 눌렸는지 여부를 알립니
        if( !Character->HasAuthority() )LocalFire(HitTarget); // 로컬 플레이어에서 발사 함수를 호출합니다.
    }
}

void UCombatComponent::FireHitscanWeapon() // 히트스캔 무기를 발사하는 함수
{
    if (EquippedWeapon && Character )
    {
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget; // 산란을 사용할 경우 십자선 아래의 물체를 추적합니다.
        Server_Fire(HitTarget, EquippedWeapon->FireDelay); // 서버에 발사 버튼이 눌렸는지 여부를 알립니
        if (!Character->HasAuthority())LocalFire(HitTarget); // 로컬 플레이어에서 발사 함수를 호출합니다.
    }
}

void UCombatComponent::FireShotgun() // 샷건 무기를 발사하는 함수
{
    AShotGun* Shotgun = Cast<AShotGun>(EquippedWeapon); // 장착된 무기가 샷건인지 확인합니다.
    if (Shotgun && Character)
    {
        TArray<FVector_NetQuantize> HitTargets;
        Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);
        if (!Character->HasAuthority())LocalShoutgunFire(HitTargets); // 로컬 플레이어에서 샷건 발사 함수를 호출합니다.
		ServerShotgunFire(HitTargets, EquippedWeapon->FireDelay); // 서버에 샷건 발사 버튼이 눌렸는지 여부를 알립니다.
    }
}

bool UCombatComponent::CanFire()
{
    if( EquippedWeapon == nullptr || Character == nullptr || 
		bLocallyReloading || Character->GetCombatState() == ECombatState::ECS_Reloading) return false; 
    // EquippedWeapon이 없거나 캐릭터가 없거나 로컬에서 리로드 중이거나 전투 상태가 리로드 중인 경우 발사할 수 없습니다.

    return !EquippedWeapon->IsEmptyAmmo() && bCanFire && CombatState == ECombatState::ECS_Unoccupied; // 무기가 비어있지 않거나 발사 가능 상태인 경우에만 발사 가능
}

void UCombatComponent::StartFireTimer()
{
    if( EquippedWeapon == nullptr || Character == nullptr ) return; // EquippedWeapon이 없으면 함수 종료

    Character->GetWorldTimerManager().SetTimer(
        FireTimer,
        this,
        &UCombatComponent::FireTimerFinished,
        EquippedWeapon->FireDelay, // 발사 딜레이 시간
        false // 반복하지 않음
    ); // 발사 타이머를 시작합니다.
}

void UCombatComponent::FireTimerFinished()
{
    if( EquippedWeapon == nullptr) return; // EquippedWeapon이 없으면 함수 종료

    bCanFire = true;
    if( bFireButtonPressed && EquippedWeapon->bAutoFire ) // 발사 버튼이 눌렸고, 자동 발사 무기인 경우
    {
        Fire(); // 발사 함수를 호출합니다.
    }
    ReloadEmptyWeapon(); // 빈 무기를 리로드합니다.
}

void UCombatComponent::TraceUnderCrosshair(FHitResult& TraceHitResult)
{
    FVector2D ViewportSize = FVector2D::ZeroVector;
    if(  GEngine && GEngine->GameViewport )
    {
        GEngine->GameViewport->GetViewportSize(ViewportSize); // 뷰포트 크기를 가져옵니다.
    }

    FVector2D CrosshairLocation(  ViewportSize.X / 2.f, ViewportSize.Y / 2.f); // 십자선 위치를 뷰포트 중앙으로 설정
    FVector CrosshairWorldLocation, CrosshairWorldDirection;

    bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld( // 화면 좌표를 월드 좌표로 변환
        UGameplayStatics::GetPlayerController(this, 0), // 캐릭터의 컨트롤러를 사용하여 화면 좌표를 월드 좌표로 변환
        CrosshairLocation, // 십자선 위치
        CrosshairWorldLocation, // 변환된 월드 위치
        CrosshairWorldDirection // 변환된 월드 방향
    );

    if( bScreenToWorld )
    {
        FVector Start = CrosshairWorldLocation; // 시작 위치는 십자선 월드 위치

        if( Character )
        {
            float DistanceToCharacter = (Character->GetActorLocation() - Start).Size(); // 캐릭터와 십자선 사이의 거리
            Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f ); // 시작 위치를 캐릭터와 십자선 사이의 거리의 50 단위 만큼 앞으로 이동
        }

        FVector End = Start + (CrosshairWorldDirection * TRACE_LENGTH); // 끝 위치는 십자선 월드 방향으로 80000 단위 떨어진 위치

        // 자기 자신 무시 파라미터 추가
        FCollisionQueryParams TraceParams;
        if (Character)
        {
            TraceParams.AddIgnoredActor(Character);
        }
        
        // 라인 트레이스를 수행하여 십자선 아래의 물체를 추적
        bool bHit = GetWorld()->LineTraceSingleByChannel(
            TraceHitResult,
            Start,
            End,
            ECollisionChannel::ECC_Visibility
        );
        if (!bHit)
        {
            TraceHitResult.ImpactPoint = End; // 트레이스 실패 시 정면 먼 곳으로
        }
        if( TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UCrosshairInterface>() ) // 히트된 액터가 있는 경우와 인터페이스가 구현된 경우
        {
            HUDPackage.CrosshairColor = FLinearColor::Red; // 십자선 색상을 빨간색으로 설정
        }
        else // 히트된 액터가 없거나 인터페이스가 구현되지 않은 경우
        {
            HUDPackage.CrosshairColor = FLinearColor::White; // 십자선 색상을 흰색으로 설정
        }
    }

}

void UCombatComponent::SetHUDCrosshair(float DeltaTime)
{
    if( Character == nullptr || Character->Controller == nullptr ) return;

    PlayerController = PlayerController == nullptr ? Cast<ABasePC>(Character->Controller) : PlayerController; // 플레이어 컨트롤러를 가져옵니다.
    if( PlayerController == nullptr ) return;

    CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(PlayerController->GetHUD()) : CharacterHUD; // 캐릭터 HUD를 가져옵니다.
    if( CharacterHUD == nullptr ) return;

    if(  EquippedWeapon )
    {
        HUDPackage.CrosshairCenter = EquippedWeapon->CrosshairCenter; // 십자선 중앙 텍스처
        HUDPackage.CrosshairLeft = EquippedWeapon->CrosshairLeft; // 십자선 왼쪽 텍스처
        HUDPackage.CrosshairRight = EquippedWeapon->CrosshairRight; // 십자선 오른쪽 텍스처
        HUDPackage.CrosshairTop = EquippedWeapon->CrosshairTop; // 십자선 위쪽 텍스처
        HUDPackage.CrosshairBottom = EquippedWeapon->CrosshairBottom; // 십자선 아래쪽 텍스처

    }
    else
    {
        HUDPackage.CrosshairCenter =nullptr; // 십자선 중앙 텍스처
        HUDPackage.CrosshairLeft = nullptr; // 십자선 왼쪽 텍스처
        HUDPackage.CrosshairRight = nullptr; // 십자선 오른쪽 텍스처
        HUDPackage.CrosshairTop = nullptr; // 십자선 위쪽 텍스처
        HUDPackage.CrosshairBottom = nullptr; // 십자선 아래쪽 텍스처
    }

    // 십자선 확산 정도를 계산합니다.
    // 최대 걷기 속도는 0~600 -> 0~1로 매핑됩니다.
    float WalkSpeed = Character->GetVelocity().Size2D(); // XY 평면 속도(이동 속도)

    CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(
        FVector2D(0.f, Character->GetCharacterMovement()->MaxWalkSpeed),
        FVector2D(0.f, 1.f),
        WalkSpeed
    );
    
    if( Character->GetCharacterMovement()->IsFalling() ) // 캐릭터가 낙하 중일 때
    {
        CrosshairInAirFactor = FMath::FInterpTo(
            CrosshairInAirFactor, // 현재 십자선 공중 계수
            2.25f, // 목표 값
            DeltaTime, // 델타 시간
            2.25f // 보간 속도
        );
    }
    else // 캐릭터가 낙하 중이 아닐 때
    {
        CrosshairInAirFactor = FMath::FInterpTo(
            CrosshairInAirFactor, // 현재 십자선 공중 계수
            0.f, // 목표 값
            DeltaTime, // 델타 시간
            30.f // 보간 속도
        );
    }

    if( bIsAiming ) // Aim 상태일 때
    {
        CrosshairAimFactor = FMath::FInterpTo(
            CrosshairAimFactor, // 현재 십자선 Aim 계수
            0.5f, // 목표 값
            DeltaTime, // 델타 시간
            30.f // 보간 속도
        );
    }
    else // Aim 상태가 아닐 때
    {
        CrosshairAimFactor = FMath::FInterpTo(
            CrosshairAimFactor, // 현재 십자선 Aim 계수
            0.f, // 목표 값
            DeltaTime, // 델타 시간
            30.f // 보간 속도
        );
    }

    CrosshairShootingFactor = FMath::FInterpTo(
        CrosshairShootingFactor, // 현재 십자선 사격 계수
        0.f, // 목표 값
        DeltaTime, // 델타 시간
        40.f // 보간 속도
    );

    HUDPackage.CrosshairSpread = 
        0.5f  // 기본 십자선 확산 정도
        + CrosshairVelocityFactor 
        + CrosshairInAirFactor 
        - CrosshairAimFactor
        + CrosshairShootingFactor;  // firebuttompressed 함수해서 버튼 눌리면 증가
    // 십자선 확산 정도를 계산합니다.


    CharacterHUD->SetHUDPackage(HUDPackage); // 캐릭터 HUD에 십자선 패키지를 설정합니다.
}

void UCombatComponent::InterpFOV(float DeltaTime)
{
    if( Character == nullptr || PlayerController == nullptr || EquippedWeapon == nullptr ) return;

    if( bIsAiming )
    {
        CurrentFOV = FMath::FInterpTo(
            CurrentFOV, // 현재 FOV
            EquippedWeapon->GetZoomedFOV(), // 목표 FOV (기본 FOV에서 25 감소)
            DeltaTime, // 델타 시간
            EquippedWeapon->GetZoomInterpSpeed() // 보간 속도
        );
    }
    else
    {
        CurrentFOV = FMath::FInterpTo(
            CurrentFOV, // 현재 FOV
            DefaultFOV, // 목표 FOV (기본 FOV)
            DeltaTime, // 델타 시간
            EquippedWeapon->GetZoomInterpSpeed() // 보간 속도
        );
    }

    if( Character->GetFollowCamera() )
    {
        Character->GetFollowCamera()->SetFieldOfView(CurrentFOV); // 카메라의 FOV를 현재 FOV로 설정합니다.
    }
    
}

void UCombatComponent::ClearEquippedWeapon()
{
    EquippedWeapon = nullptr;
}

void UCombatComponent::SetWeaponMesh( bool bShow )
{
    if( EquippedWeapon == nullptr || Character == nullptr ) return; // EquippedWeapon이 없으면 함수 종료

    if( bShow ) // 수류탄을 던지지 않는 경우
    {
        EquippedWeapon->SetActorHiddenInGame(false); // 무기를 보이게 합니다.
    }
    else // 수류탄을 던지는 경우
    {
        EquippedWeapon->SetActorHiddenInGame(true); // 무기를 숨깁니다.
    }
}

void UCombatComponent::SetGrenadeMesh( bool bShow )
{
    if( Character && Character->GetGrenadeMesh() ) // 캐릭터가 존재하고, 캐릭터의 수류탄 메시가 존재하는 경우
    {
        Character->GetGrenadeMesh()->SetVisibility(bShow); // 수류탄 메시의 가시성을 설정합니다.
    }
}

void UCombatComponent::FireGrenade()
{
    SetGrenadeMesh(false); // 수류탄 메시 숨김 (서버/클라 모두)
    if( Character && Character->IsLocallyControlled() ) // 로컬 플레이어만 수류탄을 던집니다.
    {
        Server_FireGrenade(HitTarget); // 서버에 수류탄 투척 요청을 보냅니다. 
    } 
}

void UCombatComponent::Server_FireGrenade_Implementation( const FVector_NetQuantize& Target )
{
    if( Character && GrenadeClass && Character->GetGrenadeMesh() ) // 서버에서만 실행
    {
        const FVector StartingLocation = Character->GetGrenadeMesh()->GetComponentLocation(); // 수류탄 시작 위치
        FVector ToTarget = Target - StartingLocation; // 타겟 위치에서 시작 위치를 뺀 벡터
        FActorSpawnParameters SpawnParams; // 액터 스폰 파라미터
        SpawnParams.Owner = Character; // 스폰할 액터의 소유자를 캐릭터로 설정
        SpawnParams.Instigator = Character; // 스폰할 액터의 인스티게이터를 캐릭터로 설정

        UWorld* World = GetWorld(); // 현재 월드를 가져옵니다.
        if( World )
        {
            // 수류탄을 스폰합니다.
            AProjectile* SpawnGrenade = World->SpawnActor<AProjectile>(
                GrenadeClass,
                StartingLocation,
                ToTarget.Rotation(), // 타겟 방향으로 회전
                SpawnParams
            );
            if (SpawnGrenade)
            {
                // 내 캐릭터를 충돌 무시 대상으로 등록
                if (SpawnGrenade->GetCollisionBox())
                {
                    SpawnGrenade->GetCollisionBox()->IgnoreActorWhenMoving(Character, true);
                }
            }
        }
    }
}

void UCombatComponent::FinshedReload()
{
    if (Character == nullptr) return;

    bLocallyReloading = false; // 로컬에서 리로드 중임을 표시합니다.
    if (Character->HasAuthority())
    {
        CombatState = ECombatState::ECS_Unoccupied; // 전투 상태를 비활성화 상태로 설정합니다. -> on_rep_CombatState() 함수가 호출됩니다.
        UpdateAmmoValues(); // HUD에 총알 개수를 업데이트합니다. -> 애님 노티파이로 실행
    }
    if (bFireButtonPressed)
    {
        Fire(); // 발사 버튼이 눌렸다면 발사 함수를 호출합니다.
    }
}

void UCombatComponent::FinishedThrowGrenade()
{
    CombatState = ECombatState::ECS_Unoccupied; // 전투 상태를 비활성화 상태로 설정합니다. -> on_rep_CombatState() 함수가 호출됩니다.
    SetWeaponMesh(true); // 무기 메시 다시 보이게 (서버/클라 모두)
}

void UCombatComponent::FinisheSwap()
{
    if (Character && Character->HasAuthority())
    {
		CombatState = ECombatState::ECS_Unoccupied; // 전투 상태를 비활성화 상태로 설정합니다. -> on_rep_CombatState() 함수가 호출됩니다.
    }
	if (SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(true); // 보조 무기를 커스텀 깊이로 활성화합니다.
}

void UCombatComponent::FinishedSwapAttachWeapon()
{
    EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equiped); // 장착된 무기의 상태를 설정합니다.
    AttachWeaponToSocket(EquippedWeapon); // 장착된 무기를 소켓에 부착합니다.
    EquippedWeapon->SetHUDAmmo(); // 장착된 무기의 HUD에 총알 수를 설정합니다.
    UpdateCarriedAmmo(); // 보유 총알 개수를 업데이트합니다.
    UpdateWeaponIcon(); // HUD에 무기 아이콘을 업데이트합니다.
    PlayEquipSound(EquippedWeapon); // 무기 장착 사운드를 재생합니다.
    SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary); // 보조 무기의 상태를 장착됨으로 설정합니다.
    AttachWeaponToBackpack(SecondaryWeapon); // 현재 장착된 무기를 백팩에 부착합니다.
}

void UCombatComponent::ThrowGrenade()
{
    if( Grenade <= 0 ) return; // 수류탄이 없으면 던질 수 없습니다.
    if( CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr ) return; // 전투 상태가 비활성화 상태가 아닐 때 수류탄을 던질 수 없습니다.


    CombatState = ECombatState::ECS_ThrowingGrenade; // 전투 상태를 수류탄 투척 상태로 -> on_rep_CombatState() 함수가 호출됩니다.
    SetWeaponMesh(false); // 무기 메시 숨김 (서버/클라 모두)
    if(  Character ) // 서버에서만 실행
    {
        Character->PlayThrowGrenadeMontage(); // 수류탄을 던지는 함수를 호출합니다.
        SetGrenadeMesh(true); // 수류탄 메시 보이게 (서버/클라 모두)
    }
    if( Character && !Character->HasAuthority() )
    {
        ServerThrowGrenade(); // 서버에 수류탄 투척 요청을 보냅니다.
    }
    if( Character && Character->HasAuthority() )
    {
        Grenade = FMath::Clamp(Grenade - 1, 0, 5); // 수류탄 개수를 감소시킵니다. 최소 0, 최대 5로 제한합니다.
        UpdateGrenade(); // HUD에 수류탄 개수를 업데이트합니다.
    }
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
    if( Grenade <= 0 ) return; // 수류탄이 없으면 던질 수 없습니다.

    CombatState = ECombatState::ECS_ThrowingGrenade; // 전투 상태를 수류탄 투척 상태로 -> on_rep_CombatState() 함수가 호출됩니다.
    SetWeaponMesh(false); // 무기 메시 숨김 (서버/클라 모두)
    if(  Character ) // 서버에서만 실행
    {
        Character->PlayThrowGrenadeMontage(); // 수류탄을 던지는 함수를 호출합니다.
        SetGrenadeMesh(true); // 수류탄 메시 보이게 (서버/클라 모두)
    }
    Grenade = FMath::Clamp(Grenade - 1, 0, 5); // 수류탄 개수를 감소시킵니다. 최소 0, 최대 5로 제한합니다.
    UpdateGrenade(); // HUD에 수류탄 개수를 업데이트합니다.
}

void UCombatComponent::UpdateGrenade()
{
    PlayerController = PlayerController == nullptr ? Cast<ABasePC>(Character->Controller) : PlayerController; // 플레이어 컨트롤러를 가져옵니다.
    if( PlayerController )
    {
        PlayerController->SetHUDGrenade(Grenade); // HUD에 수류탄 개수를 설정합니다.
    }
}

