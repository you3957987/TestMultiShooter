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

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;


}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if( Character )
    {
        DefaultFOV = Character->GetFollowCamera()->FieldOfView; // 기본 FOV를 캐릭터의 카메라에서 가져옵니다.
        CurrentFOV = DefaultFOV; // 현재 FOV를 기본 FOV로 설정
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

void UCombatComponent::Server_Fire_Implementation( const FVector_NetQuantize& TraceHitTarget )
{
    Multicast_Fire( TraceHitTarget ); // 서버에서 발사 버튼이 눌렸을 때 멀티캐스트로 호출합니다.
}

void UCombatComponent::Multicast_Fire_Implementation( const FVector_NetQuantize& TraceHitTarget )
{
    if( EquippedWeapon == nullptr ) return; // EquippedWeapon이 없으면 함수 종료
    if( Character )
    {
        Character->PlayFireMontage(); // 캐릭터의 무기 발사 애니메이션 몽타주를 재생합니다.

        EquippedWeapon->Fire(TraceHitTarget); // EquippedWeapon의 Fire 함수를 호출합니다.
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
    if( bCanFire )
    {
        bCanFire = false;
        // FHitResult HitResult; // 십자선 아래의 물체를 추적하기 위한 히트 결과 변수
        // TraceUnderCrosshair(HitResult); // 십자선 아래의 물체를 추적하는 함수 호출 이때 힛 리절트 값이 HitResult에 저장됩니다.
        Server_Fire(HitTarget); // 서버에 발사 버튼이 눌렸는지 여부를 알립니다.

        if (EquippedWeapon)
        {
            CrosshairShootingFactor = 0.75f;
        }
        StartFireTimer(); // 발사 타이머를 시작합니다.
    }

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

        FVector End = Start + (CrosshairWorldDirection * 80000.f); // 끝 위치는 십자선 월드 방향으로 80000 단위 떨어진 위치

        // 자기 자신 무시 파라미터 추가
        FCollisionQueryParams TraceParams;
        if (Character)
        {
            TraceParams.AddIgnoredActor(Character);
        }
        
        // 라인 트레이스를 수행하여 십자선 아래의 물체를 추적
        GetWorld()->LineTraceSingleByChannel(
            TraceHitResult,
            Start,
            End,
            ECC_Visibility, // 가시성  
            TraceParams 
        );
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

void UCombatComponent::SetHUDCrosshair(float DeltaTime)
{
    if( Character == nullptr || Character->Controller == nullptr ) return;

    PlayerController = PlayerController == nullptr ? Cast<APlayerController>(Character->GetController()) : PlayerController; // 플레이어 컨트롤러를 가져옵니다.
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



