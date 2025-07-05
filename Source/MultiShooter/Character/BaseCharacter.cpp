#include "BaseCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h" // UCapsuleComponent 사용을 위한 include
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h" // 네트워크 복제를 위한 헤더 파일
#include "MultiShooter/Weapon/BaseWeapon.h" // ABaseWeapon 클래스 사용을 위한 헤더 파일
#include "MultiShooter/CharacterComponent/CombatComponent.h" // UCombatComponent 클래스 사용을 위한 헤더 파일
#include "Kismet/KismetMathLibrary.h"
#include "BaseAnimInstance.h" // ABaseAnimInstance 클래스 사용을 위한 헤더 파일
#include "MultiSHooter/MultiShooter.h" // MultiShooter 헤더 파일을 포함합니다. -> 스켈레탈 콜리전 정의 가져오기
#include "MultiShooter/PlayerController/BasePC.h" // ABasePC 클래스 사용을 위한 헤더 파일
#include "MultiShooter/GameMode/MainGameMode.h" // AMainGameMode 클래스 사용을 위한 헤더 파일
#include "TimerManager.h" // 타이머 매니저 사용을 위한 헤더 파일

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	//SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn; 
	// 스폰 충돌 처리 방법 설정 스폰 설정시 겹쳐도 항상 스폰되도록

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 600.0f; // 한글로 카메라가 캐릭터 뒤에서 이 거리를 유지합니다.
	CameraBoom->bUsePawnControlRotation = true; // 컨트롤러 회전 사용

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);
	FollowCamera->bUsePawnControlRotation = false; // 컨트롤러 회전 사용 안함

	bUseControllerRotationYaw = false; // 캐릭터가 컨트롤러 방향으로 회전하게 만들기
	GetCharacterMovement()->bOrientRotationToMovement = true; // 캐릭터가 이동 방향을 향하도록 설정

	OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHeadWidget"));
	OverHeadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent")); // 전투 컴포넌트 생성
	CombatComponent->SetIsReplicated(true); // 전투 컴포넌트도 네트워크에서 복제되도록 설정

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore); // 캡슐 컴포넌트가 카메라 채널에 반응하지 않도록 설정
	GetMesh()->SetCollisionObjectType( ECC_SkeletalMesh ); // 메쉬의 충돌 오브젝트 타입을 스켈레탈 메쉬로 설정
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore); // 메쉬가 카메라 채널에 반응하지 않도록 설정
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); // 메쉬가 트레이스 채널에 반응하지 않도록 설정
	//GetCharacterMovement()->RotationRate = FRotator(0.f, 850.f, 0.f); // Yaw(좌우 회전) 속도 850도/초 이거 안쓰는게 좋은듯

	TurningInPlace = ETurningInPlace::ETIP_NotTurning; // 초기 좌우 회전 상태 설정

	SetNetUpdateFrequency(66.f);      // 네트워크 업데이트 빈도 설정
	SetMinNetUpdateFrequency(33.f);   // 최소 네트워크 업데이트 빈도 설정

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimeline")); // 디졸브 타임라인 컴포넌트 생성

}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            if (DefaultMappingContext)
            {
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }
	UpdateHUDHealth();

	if( HasAuthority() )
	{
		OnTakeAnyDamage.AddDynamic(this, &ABaseCharacter::ReceiveDamage); // 피해를 받았을 때 호출되는 함수 바인딩/ 어플라이 대미지는 프로젝타일에서!
		// 이 함수는 서버에서만 실행되므로 변수 복제시 클라에서도 실행 되게 해야함!
	}
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	/*
	if( GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled() ) 
	{
		AimOffset(DeltaTime); // AimOffset 함수 호출
	}
	else
	{
		TimeSinceLastMovementReplicattion += DeltaTime;
		if (TimeSinceLastMovementReplicattion > 0.25f) // 0.25초마다 이동을 복제합니다.
		{
			OnRep_ReplicatedMovement(); // SimProxyTurn 함수를 호출하여 회전 처리
		}
		Calculate_AO_Pitch(); // AO 피치를 계산합니다.
	}
	*/

	AimOffset(DeltaTime); // AimOffset 함수 호출
	HideCameraIfCharacterClose(); // 캐릭터가 카메라에 너무 가까이 있으면 카메라 숨기기
}

void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInput->BindAction(IA_Jump, ETriggerEvent::Started, this, &ACharacter::Jump);
        EnhancedInput->BindAction(IA_Move, ETriggerEvent::Triggered, this, &ABaseCharacter::Move);
        EnhancedInput->BindAction(IA_Look, ETriggerEvent::Triggered, this, &ABaseCharacter::Look);
        EnhancedInput->BindAction(IA_Equip, ETriggerEvent::Started, this, &ABaseCharacter::Equip);
		EnhancedInput->BindAction(IA_Aim, ETriggerEvent::Started, this, &ABaseCharacter::AimPressed);
		EnhancedInput->BindAction(IA_Aim, ETriggerEvent::Completed, this, &ABaseCharacter::AimReleased);
		EnhancedInput->BindAction(IA_Fire, ETriggerEvent::Started, this, &ABaseCharacter::FirePressed);
		EnhancedInput->BindAction(IA_Fire, ETriggerEvent::Completed, this, &ABaseCharacter::FireReleased);
    }
}

void ABaseCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    if (Controller == nullptr) return;

    // 컨트롤러의 회전값(Yaw만 사용)
    const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);

    // Forward/Backward
    if (MovementVector.Y != 0.f)
    {
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(ForwardDirection, MovementVector.Y);
    }

    // Left/Right
    if (MovementVector.X != 0.f)
    {
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void ABaseCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();
    AddControllerYawInput(LookAxisVector.X);
    AddControllerPitchInput(LookAxisVector.Y);
}

void ABaseCharacter::Equip()
{
    if(  CombatComponent  ) // CombatComponent가 존재하고,
	{
		if( HasAuthority() )
		{
			CombatComponent->EquipWeapon(OverlappingWeapon); // CombatComponent의 EquipWeapon 함수를 호출하여 무기를 장착합니다.
		}
		else // 서버 권한이 없는 경우
		{
			ServerEquipWeapon(); // SetOverlappingWeapon 함수를 호출하여 서버에 무기 장착을 요청합니다.
		}

		if (IsLocallyControlled())
        {
            StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
        }
	}
}

void ABaseCharacter::AimPressed()
{
	if (CombatComponent) // CombatComponent가 존재하는 경우
	{
		CombatComponent->SetAiming(true); // Aim 상태를 true로 설정
	}
}

void ABaseCharacter::AimReleased()
{
	if (CombatComponent) // CombatComponent가 존재하는 경우
	{
		CombatComponent->SetAiming(false); // Aim 상태를 false로 설정
	}
}

void ABaseCharacter::FirePressed()
{
	if (CombatComponent) // CombatComponent가 존재하는 경우
	{
		CombatComponent->FireButtonPressed(true); // 발사 버튼이 눌렸음을 CombatComponent에 알립니다.
	}
}

void ABaseCharacter::FireReleased()
{
	if (CombatComponent) // CombatComponent가 존재하는 경우
	{
		CombatComponent->FireButtonPressed(false); // 발사 버튼이 떼어졌음을 CombatComponent에 알립니다.
	}
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// OverlappingWeapon 변수를 네트워크에서 복제합니다.
	DOREPLIFETIME_CONDITION(ABaseCharacter, OverlappingWeapon, COND_OwnerOnly); // 이 변수는 오직 소유자에게만 복제됩니다.
	DOREPLIFETIME(ABaseCharacter, Health); // CurrentHealth 변수를 네트워크에서 복제합니다.

}

void ABaseCharacter::OnRep_OverlappingWeapon( ABaseWeapon* LastWeapon ) // 이 함수는 클라이언트에서만 실행 되므로 서버용도 필요
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickUpWidget(true); // 위젯을 보이게 합니다.
	}
	if( LastWeapon ) // 이전 무기가 있다면. 즉 복제되는 변수 이전에 가지고 있던 무기
	{
		LastWeapon->ShowPickUpWidget(false); // 이전 무기의 위젯을 숨깁니다.
	}
}

void ABaseCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage(); // 멀티캐스트로 애니메이션 몽타주를 재생합니다.
}

void ABaseCharacter::SetOverlappingWeapon(ABaseWeapon* Weapon) // 겹치면 무기, 아니면 널값 받아옴. 이 함수는 서버에서만 실행됨
{
	if( OverlappingWeapon )
	{
		OverlappingWeapon->ShowPickUpWidget(false); // 이전 무기의 위젯을 숨깁니다.
	}

	OverlappingWeapon = Weapon; // OverlappingWeapon 변수에 새로운 무기를 설정합니다.

	if( IsLocallyControlled() ) // 로컬 플레이어가 컨트롤하는 캐릭터인 경우 즉 내 캐릭터만!
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickUpWidget(true); // 위젯을 보이게 합니다.
		}
	}
}

void ABaseCharacter::PostInitializeComponents() // 컴포넌트 초기화시 실행. 컴뱃 컴포넌트 초기화
{
	Super::PostInitializeComponents();

	// CombatComponent의 Character를 이 캐릭터로 설정
	if (CombatComponent)
	{
		CombatComponent->Character = this;
	}
}

void ABaseCharacter::ServerEquipWeapon_Implementation() // 서버에서 지시하는 클라이언트가 무기 장착하는 함수
{
	if( CombatComponent ) // CombatComponent가 존재하고, 서버 권한이 있는 경우
	{
		CombatComponent->EquipWeapon(OverlappingWeapon); // CombatComponent의 EquipWeapon 함수를 호출하여 무기를 장착합니다.
	}
}

bool ABaseCharacter::IsWeaponEquipped()
{
	return ( CombatComponent && CombatComponent->EquippedWeapon ); // CombatComponent가 존재하고, 장착된 무기가 있는지 확인
}

bool ABaseCharacter::IsAiming()
{
	return ( CombatComponent && CombatComponent->bIsAiming ); // CombatComponent가 존재하고, Aim 상태인지 확인
}

void ABaseCharacter::AimOffset( float DeltaTime )
{
	if( CombatComponent && CombatComponent->EquippedWeapon == nullptr ) return; // AimOffset을 계산하지 않습니다.

	FVector Velocity = GetVelocity();
    Velocity.Z = 0; // Z축 속도는 무시합니다.
    float Speed = Velocity.Size(); // 캐릭터의 속도를 계산합니다.
	bool bIsAir = GetCharacterMovement()->IsFalling(); // 캐릭터가 공중에 있는지 확인합니다.

	if( Speed == 0.f && !bIsAir ) // 속도가 0이고 공중에 있지 않다면
	{
		bRotateRootBone = true; // 루트 본을 회전하도록 설정합니다.

		FRotator CurrentRotation = FRotator( 0.f, GetBaseAimRotation().Yaw, 0.f );
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator( CurrentRotation, StartingAimRotation ); // 현재 회전과 시작 회전의 차이를 계산합니다.
		AO_Yaw = DeltaAimRotation.Yaw; // Yaw 회전값을 저장합니다.
		
		if( TurningInPlace == ETurningInPlace::ETIP_NotTurning ) // 좌우 회전 상태가 아닌 경우
		{
			InterpAO_Yaw = AO_Yaw;
		}
		
		bUseControllerRotationYaw = true; // 컨트롤러 회전 사용
		TurnInPlace(DeltaTime); // TurnInPlace 함수를 호출하여 회전 처리
	}
	if( Speed > 0.0f || bIsAir ) // 
	{
		bRotateRootBone = false; // 루트 본을 회전하지 않도록 설정합니다.

		StartingAimRotation = FRotator( 0.f, GetBaseAimRotation().Yaw, 0.f ); // 조준 회전을 시작합니다.
		AO_Yaw = 0.f; // Yaw 회전값을 0으로. 움직일떄는 좌우로 못 보게
		bUseControllerRotationYaw = true; // 컨트롤러 회전 사용
		TurningInPlace = ETurningInPlace::ETIP_NotTurning; // 좌우 회전 상태를 초기화합니다.
	}

	Calculate_AO_Pitch();
}

void ABaseCharacter::Calculate_AO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch; // AO 피치를 현재 조준 회전의 Pitch로 설정합니다. -90 ~ 90도 사이로 제한됩니다.
	if( AO_Pitch > 90.f && !IsLocallyControlled() ) // 로컬 플레이어가 아닌 경우
	{
		FVector2D InRange(270.f, 360.f); // 270도에서 360도 사이로 제한합니다.
		FVector2D OutRange( -90.f, 0.f ); // -90도에서 0도 사이로 변환합니다.
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch); // 범위 변환
		/*
		언리얼 엔진에서 Pitch 값이 네트워크로 복제될 때 0~360도로 변환되어
		-90<del>0도(아래로 조준) 값이 270</del>360도로 올 수 있습니다.
		이걸 다시 -90~0도로 변환해서
		다른 플레이어가 보는 캐릭터의 조준 방향이 올바르게 보이도록 하는 코드입니다.
		*/
	}
}

void ABaseCharacter::TurnInPlace(float DeltaTime)
{
	if( AO_Yaw > 90.f )
	{
		TurningInPlace = ETurningInPlace::ETIP_Right; // 오른쪽으로 회전 중
	}	
	else if( AO_Yaw < -90.f )
	{
		TurningInPlace = ETurningInPlace::ETIP_Left; // 왼쪽으로 회전 중
	}
	
	if( TurningInPlace != ETurningInPlace::ETIP_NotTurning )
	{
		InterpAO_Yaw = FMath::FInterpTo( InterpAO_Yaw, 0.f, DeltaTime, 4.f ); // AO_Yaw를 보간합니다.
		AO_Yaw = InterpAO_Yaw; // 보간된 값을 AO_Yaw에 저장합니다.
		if( FMath::Abs( AO_Yaw ) < 15.f ) // 보간된 값이 15보다 작으면
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning; // 좌우 회전 상태를 초기화합니다.
			StartingAimRotation = FRotator( 0.f, GetBaseAimRotation().Yaw, 0.f ); // 조준 회전을 시작합니다.
		}
	}
	
}

void ABaseCharacter::SimProxyTurn()
{
	if( CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr ) return; // CombatComponent나 EquippedWeapon이 없으면 함수 종료

	bRotateRootBone = false;

	ProxyRotationLastFrame = ProxyRotation; // 이전 프레임의 회전값을 저장합니다.
	ProxyRotation = GetActorRotation(); // 현재 회전값을 저장합니다.
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator( ProxyRotation, ProxyRotationLastFrame ).Yaw; // 현재 회전값과 이전 프레임의 회전값의 차이를 계산합니다.

	if( FMath::Abs( ProxyYaw ) > TurnThreshold ) // 회전값의 Yaw가 TurnThreshold보다 크면
	{
		if( ProxyYaw > TurnThreshold )
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if( ProxyYaw < TurnThreshold )
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning; // 좌우 회전 상태를 초기화합니다.
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning; // 좌우 회전 상태를 초기화합니다.
}

void ABaseCharacter::PlayFireMontage()
{
	if( CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr || FireWeaponMontage == nullptr ) return; // CombatComponent, EquippedWeapon, FireWeaponMontage가 없으면 함수 종료

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if( AnimInstance && FireWeaponMontage )
	{
		AnimInstance->Montage_Play(FireWeaponMontage); // 무기 발사 애니메이션 몽타주를 재생합니다.
		FName SectionName = "Fire"; // 애니메이션 몽타주의 섹션 이름을 "Fire"로 설정합니다.
		AnimInstance->Montage_JumpToSection(SectionName); // 해당 섹션으로 점프합니다.
	}
}

void ABaseCharacter::PlayHitReactMontage()
{
	if( CombatComponent == nullptr || HitReactMontage == nullptr || CombatComponent->EquippedWeapon == nullptr ) return; // CombatComponent나 HitReactMontage가 없으면 함수 종료

	//UE_LOG(LogTemp, Warning, TEXT("PlayHitReactMontage called")); // 디버깅용 로그 출력

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if( AnimInstance && HitReactMontage && !bDead ) // HitReactMontage가 존재하고, 캐릭터가 죽지 않은 경우
	{
		AnimInstance->Montage_Play(HitReactMontage); // 적에게 맞았을 때 애니메이션 몽타주를 재생합니다.
		FName SectionName = "HitFront"; // 애니메이션 몽타주의 섹션 이름을 "HitReact"로 설정합니다.
		AnimInstance->Montage_JumpToSection(SectionName); // 해당 섹션으로 점프합니다.
	}
}

void ABaseCharacter::PlayDieMontage()
{
	if( CombatComponent == nullptr || HitReactMontage == nullptr || CombatComponent->EquippedWeapon == nullptr ) return; // CombatComponent나 HitReactMontage가 없으면 함수 종료

	//UE_LOG(LogTemp, Warning, TEXT("PlayHitReactMontage called")); // 디버깅용 로그 출력

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if( AnimInstance && DieMontage )
	{
		AnimInstance->Montage_Play(DieMontage); // 적에게 맞았을 때 애니메이션 몽타주를 재생합니다.
	}
}

void ABaseCharacter::HideCameraIfCharacterClose()
{
	if( !IsLocallyControlled() ) return; // 로컬 플레이어가 아닌 경우 함수 종료

	if( ( FollowCamera->GetComponentLocation() - GetActorLocation() ).Size() < CameraThreshold )
	{
		bCloseWall = true; // 카메라가 캐릭터에 너무 가까이 있으면 bCloseWall을 true로 설정합니다.
		GetMesh()->SetVisibility(false); // 카메라가 캐릭터에 너무 가까우면 메쉬를 숨깁니다.
		if( CombatComponent &&  CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh() ) 
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true; // 무기 메쉬도 숨깁니다.
		}
	}
	else
	{
		bCloseWall = false; // 카메라가 캐릭터에 너무 가까이 있지 않으면 bCloseWall을 false로 설정합니다.
		GetMesh()->SetVisibility(true); // 카메라가 캐릭터에 너무 가까우지 않으면 메쉬를 보이게 합니다.
		if( CombatComponent &&  CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh() ) 
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false; 
		}
	} 
}

void ABaseCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement(); // 부모 클래스의 OnRep_ReplicatedMovement를 호출합니다.

	if( GetLocalRole() == ENetRole::ROLE_SimulatedProxy ) // 로컬 플레이어가 아닌 경우
	{
		//SimProxyTurn(); // SimProxyTurn 함수를 호출하여 회전 처리
	}
	TimeSinceLastMovementReplicattion = 0.f; // 마지막 이동 복제 시간 초기화
}

void ABaseCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MAxHealth); // 피해를 받아 Health를 감소시키고, 0과 MAxHealth 사이로 제한합니다.
	// 이떄 OnRep_Health가 호출 클라이언트에서 플레이 힛 몽타주 실행
	// 이떄 캐릭터 체력이 0 이면 변수 변경 없고 복제도 안 일어남. 그레서 0 이하로 내려가면 힛 몽타주가 한쪽에서 안보임!
	PlayHitReactMontage(); // 멀티캐스트로 애니메이션 몽타주를 재생합니다.
	UpdateHUDHealth();

	if(  Health == 0.f ) // Health가 0 이하로 떨어지면
	{
		AMainGameMode* MainGameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();
		if( MainGameMode )
		{
			PlayerController = PlayerController == nullptr ? Cast<ABasePC>(Controller) : PlayerController; // 플레이어 컨트롤러를 ABasePC로 캐스팅합니다.

			ABasePC* AttackerController = Cast<ABasePC>(InstigatorController); // 공격자를 ABasePC로 캐스팅합니다.

			MainGameMode->PlayerEliminated(this, PlayerController, AttackerController ); // 플레이어 제거 함수 호출
		}
	}

}

void ABaseCharacter::UpdateHUDHealth()
{
	PlayerController = PlayerController == nullptr ? Cast<ABasePC>(Controller) : PlayerController; // 플레이어 컨트롤러를 ABasePC로 캐스팅합니다.
	if( PlayerController )
	{
		PlayerController->SetHUDHealth(Health, MAxHealth); // 플레이어 컨트롤러의 HUD 체력을 설정합니다.
	}
}

void ABaseCharacter::MulticastEliminate_Implementation()
{
	bDead = true;
	PlayDieMontage(); // 죽었을 때 애니메이션 몽타주를 재생합니다.

    // 2초 뒤에 StartDissolveMaterials 실행
    FTimerHandle DissolveTimerHandle;
    GetWorldTimerManager().SetTimer(
        DissolveTimerHandle,
        this,
        &ABaseCharacter::StartDissolve,
        2.0f, // 2초 뒤에 실행
        false
    );
}

void ABaseCharacter::Eliminate()
{
	MulticastEliminate(); // 멀티캐스트로 함수 실행

	// 이 아래로는 서버에서만 실행
	GetWorldTimerManager().SetTimer(
		EliminateTimer, // 타이머 핸들을 설정합니다.
		this, // 현재 객체를 타겟으로 설정
		&ABaseCharacter::EliminateTimerFinished, // 타이머가 끝났을 때 호출할 함수
		EliminateDelay); // 타이머 설정
}

void ABaseCharacter::EliminateTimerFinished()
{
	AMainGameMode* MainGameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();
	if( MainGameMode )
	{
		MainGameMode->RequestRespawn( this, Controller ); // 플레이어 제거 함수 호출
	}
}

void ABaseCharacter::UpdateDissolve(float DissolveValue)
{
	UE_LOG(LogTemp, Warning, TEXT("DissolveValue: %f"), DissolveValue); // 디졸브 값 출력
	if( DynamicDissolveMaterialInstanceBody )
	{
		DynamicDissolveMaterialInstanceBody->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue); // 디졸브 값을 업데이트합니다.
	}
	if( DynamicDissolveMaterialInstanceHead )
	{
		DynamicDissolveMaterialInstanceHead->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue); // 디졸브 값을 업데이트합니다.
	}
}

void ABaseCharacter::StartDissolve()
{
	if (DIssolveMaterialInstanceBody)
    {
        DynamicDissolveMaterialInstanceBody = UMaterialInstanceDynamic::Create(DIssolveMaterialInstanceBody, this);
        GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstanceBody);
        DynamicDissolveMaterialInstanceBody->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
        DynamicDissolveMaterialInstanceBody->SetScalarParameterValue(TEXT("Glow"), 200.f);
    }
    if (DIssolveMaterialInstanceHead)
    {
        DynamicDissolveMaterialInstanceHead = UMaterialInstanceDynamic::Create(DIssolveMaterialInstanceHead, this);
        GetMesh()->SetMaterial(1, DynamicDissolveMaterialInstanceHead);
        DynamicDissolveMaterialInstanceHead->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
        DynamicDissolveMaterialInstanceHead->SetScalarParameterValue(TEXT("Glow"), 200.f);	
    }

	DissolveTrack.BindDynamic(this, &ABaseCharacter::UpdateDissolve); // 타임라인의 업데이트 함수에 UpdateDissolve를 바인딩합니다.

	if( DissolveCurve && DissolveTimeline )
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack); // 타임라인에 디졸브 커브를 추가합니다.
		DissolveTimeline->Play();
	}
}



