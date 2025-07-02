#include "BaseCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Net/UnrealNetwork.h" // 네트워크 복제를 위한 헤더 파일
#include "MultiShooter/Weapon/BaseWeapon.h" // ABaseWeapon 클래스 사용을 위한 헤더 파일
#include "MultiShooter/CharacterComponent/CombatComponent.h" // UCombatComponent 클래스 사용을 위한 헤더 파일
#include "Kismet/KismetMathLibrary.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;


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
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime); // AimOffset 함수 호출

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

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// OverlappingWeapon 변수를 네트워크에서 복제합니다.
	DOREPLIFETIME_CONDITION(ABaseCharacter, OverlappingWeapon, COND_OwnerOnly); // 이 변수는 오직 소유자에게만 복제됩니다.
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
		FRotator CurrentRotation = FRotator( 0.f, GetBaseAimRotation().Yaw, 0.f );
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator( CurrentRotation, StartingAimRotation ); // 현재 회전과 시작 회전의 차이를 계산합니다.
		AO_Yaw = DeltaAimRotation.Yaw; // Yaw 회전값을 저장합니다.
		bUseControllerRotationYaw = false; // 컨트롤러 회전 사용
	}
	if( Speed > 0.0f || bIsAir ) // 
	{
		StartingAimRotation = FRotator( 0.f, GetBaseAimRotation().Yaw, 0.f ); // 조준 회전을 시작합니다.
		AO_Yaw = 0.f; // Yaw 회전값을 초기화합니다.
		bUseControllerRotationYaw = true; // 컨트롤러 회전 사용
	}

	AO_Pitch = GetBaseAimRotation().Pitch; // AO 피치를 현재 조준 회전의 Pitch로 설정합니다.

}