#include "BaseCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h" // UCapsuleComponent 사용을 위한 include
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h" // 네트워크 복제를 위한 헤더 파일
#include "TimerManager.h" // 타이머 매니저 사용을 위한 헤더 파일
#include "Kismet/GameplayStatics.h" // 게임플레이 스태틱스 사용을 위한 헤더 파일
#include "Sound/SoundCue.h" // 사운드 큐 사용을 위한 헤더 파일
#include "Particles/ParticleSystemComponent.h" // 파티클 시스템 컴포넌트 사용을 위한 헤더 파일
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/BoxComponent.h" // 박스 컴포넌트 사용을 위한 헤더 파일

#include "MultiShooter/Weapon/BaseWeapon.h" // ABaseWeapon 클래스 사용을 위한 헤더 파일
#include "MultiShooter/CharacterComponent/CombatComponent.h" // UCombatComponent 클래스 사용을 위한 헤더 파일
#include "MultiShooter/CharacterComponent/BuffComponent.h"
#include "BaseAnimInstance.h" // ABaseAnimInstance 클래스 사용을 위한 헤더 파일
#include "MultiSHooter/MultiShooter.h" // MultiShooter 헤더 파일을 포함합니다. -> 스켈레탈 콜리전 정의 가져오기
#include "MultiShooter/PlayerController/BasePC.h" // ABasePC 클래스 사용을 위한 헤더 파일
#include "MultiShooter/GameMode/MainGameMode.h" // AMainGameMode 클래스 사용을 위한 헤더 파일
#include "MultiShooter/CharacterState/CharacterState.h" // ACharacterState 클래스 사용을 위한 헤더 파일
#include "MultiShooter/Weapon/WeaponType.h" // EWeaponType 열거형 사용을 위한 헤더 파일
#include "MultiShooter/CharacterComponent/LagCompensationComponent.h"
#include "MultiShooter/GameState/BaseGameState.h" // ABaseGameState 클래스 사용을 위한 헤더 파일
#include "MultiShooter/TypeHeader/Team.h" // ETeam 열거형 사용을 위한 헤더 파일
#include "MultiShooter/PlayerStart/TeamPlayerStart.h" // ETeam 열거형 사용을 위한 헤더 파일
#include "MultiShooter/PlayerStart/SoloPlayerStart.h" // ETeam 열거형 사용을 위한 헤더 파일
#include "MultiShooter/GameInstance/BaseGameInstance.h" // UBaseGameInstance 클래스 사용을 위한 헤더 파일

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

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FrontCamera"));
	FrontCamera->SetupAttachment(GetCapsuleComponent()); // 캡슐 컴포넌트에 연결

	bUseControllerRotationYaw = false; // 캐릭터가 컨트롤러 방향으로 회전하게 만들기
	GetCharacterMovement()->bOrientRotationToMovement = true; // 캐릭터가 이동 방향을 향하도록 설정

	OverHeadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverHeadWidget"));
	OverHeadWidget->SetupAttachment(RootComponent);

	TeamLogoWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("TeamLogoWidget"));
	TeamLogoWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent")); // 전투 컴포넌트 생성
	CombatComponent->SetIsReplicated(true); // 전투 컴포넌트도 네트워크에서 복제되도록 설정

	BuffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent")); // 버프 컴포넌트 생성
	BuffComponent->SetIsReplicated(true); // 버프 컴포넌트도 네트워크에서 복제되도록 설정

	LagCompensationComponent = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensationComponent")); // 지연 보상 컴포넌트 생성
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore); // 캡슐 컴포넌트가 카메라 채널에 반응하지 않도록 설정
	GetMesh()->SetCollisionObjectType( ECC_SkeletalMesh ); // 메쉬의 충돌 오브젝트 타입을 스켈레탈 메쉬로 설정
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore); // 메쉬가 카메라 채널에 반응하지 않도록 설정
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); // 메쉬가 트레이스 채널에 반응하지 않도록 설정
	//GetCharacterMovement()->RotationRate = FRotator(0.f, 850.f, 0.f); // Yaw(좌우 회전) 속도 850도/초 이거 안쓰는게 좋은듯

	TurningInPlace = ETurningInPlace::ETIP_NotTurning; // 초기 좌우 회전 상태 설정

	SetNetUpdateFrequency(66.f);      // 네트워크 업데이트 빈도 설정
	SetMinNetUpdateFrequency(33.f);   // 최소 네트워크 업데이트 빈도 설정

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimeline")); // 디졸브 타임라인 컴포넌트 생성

	// 수류탄 메쉬 컴포넌트 생성
	GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMesh"));
    GrenadeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (GrenadeMesh && GetMesh())
    {
        GrenadeMesh->SetupAttachment(GetMesh(), FName("hand_rSocket")); // 생성자에서는 반드시 SetupAttachment 사용
        GrenadeMesh->SetRelativeLocation(FVector(-15.603674f, 8.142267f, -0.512289f ));
        GrenadeMesh->SetRelativeRotation(FRotator(0.000051f, 109.77877f,0.000034f ));
    }

	// 렉걸릴때 서버에서 리와인드 위한 박스 컴포넌트---------------------------------------
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("HeadBox"));
	head->SetupAttachment(GetMesh(), FName("head")); // 헤드 소켓에 박스 컴포넌트 연결
	HitCollisionBoxes.Add(FName("head"), head); // 히트 박스 리스트에 헤드 박스 추가
	
	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("PelvisBox"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis")); // 펠비스 소켓에 박스 컴포넌트 연결
	HitCollisionBoxes.Add(FName("pelvis"), pelvis); // 히트 박스 리스트에 펠비스 박스 추가
	
	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine_02Box"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02")); // 스파인_02 소켓에 박스 컴포넌트 연결
	HitCollisionBoxes.Add(FName("spine_02"), spine_02); // 히트 박스 리스트에 스파인_02 박스 추가
	
	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("Spine_03Box"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03")); // 스파인_03 소켓에 박스 컴포넌트 연결
	HitCollisionBoxes.Add(FName("spine_03"), spine_03); // 히트 박스 리스트에 스파인_03 박스 추가
	
	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("Upperarm_LBox"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l")); // 어퍼암_L 소켓에 박스 컴포넌트 연결
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l); // 히트 박스 리스트에 어퍼암_L 박스 추가
	
	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("Upperarm_RBox"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r")); // 어퍼암_R 소켓에 박스 컴포넌트 연결
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r); // 히트 박스 리스트에 어퍼암_R 박스 추가
	
	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("Lowerarm_LBox"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l")); // 로워암_L 소켓에 박스 컴포넌트 연결
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l); // 히트 박스 리스트에 로워암_L 박스 추가
	
	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("Lowerarm_RBox"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r")); // 로워암_R 소켓에 박스 컴포넌트 연결
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r); // 히트 박스 리스트에 로워암_R 박스 추가
	
	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("Hand_LBox"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l")); // 핸드_L 소켓에 박스 컴포넌트 연결
	HitCollisionBoxes.Add(FName("hand_l"), hand_l); // 히트 박스 리스트에 핸드_L 박스 추가
	
	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("Hand_RBox"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r")); // 핸드_R 소켓에 박스 컴포넌트 연결
	HitCollisionBoxes.Add(FName("hand_r"), hand_r); // 히트 박스 리스트에 핸드_R 박스 추가
	
	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("Thigh_LBox"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l")); // 타이_L 소켓에 박스 컴포넌트 연결
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l); // 히트 박스 리스트에 타이_L 박스 추가
	
	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("Thigh_RBox"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r")); // 타이_R 소켓에 박스 컴포넌트 연결
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r); // 히트 박스 리스트에 타이_R 박스 추가
	
	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("Calf_LBox"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l")); // 칼프_L 소켓에 박스 컴포넌트 연결
	HitCollisionBoxes.Add(FName("calf_l"), calf_l); // 히트 박스 리스트에 칼프_L 박스 추가
	
	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("Calf_RBox"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r")); // 칼프_R 소켓에 박스 컴포넌트 연결
	HitCollisionBoxes.Add(FName("calf_r"), calf_r); // 히트 박스 리스트에 칼프_R 박스 추가
	
	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("Foot_LBox"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l")); // 풋_L 소켓에 박스 컴포넌트 연결
	HitCollisionBoxes.Add(FName("foot_l"), foot_l); // 히트 박스 리스트에 풋_L 박스 추가
	
	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("Foot_RBox"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r")); // 풋_R 소켓에 박스 컴포넌트 연결
	HitCollisionBoxes.Add(FName("foot_r"), foot_r); // 히트 박스 리스트에 풋_R 박스 추가

	for (auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox); // 박스 컴포넌트의 충돌 오브젝트 타입을 ECC_HitBox로 설정
			Box.Value->SetCollisionResponseToAllChannels(ECR_Ignore); // 모든 채널에 대해 충돌 응답을 무시합니다.
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block); // 히트 박스 채널에 대해서는 충돌 응답을 차단합니다.
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 초기에는 충돌을 비활성화합니다.
		}
	}
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	/* 입력 매핑은 restart 함수에서 처리합니다.
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
	*/

	// 이 코드 없애면 기본 무기 사라짐
	FTimerHandle WeaponInitTimerHandle;
    GetWorldTimerManager().SetTimer(
        WeaponInitTimerHandle,
        this,
        &ABaseCharacter::InitWeaponAndHUD, 
        0.1f
    );

	UpdateHUDHealth();
	UpdateHUDShield(); // HUD에 체력과 방어구 업데이트
	UpdateHUDGrenade(); // HUD에 수류탄 개수를 업데이트합니다.

	if( HasAuthority() )
	{
		OnTakeAnyDamage.AddDynamic(this, &ABaseCharacter::ReceiveDamage); // 피해를 받았을 때 호출되는 함수 바인딩/ 어플라이 대미지는 프로젝타일에서!
		// 이 함수는 서버에서만 실행되므로 변수 복제시 클라에서도 실행 되게 해야함!
	}
	if( GrenadeMesh )
	{
		GrenadeMesh->SetVisibility(false); // 수류탄 메쉬를 처음에는 보이지 않게 설정합니다.
	}

	SetCamera(); // 비긴 플레이에서 카메라 설정
	SetCharacterMesh(); // 캐릭터 메쉬 설정
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

	if (CombatComponent && CombatComponent->bHoldingFlag)
	{
		bUseControllerRotationYaw = false; // 깃발을 들고 있는 경우 컨트롤러 회전 사용 안함
		GetCharacterMovement()->bOrientRotationToMovement = true; // 이동 방향을 향하도록 설정
		GetCharacterMovement()->MaxWalkSpeed = 100.f;// 이속 최대를 100으로 설정
		TurningInPlace = ETurningInPlace::ETIP_NotTurning; // 깃발을 들고 있는 경우 회전 상태를 초기화
	}
	else
	{
		if (CombatComponent && CombatComponent->EquippedWeapon)
		{
			bUseControllerRotationYaw = true; 
			GetCharacterMovement()->bOrientRotationToMovement = false; 
		}
		if (bDisableGamePlay)
		{
			bUseControllerRotationYaw = false; // 게임 플레이가 비활성화된 경우 컨트롤러 회전 사용 안함
			TurningInPlace = ETurningInPlace::ETIP_NotTurning; // 게임 플레이가 비활성화된 경우 회전 상태를 초기화	
		}
		else
		{
			AimOffset(DeltaTime); // AimOffset 함수 호출
		}
	}

	PollInit(DeltaTime); // 리스폰 될 시 캐릭터 초기화

	HideCameraIfCharacterClose(DeltaTime); // 캐릭터가 카메라에 너무 가까이 있으면 카메라 숨기기
	// 머리 위의 위젯을 캐릭터한테 맞게 업데이트
	UpdateWidgetForLocalView(TeamLogoWidget);
	UpdateWidgetForLocalView(OverHeadWidget);
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// OverlappingWeapon 변수를 네트워크에서 복제합니다.
	DOREPLIFETIME_CONDITION(ABaseCharacter, OverlappingWeapon, COND_OwnerOnly); // 이 변수는 오직 소유자에게만 복제됩니다.
	DOREPLIFETIME(ABaseCharacter, Health); // CurrentHealth 변수를 네트워크에서 복제합니다.
	DOREPLIFETIME(ABaseCharacter, Shield); // MaxHealth 변수를 네트워크에서 복제합니다.
	DOREPLIFETIME(ABaseCharacter, bDisableGamePlay); // 게임 플레이 비활성화 변수 복제
	DOREPLIFETIME(ABaseCharacter, CharacterSkeletalMeshName); // 죽음 상태 변수 복제
}
// 컴포넌트 초기화시 실행. 컴뱃 컴포넌트 및 버프 컴포넌트, 렉 컴포넌트 초기화
void ABaseCharacter::PostInitializeComponents() 
{
	Super::PostInitializeComponents();

	// CombatComponent의 Character를 이 캐릭터로 설정
	if (CombatComponent)
	{
		CombatComponent->Character = this;
	}
	if( BuffComponent )
	{
		BuffComponent->Character = this; // BuffComponent의 Character를 이 캐릭터로 설정
		BuffComponent->SetInitialSpeed(GetCharacterMovement()->MaxWalkSpeed); // 초기 속도를 설정합니다.
		BuffComponent->SetInitialJumpZVelocity(GetCharacterMovement()->JumpZVelocity); // 초기 점프 속도를 설정합니다.
	}
	if (LagCompensationComponent)
	{
		LagCompensationComponent->Character = this; // LagCompensationComponent의 Character를 이 캐릭터로 설정
		if (Controller)
		{
			LagCompensationComponent->Controller = Cast<ABasePC>(Controller);
		}
	}
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
		EnhancedInput->BindAction(IA_Reload, ETriggerEvent::Started, this, &ABaseCharacter::Reload); // 리로드 액션 바인딩
		EnhancedInput->BindAction(IA_Grenade, ETriggerEvent::Started, this, &ABaseCharacter::Grenade); // 수류탄 액션 바인딩
    }
}

//캐릭터 리스폰 되면 캐릭터 스테이트가 널 포인트가 될 수 있으므로 틱에서 캐릭터 스테이트 바로 만들어주기!
void ABaseCharacter::PollInit(float DeltaTime)
{
	if (CharacterState == nullptr)
	{
		CharacterState = GetPlayerState<ACharacterState>(); // 플레이어 상태를 ACharacterState로 캐스팅합니다.
		if (CharacterState)
		{
			OnPlayerStateInitialized();

			ABasePC* BasePC = Cast<ABasePC>(Controller);
			if (BasePC)
			{
				BasePC->HideHUDKilledBy(); // 누구에게 죽었는지 보여주는 위젯을 숨깁니다.
			}

			ABaseGameState* BaseGameState = Cast<ABaseGameState>(UGameplayStatics::GetGameState(this));
			if (BaseGameState && BaseGameState->TopScoreingPlayers.Contains(CharacterState))
			{
				MulticastGainedTheLead(); // 멀티캐스트로 우위를 점한 플레이어를 알립니다.
			}
		}
	}
}

void ABaseCharacter::OnPlayerStateInitialized()
{
	CharacterState->AddToScore(0.f); // 플레이어 상태에 점수 0 추가해서 업데이트 시키기!
	CharacterState->AddToDefeat(0); // 플레이어 상태에 처치 수 0 추가해서 업데이트 시키기!
	SetTeamColor(CharacterState->GetTeam()); // 팀 색상을 설정합니다.
	SetSpawnPoint(); // 스폰 포인트를 설정합니다.
}

// 팀 게임에서 스폰 포인트 설정
void ABaseCharacter::SetSpawnPoint()
{
	if (HasAuthority() && CharacterState && CharacterState->GetTeam() != ETeam::ET_NoTeam) // 팀게임에서만 스폰 포인트 설정
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ATeamPlayerStart::StaticClass(), PlayerStarts); // 팀 플레이어 스타트 액터를 모두 가져옵니다.
		TArray<ATeamPlayerStart*> TeamPlayerStarts; // 팀 플레이어 스타트 액터 배열
		for (auto Start : PlayerStarts)
		{
			ATeamPlayerStart* TeamStart = Cast<ATeamPlayerStart>(Start); // ATeamPlayerStart로 캐스팅

			if (TeamStart && TeamStart->Team == CharacterState->GetTeam() && TeamStart->bSpawned == false) 
			// 캐릭터의 팀과 일치하는 플레이어 스타트만 추가하고 스폰 쿨타 보기
			{
				TeamPlayerStarts.Add(TeamStart); // 캐릭터가 스폰 가능한 플레이어 스타트만 추가
			}
		}
		if (TeamPlayerStarts.Num() > 0)
		{
			ATeamPlayerStart* SpawnPoint = TeamPlayerStarts[FMath::RandRange(0, TeamPlayerStarts.Num() - 1)]; // 랜덤으로 스폰 포인트 선택
			SetActorLocationAndRotation(SpawnPoint->GetActorLocation(), SpawnPoint->GetActorRotation()); // 스폰 포인트 위치와 회전 설정
			SpawnPoint->StartSpawnCooldown(); // 스폰 쿨타임 시작
		}
	}
	if (HasAuthority() && CharacterState && CharacterState->GetTeam() == ETeam::ET_NoTeam && bFirstSpawn) // 개인 게임에서 첫 스폰 관리
	{
		bFirstSpawn = false; // 첫 스폰이 완료되었음을 표시합니다. 이후로는 메인 게임 모드의 requestspawn을 통해 랜덤 스폰

		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASoloPlayerStart::StaticClass(), PlayerStarts);
		TArray<ASoloPlayerStart*> SoloPlayerStarts; // 솔로 플레이어 스타트 액터 배열
		for (auto Start : PlayerStarts)
		{
			ASoloPlayerStart* SoloStart = Cast<ASoloPlayerStart>(Start); // ASoloPlayerStart로 캐스팅
			if (SoloStart && SoloStart->bSpawned == false) // 스폰 쿨타임이 없는 스타트만 추가
			{
				SoloPlayerStarts.Add(SoloStart); // 스폰 가능한 플레이어 스타트만 추가
			}
		}
		if (SoloPlayerStarts.Num() > 0)
		{
			ASoloPlayerStart* SpawnPoint = SoloPlayerStarts[FMath::RandRange(0, SoloPlayerStarts.Num() - 1)]; // 랜덤으로 스폰 포인트 선택
			SetActorLocationAndRotation(SpawnPoint->GetActorLocation(), SpawnPoint->GetActorRotation()); // 스폰 포인트 위치와 회전 설정
			SpawnPoint->StartSpawnCooldown(); // 스폰 쿨타임 시작
		}
	}
}

// 캐릭터가 카메라에 너무 가까이 있으면 무기랑 보조무기랑 캐릭터 숨기기
void ABaseCharacter::HideCameraIfCharacterClose(float DeltaTime)
{
	if (!IsLocallyControlled()) return; // 로컬 플레이어가 아닌 경우 함수 종료

	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		bCloseWall = true; // 카메라가 캐릭터에 너무 가까이 있으면 bCloseWall을 true로 설정합니다.
		GetMesh()->SetVisibility(false); // 카메라가 캐릭터에 너무 가까우면 메쉬를 숨깁니다.
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true; // 무기 메쉬도 숨깁니다.
		}
		if (CombatComponent && CombatComponent->SecondaryWeapon && CombatComponent->SecondaryWeapon->GetWeaponMesh())
		{
			CombatComponent->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true; // 무기 메쉬도 숨깁니다.
		}
	}
	else
	{
		bCloseWall = false; // 카메라가 캐릭터에 너무 가까이 있지 않으면 bCloseWall을 false로 설정합니다.
		GetMesh()->SetVisibility(true); // 카메라가 캐릭터에 너무 가까우지 않으면 메쉬를 보이게 합니다.
		if (CombatComponent && CombatComponent->EquippedWeapon && CombatComponent->EquippedWeapon->GetWeaponMesh())
		{
			CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
		if (CombatComponent && CombatComponent->SecondaryWeapon && CombatComponent->SecondaryWeapon->GetWeaponMesh())
		{
			CombatComponent->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

// 로컬 플레이어의 뷰에 맞게 위젯을 업데이트합니다.
void ABaseCharacter::UpdateWidgetForLocalView(UWidgetComponent* Widget)
{
	if (!Widget) return;

	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC && PC->PlayerCameraManager)
	{
		FVector CamLoc = PC->PlayerCameraManager->GetCameraLocation();
		FVector WidgetLoc = Widget->GetComponentLocation();

		// 항상 카메라 바라보기
		FRotator LookAtRot = (CamLoc - WidgetLoc).Rotation();
		Widget->SetWorldRotation(LookAtRot);

		Widget->SetVisibility(true);
	}
}

// ------------------------------------------------------------------------------------------------조준 관련ㅜ 

// AimOffset 함수는 캐릭터의 조준 오프셋을 계산합니다. 여기서 TurnInPlace로 델타 타임 전달
void ABaseCharacter::AimOffset(float DeltaTime)
{
	if (CombatComponent && CombatComponent->EquippedWeapon == nullptr) return; // AimOffset을 계산하지 않습니다.

	FVector Velocity = GetVelocity();
	Velocity.Z = 0; // Z축 속도는 무시합니다.
	float Speed = Velocity.Size(); // 캐릭터의 속도를 계산합니다.
	bool bIsAir = GetCharacterMovement()->IsFalling(); // 캐릭터가 공중에 있는지 확인합니다.

	if (Speed == 0.f && !bIsAir) // 속도가 0이고 공중에 있지 않다면
	{
		bRotateRootBone = true; // 루트 본을 회전하도록 설정합니다.

		FRotator CurrentRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentRotation, StartingAimRotation); // 현재 회전과 시작 회전의 차이를 계산합니다.
		AO_Yaw = DeltaAimRotation.Yaw; // Yaw 회전값을 저장합니다.

		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning) // 좌우 회전 상태가 아닌 경우
		{
			InterpAO_Yaw = AO_Yaw;
		}

		bUseControllerRotationYaw = true; // 컨트롤러 회전 사용
		TurnInPlace(DeltaTime); // TurnInPlace 함수를 호출하여 회전 처리
	}
	if (Speed > 0.0f || bIsAir) // 
	{
		bRotateRootBone = false; // 루트 본을 회전하지 않도록 설정합니다.

		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f); // 조준 회전을 시작합니다.
		AO_Yaw = 0.f; // Yaw 회전값을 0으로. 움직일떄는 좌우로 못 보게
		bUseControllerRotationYaw = true; // 컨트롤러 회전 사용
		TurningInPlace = ETurningInPlace::ETIP_NotTurning; // 좌우 회전 상태를 초기화합니다.
	}

	Calculate_AO_Pitch();
}

// 회전 방향에 따라 TurningInPlace 변수 설정
void ABaseCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right; // 오른쪽으로 회전 중
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left; // 왼쪽으로 회전 중
	}

	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f); // AO_Yaw를 보간합니다.
		AO_Yaw = InterpAO_Yaw; // 보간된 값을 AO_Yaw에 저장합니다.
		if (FMath::Abs(AO_Yaw) < 15.f) // 보간된 값이 15보다 작으면
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning; // 좌우 회전 상태를 초기화합니다.
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f); // 조준 회전을 시작합니다.
		}
	}

}

void ABaseCharacter::Calculate_AO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch; // AO 피치를 현재 조준 회전의 Pitch로 설정합니다. -90 ~ 90도 사이로 제한됩니다.
	if (AO_Pitch > 90.f && !IsLocallyControlled()) // 로컬 플레이어가 아닌 경우
	{
		FVector2D InRange(270.f, 360.f); // 270도에서 360도 사이로 제한합니다.
		FVector2D OutRange(-90.f, 0.f); // -90도에서 0도 사이로 변환합니다.
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch); // 범위 변환
		/*
		언리얼 엔진에서 Pitch 값이 네트워크로 복제될 때 0~360도로 변환되어
		-90<del>0도(아래로 조준) 값이 270</del>360도로 올 수 있습니다.
		이걸 다시 -90~0도로 변환해서
		다른 플레이어가 보는 캐릭터의 조준 방향이 올바르게 보이도록 하는 코드입니다.
		*/
	}
}

void ABaseCharacter::SimProxyTurn()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return; // CombatComponent나 EquippedWeapon이 없으면 함수 종료

	bRotateRootBone = false;

	ProxyRotationLastFrame = ProxyRotation; // 이전 프레임의 회전값을 저장합니다.
	ProxyRotation = GetActorRotation(); // 현재 회전값을 저장합니다.
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw; // 현재 회전값과 이전 프레임의 회전값의 차이를 계산합니다.

	if (FMath::Abs(ProxyYaw) > TurnThreshold) // 회전값의 Yaw가 TurnThreshold보다 크면
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < TurnThreshold)
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

// ------------------------------------------------------------------------------------------------조준 관련ㅗ 

// ------------------------------------------------------------------------------------------------입력 바인딩 관련 함수들ㅜ 

void ABaseCharacter::Move(const FInputActionValue& Value)
{
	if (bDisableGamePlay) return; // 게임 플레이가 비활성화된 경우 이동을 무시합니다.

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

void ABaseCharacter::Jump()
{
	if (bDisableGamePlay) return; // 게임 플레이가 비활성화된 경우 점프를 무시합니다.
	if (CombatComponent && CombatComponent->bHoldingFlag == true) return; // 깃발을 들고 있는 경우 무기 장착을 무시합니다.

	Super::Jump(); // 기본 점프 함수 호출
}

void ABaseCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();
    AddControllerYawInput(LookAxisVector.X);
    AddControllerPitchInput(LookAxisVector.Y);
}

void ABaseCharacter::Equip()
{
	if (bDisableGamePlay) return; // 게임 플레이가 비활성화된 경우 이동을 무시합니다.

    if(  CombatComponent  ) // CombatComponent가 존재
	{
		if (CombatComponent->bHoldingFlag == true) return; // 깃발을 들고 있는 경우 무기 장착을 무시합니다.

		if(CombatComponent->CombatState == ECombatState::ECS_Unoccupied ) ServerEquipWeapon();

		if (CombatComponent->ShouldSwapWeapon() && HasAuthority() == false && 
			CombatComponent->CombatState == ECombatState::ECS_Unoccupied
			&& OverlappingWeapon == nullptr) // 권한이 없고, CombatState가 Unoccupied 상태인 경우
		{
			PlaySwapMontage();
			CombatComponent->CombatState = ECombatState::ECS_SwappingWeapons; // 무기 교체 상태로 설정
		}
		/*
		if (IsLocallyControlled())
		{
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f); // 무기 먹었을떄 이상한 곳으로 회전하는거 고치는 코드
		}
		*/
	}
}

void ABaseCharacter::ServerEquipWeapon_Implementation() // 서버에서 지시하는 클라이언트가 무기 장착하는 함수
{
	if (CombatComponent) // CombatComponent가 존재하고, 서버 권한이 있는 경우
	{
		if (OverlappingWeapon)
		{
			CombatComponent->EquipWeapon(OverlappingWeapon); // CombatComponent의 EquipWeapon 함수를 호출하여 무기를 장착합니다.
		}
		else if (CombatComponent->ShouldSwapWeapon())
		{
			CombatComponent->SwapWeapon(); // CombatComponent의 SwapWeapon 함수를 호출하여 무기를 교체합니다.
		}
	}
}

void ABaseCharacter::AimPressed()
{
	if (bDisableGamePlay) return; // 게임 플레이가 비활성화된 경우 이동을 무시합니다.
	if (CombatComponent && CombatComponent->bHoldingFlag == true) return; // 깃발을 들고 있는 경우 무기 장착을 무시합니다.

	if (CombatComponent) // CombatComponent가 존재하는 경우
	{
		CombatComponent->SetAiming(true); // Aim 상태를 true로 설정
	}
}

void ABaseCharacter::AimReleased()
{
	if (bDisableGamePlay) return; // 게임 플레이가 비활성화된 경우 이동을 무시합니다.
	if (CombatComponent && CombatComponent->bHoldingFlag == true) return; // 깃발을 들고 있는 경우 무기 장착을 무시합니다.
	
	if (CombatComponent) // CombatComponent가 존재하는 경우
	{
		CombatComponent->SetAiming(false); // Aim 상태를 false로 설정
	}
}

void ABaseCharacter::FirePressed()
{
	if (bDisableGamePlay) return; // 게임 플레이가 비활성화된 경우 이동을 무시합니다.
	if (CombatComponent && CombatComponent->bHoldingFlag == true) return; // 깃발을 들고 있는 경우 무기 장착을 무시합니다.
	
	if (CombatComponent) // CombatComponent가 존재하는 경우
	{
		CombatComponent->FireButtonPressed(true); // 발사 버튼이 눌렸음을 CombatComponent에 알립니다.
	}
}

void ABaseCharacter::FireReleased()
{
	if (bDisableGamePlay) return; // 게임 플레이가 비활성화된 경우 이동을 무시합니다.
	if (CombatComponent && CombatComponent->bHoldingFlag == true) return; // 깃발을 들고 있는 경우 무기 장착을 무시합니다.
	
	if (CombatComponent) // CombatComponent가 존재하는 경우
	{
		CombatComponent->FireButtonPressed(false); // 발사 버튼이 떼어졌음을 CombatComponent에 알립니다.
	}
}

void ABaseCharacter::Reload()
{
	if (bDisableGamePlay) return; // 게임 플레이가 비활성화된 경우 이동을 무시합니다.
	if (CombatComponent && CombatComponent->bHoldingFlag == true) return; // 깃발을 들고 있는 경우 무기 장착을 무시합니다.
	
	if( CombatComponent )
	{
		CombatComponent->Reload(); // CombatComponent의 Reload 함수를 호출하여 무기를 재장전합니다.
	}
}

void ABaseCharacter::Grenade()
{
	if( CombatComponent ) 
	{
		if (CombatComponent->bHoldingFlag == true) return; // 깃발을 들고 있는 경우 무기 장착을 무시합니다.

		CombatComponent->ThrowGrenade(); // CombatComponent의 ThrowGrenade 함수를 호출하여 수류탄을 던집니다.
	}
}

// ------------------------------------------------------------------------------------------------입력 바인딩 관련 함수들ㅗ 

// ------------------------------------------------------------------------------------------------기타 함수들ㅜ

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

void ABaseCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	
	if( Health < LastHealth ) // 체력이 감소했을떄만
	{
		PlayHitReactMontage(); // 멀티캐스트로 애니메이션 몽타주를 재생합니다.
	}
}

void ABaseCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield(); // HUD에 방어력을 업데이트합니다.
	if( Shield < LastShield ) // 방어력이 감소했을떄만
	{
		PlayHitReactMontage(); // 멀티캐스트로 애니메이션 몽타주를 재생합니다.
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

void ABaseCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement(); // 부모 클래스의 OnRep_ReplicatedMovement를 호출합니다.

	if (GetLocalRole() == ENetRole::ROLE_SimulatedProxy) // 로컬 플레이어가 아닌 경우
	{
		//SimProxyTurn(); // SimProxyTurn 함수를 호출하여 회전 처리
	}
	TimeSinceLastMovementReplicattion = 0.f; // 마지막 이동 복제 시간 초기화
}

void ABaseCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, 
	AController* InstigatorController, AActor* DamageCauser)
{
	MainGameMode = MainGameMode == nullptr ? GetWorld()->GetAuthGameMode<AMainGameMode>() : MainGameMode;

	if (bDead || MainGameMode == nullptr) return; // 이미 죽었거나 게임 모드가 널인 경우 함수 종료

	Damage = MainGameMode->CalculateDamage(InstigatorController, Controller, Damage); // 게임 모드에서 팀을 확인후 같은 팀이면 피해를 0으로 설정합니다.

	float DamageToHealth = Damage; // 피해를 Health에 적용할 값으로 설정합니다.

	if (Shield > 0.f)
	{
		if (Shield >= Damage) // 방어구가 피해보다 크거나 같으면
		{
			Shield = FMath::Clamp(Shield - DamageToHealth, 0.f, MAXShield); // 방어구에서 피해를 제거하고, 0과 MAXShield 사이로 제한합니다.
			DamageToHealth = 0.f; // Health에 적용할 피해를 0으로 설정합니다.
		}
		else // 방어구가 피해보다 작으면
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage); // Health에 적용할 피해를 방어구를 제외한 나머지 피해로 설정합니다.
			Shield = 0.f; // 방어구를 0으로 설정합니다.
		}
	}

	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MAxHealth); // 피해를 받아 Health를 감소시키고, 0과 MAxHealth 사이로 제한합니다.
	// 이떄 OnRep_Health가 호출 클라이언트에서 플레이 힛 몽타주 실행
	// 이떄 캐릭터 체력이 0 이면 변수 변경 없고 복제도 안 일어남. 그레서 0 이하로 내려가면 힛 몽타주가 한쪽에서 안보임!
	PlayHitReactMontage(); // 멀티캐스트로 애니메이션 몽타주를 재생합니다.
	UpdateHUDHealth();
	UpdateHUDShield(); // HUD에 체력과 방어구 업데이트

	if (Health == 0.f) // Health가 0 이하로 떨어지면
	{
		if (MainGameMode)
		{
			PlayerController = PlayerController == nullptr ? Cast<ABasePC>(Controller) : PlayerController; // 플레이어 컨트롤러를 ABasePC로 캐스팅합니다.

			ABasePC* AttackerController = Cast<ABasePC>(InstigatorController); // 공격자를 ABasePC로 캐스팅합니다.

			MainGameMode->PlayerEliminated(this, PlayerController, AttackerController); // 플레이어 제거 함수 호출
		}
	}

}

// ------------------------------------------------------------------------------------------------기타 함수들ㅗ

// ------------------------------------------------------------------------------------------------몽타주 실행 함수들ㅜ

// 무기 추가시 몽타주 추가
void ABaseCharacter::PlayFireMontage() 
{
    if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

    UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
    if (!AnimInstance) return;

    switch (CombatComponent->EquippedWeapon->GetWeaponType())
    {
        case EWeaponType::EWT_AssaultRifle:
            if (FireAssultMontage)
            {
                AnimInstance->Montage_Play(FireAssultMontage);
            }
            break;
        case EWeaponType::EWT_RocketLauncher:
            if (FireRocketMontage)
            {
                AnimInstance->Montage_Play(FireRocketMontage);
            }
            break;
        case EWeaponType::EWT_Pistol:
            if (FirePistolMontage)
            {
                AnimInstance->Montage_Play(FirePistolMontage);
            }
            break;
		case EWeaponType::EWT_SMG:
			if (FireSMGMontage)
			{
				AnimInstance->Montage_Play(FireSMGMontage);
			}
			break;
		case EWeaponType::EWT_Shotgun:
			if (FireShotgunMontage)
			{
				AnimInstance->Montage_Play(FireShotgunMontage);
			}
			break;
		case EWeaponType::EWT_Sniper:
			if (FireSniperMontage)
			{
				AnimInstance->Montage_Play(FireSniperMontage);
			}
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			if (FireGrenadeLauncherMontage)
			{
				AnimInstance->Montage_Play(FireGrenadeLauncherMontage);
			}
			break;
		case EWeaponType::EWT_MiniGun:
			if (FireMinigunMontage)
			{
				AnimInstance->Montage_Play(FireMinigunMontage);
			}
			break;
        default:
            break;
    }
}

// 무기 추가시 장전 몽타주 추가
void ABaseCharacter::PlayReloadMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (!AnimInstance) return;

	switch (CombatComponent->EquippedWeapon->GetWeaponType())
	{
	case EWeaponType::EWT_AssaultRifle:
		if (ReloadAssultMontage)
		{
			AnimInstance->Montage_Play(ReloadAssultMontage);
		}
		break;
	case EWeaponType::EWT_RocketLauncher:
		if (ReloadRocketMontage)
		{
			AnimInstance->Montage_Play(ReloadRocketMontage);
		}
		break;
	case EWeaponType::EWT_Pistol:
		if (ReloadPistolMontage)
		{
			AnimInstance->Montage_Play(ReloadPistolMontage);
		}
		break;
	case EWeaponType::EWT_SMG:
		if (ReloadSMGMontage)
		{
			AnimInstance->Montage_Play(ReloadSMGMontage);
		}
		break;
	case EWeaponType::EWT_Shotgun:
		if (ReloadShotgunMontage)
		{
			AnimInstance->Montage_Play(ReloadShotgunMontage);
		}
		break;
	case EWeaponType::EWT_Sniper:
		if (ReloadSniperMontage)
		{
			AnimInstance->Montage_Play(ReloadSniperMontage);
		}
		break;
	case EWeaponType::EWT_GrenadeLauncher:
		if (ReloadGrenadeLauncherMontage)
		{
			AnimInstance->Montage_Play(ReloadGrenadeLauncherMontage);
		}
		break;
	case EWeaponType::EWT_MiniGun:
		if (ReloadMinigunMontage)
		{
			AnimInstance->Montage_Play(ReloadMinigunMontage);
		}
		break;
	default:
		break;
	}
}

void ABaseCharacter::PlayHitReactMontage()
{
	if( CombatComponent == nullptr || HitReactMontage == nullptr  ) return; // CombatComponent나 HitReactMontage가 없으면 함수 종료

	//UE_LOG(LogTemp, Warning, TEXT("PlayHitReactMontage called")); // 디버깅용 로그 출력

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if( AnimInstance && HitReactMontage && !bDead ) // HitReactMontage가 존재하고, 캐릭터가 죽지 않은 경우
	{
		AnimInstance->Montage_Play(HitReactMontage); // 적에게 맞았을 때 애니메이션 몽타주를 재생합니다.
		FName SectionName = "HitFront"; // 애니메이션 몽타주의 섹션 이름을 "HitReact"로 설정합니다.
		AnimInstance->Montage_JumpToSection(SectionName); // 해당 섹션으로 점프합니다.
	}

	CombatComponent->FinshedReload();
}

void ABaseCharacter::PlayDieMontage()
{
	//UE_LOG(LogTemp, Warning, TEXT("PlayHitReactMontage called")); // 디버깅용 로그 출력

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if( AnimInstance && DieMontage )
	{
		AnimInstance->Montage_Play(DieMontage); // 적에게 맞았을 때 애니메이션 몽타주를 재생합니다.
	}
}

void ABaseCharacter::PlayThrowGrenadeMontage()
{
	if( CombatComponent == nullptr || ThrowGrenadeMontage == nullptr ) return; // CombatComponent나 HitReactMontage가 없으면 함수 종료

	//UE_LOG(LogTemp, Warning, TEXT("PlayHitReactMontage called")); // 디버깅용 로그 출력

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if( AnimInstance && ThrowGrenadeMontage )
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage); // 적에게 맞았을 때 애니메이션 몽타주를 재생합니다.
	}
}

void ABaseCharacter::PlaySwapMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage); // 적에게 맞았을 때 애니메이션 몽타주를 재생합니다.
	}
}

// ------------------------------------------------------------------------------------------------몽타주 실행 함수들ㅗ

// ------------------------------------------------------------------------------------------------허드 업데이트 함수들ㅜ

void ABaseCharacter::UpdateHUDHealth()
{
	PlayerController = PlayerController == nullptr ? Cast<ABasePC>(Controller) : PlayerController; // 플레이어 컨트롤러를 ABasePC로 캐스팅합니다.
	if( PlayerController )
	{
		PlayerController->SetHUDHealth(Health, MAxHealth); // 플레이어 컨트롤러의 HUD 체력을 설정합니다.
	}
}

void ABaseCharacter::UpdateHUDShield() // HUD에 방어력을 업데이트하는 함수  버프 컴포넌트에서 사용할 예정이므로 퍼블릭
{
	PlayerController = PlayerController == nullptr ? Cast<ABasePC>(Controller) : PlayerController; // 플레이어 컨트롤러를 ABasePC로 캐스팅합니다.
	if( PlayerController )
	{
		PlayerController->SetHUDShield(Shield, MAXShield); // 플레이어 컨트롤러의 HUD 방어력을 설정합니다.
	}
}

void ABaseCharacter::UpdateHUDGrenade()
{
	PlayerController = PlayerController == nullptr ? Cast<ABasePC>(Controller) : PlayerController; // 플레이어 컨트롤러를 ABasePC로 캐스팅합니다.
	if( PlayerController )
	{
		PlayerController->SetHUDGrenade( CombatComponent->GetGrenade() ); // 플레이어 컨트롤러의 HUD 체력을 설정합니다.
	}
}

void ABaseCharacter::UpdateHUDAmmo()
{
	PlayerController = PlayerController == nullptr ? Cast<ABasePC>(Controller) : PlayerController; // 플레이어 컨트롤러를 ABasePC로 캐스팅합니다.
	if( PlayerController && CombatComponent && CombatComponent->EquippedWeapon )
	{
		PlayerController->SetHUDCarriedAmmo( CombatComponent->CarriedAmmo ); // 플레이어 컨트롤러의 HUD 탄약을 설정합니다.
		PlayerController->SetHUDAmmo( CombatComponent->EquippedWeapon->GetAmmo() ); // 플레이어 컨트롤러의 HUD 무기 탄약을 설정합니다.
	}
}

// ------------------------------------------------------------------------------------------------허드 업데이트 함수들ㅗ

// ------------------------------------------------------------------------------------------------플레이거 죽었을때 관련 함수들ㅜ 

// bLeftGame 값을 bPlayerLeftGame 따라 결정 후 타이머 피니쉬에서 따로 실행
void ABaseCharacter::MulticastEliminate_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;

	if( PlayerController )
	{
		PlayerController->SetHUDAmmo(0); // 플레이어 컨트롤러의 HUD 탄약을 0으로 설정합니다.
	}

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

	FTimerHandle EliminateBotTimer; // 제거 봇 타이머 핸들
	GetWorldTimerManager().SetTimer(
		EliminateBotTimer, // 타이머 핸들을 설정합니다.
		this, // 현재 객체를 타겟으로 설정
		&ABaseCharacter::EliminateBotTimerFinished, // 타이머가 끝났을 때 호출할 함수
		2.0f, // 타이머 설정
		false // 반복하지 않음
	); // 타이머 설정

	GetCharacterMovement()->DisableMovement(); // 캐릭터의 움직임을 비활성화합니다.
	GetCharacterMovement()->StopMovementImmediately(); // 즉시 움직임을 중지합니다. == 마우스 회전까지 정지
	bDisableGamePlay = true; // 게임 플레이를 비활성화합니다.
	if( CombatComponent ) CombatComponent->FireButtonPressed(false); // CombatComponent의 발사 버튼을 비활성화합니다.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 캡슐 컴포넌트의 충돌을 비활성화합니다.
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 메쉬의 충돌을 비활성화합니다.
	GrenadeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 수류탄 메쉬의 충돌을 비활성화합니다.

	if( IsLocallyControlled() && CombatComponent && CombatComponent->bIsAiming && CombatComponent->EquippedWeapon 
		&& CombatComponent->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Sniper ) 
	{
		ShowScopeSniperWidget(false); // 스나이퍼 조준 위젯을 숨깁니다.
	}
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent(); // 크라운 컴포넌트를 파괴합니다.
	}
	GetWorldTimerManager().SetTimer(
		EliminateTimer, // 타이머 핸들을 설정합니다.
		this, // 현재 객체를 타겟으로 설정
		&ABaseCharacter::EliminateTimerFinished, // 타이머가 끝났을 때 호출할 함수
		EliminateDelay
	); // 타이머 설정
}

// 플레이어 제거인데, bPlayerLeftGame 값이 트루면 플레이어가 게임을 떠난 경우고 fasle면 그냥 죽은거
void ABaseCharacter::Eliminate(bool bPlayerLeftGame)
{
	//if( CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr ) return; // CombatComponent나 EquippedWeapon이 없으면 함수 종료
	// CombatComponent->EquippedWeapon->Dropped(); // 무기를 떨어뜨립니다.
	// CombatComponent나 EquippedWeapon이 있을 때만 무기 드랍
	if (CombatComponent)
	{
		if (CombatComponent->EquippedWeapon)
		{
			if (CombatComponent->EquippedWeapon->bDestroyWeapon)
			{
				CombatComponent->EquippedWeapon->Destroy(); // 무기를 파괴합니다.
			}
			else
			{
				CombatComponent->EquippedWeapon->Dropped();
			}
		}
		if (CombatComponent->SecondaryWeapon)
		{
			if (CombatComponent->SecondaryWeapon->bDestroyWeapon)
			{
				CombatComponent->SecondaryWeapon->Destroy(); // 무기를 파괴합니다.
			}
			else
			{
				CombatComponent->SecondaryWeapon->Dropped();
			}
		}
		if (CombatComponent->TheFlag)
		{
			CombatComponent->TheFlag->Dropped(); // 깃발을 떨어뜨립니다.
			CombatComponent->TheFlag = nullptr; // 깃발을 널로 설정합니다.
		}
	}
	MulticastEliminate(bPlayerLeftGame); // 멀티캐스트로 함수 실행
}

//bLeftGame 따라 리스폰 아니면 OnLeftGame.Broadcast() 실행
void ABaseCharacter::EliminateTimerFinished()
{
	MainGameMode = MainGameMode == nullptr ? GetWorld()->GetAuthGameMode<AMainGameMode>() : MainGameMode;
	if( MainGameMode && bLeftGame == false ) // 게임 모드가 존재하고 플레이어가 게임을 떠나지 않은 경우
	{
		UE_LOG(LogTemp, Error, TEXT("Respawn"));
		MainGameMode->RequestRespawn( this, Controller ); // 플레이어 제거 함수 호출
	}
	if (bLeftGame == true && IsLocallyControlled()) // 로컬 플레이어가 게임을 떠난 경우
	{
		OnLeftGame.Broadcast(); // 플레이어가 게임을 떠난 경우 OnLeftGame 이벤트를 브로드캐스트합니다.
	}
}

// 죽고 나서 일정 시간 후 에 제거 봇 효과와 사운드를 재생하는 함수
void ABaseCharacter::EliminateBotTimerFinished()
{
	if( EliminateBotEffect )
	{
		FVector EliminateBotLocation( GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f ); // 제거 봇 효과 위치 설정
		EliminateBotEffectComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(), // 현재 월드
			EliminateBotEffect, // 이펙트
			EliminateBotLocation, // 위치
			GetActorRotation(), // 회전값
			true // 월드 공간에서 스폰
		);
	}

	if( EliminateSound )
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this, // 현재 액터
			EliminateSound, // 사운드 큐
			GetActorLocation() // 위치
		);
	}
}

void ABaseCharacter::Destroyed()
{
	Super::Destroyed(); // 부모 클래스의 Destroyed 함수를 호출합니다.

	if (EliminateBotEffectComponent)
	{
		EliminateBotEffectComponent->DestroyComponent();
	}

	MainGameMode = MainGameMode == nullptr ? GetWorld()->GetAuthGameMode<AMainGameMode>() : MainGameMode; // 현재 게임 모드를 AMainGameMode로 캐스팅합니다.

	bool bMatchNoInProgress = MainGameMode && MainGameMode->GetMatchState() != MatchState::InProgress; // 매치가 진행 중인지 확인합니다.

	if( CombatComponent && CombatComponent->EquippedWeapon && bMatchNoInProgress )
	{
		CombatComponent->EquippedWeapon->Destroy(); 
	}
}

void ABaseCharacter::UpdateDissolve(float DissolveValue)
{
	//UE_LOG(LogTemp, Warning, TEXT("DissolveValue: %f"), DissolveValue); // 디졸브 값 출력
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

void ABaseCharacter::Restart() // Restart는 리스폰/매치 시작 시 호출됨 (클라/서버 모두)
{
    Super::Restart();
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
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

// ------------------------------------------------------------------------------------------------플레이거 죽었을때 관련 함수들ㅗ 

// ------------------------------------------------------------------------------------------------- 캐릭터 기본 무기 주는 함수ㅜ

void ABaseCharacter::SpawnDefaultWeapon() // 기본 무기를 스폰하는 함수
{
	MainGameMode = MainGameMode == nullptr ? GetWorld()->GetAuthGameMode<AMainGameMode>() : MainGameMode; // 현재 게임 모드를 ABaseGameMode로 캐스팅합니다.
	UWorld* World = GetWorld(); // 현재 월드를 가져옵니다.
	if(MainGameMode && World && !bDead && DefaultWeaponClass )
	{
		ABaseWeapon* StartingWeapon = World->SpawnActor<ABaseWeapon>(DefaultWeaponClass); // 기본 무기를 스폰합니다.
		StartingWeapon->bDestroyWeapon = true; // 무기를 파괴할 수 있도록 설정합니다.
		if( CombatComponent )
		{
			CombatComponent->EquipWeapon(StartingWeapon);
		}
		if (IsLocallyControlled())
        {
            StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f); // 무기 먹었을떄 이상한 곳으로 회전하는거 고치는 코드
        }
	}
}

void ABaseCharacter::InitWeaponAndHUD()
{
    SpawnDefaultWeapon();
    UpdateHUDAmmo();
}

// -------------------------------------------------------------------------------------------------캐릭터 기본 무기 주는 함수ㅗ

// --------------------------------------------------------------------------------------------------게터 함수들 모음ㅜ

bool ABaseCharacter::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent->EquippedWeapon); // CombatComponent가 존재하고, 장착된 무기가 있는지 확인
}

bool ABaseCharacter::IsAiming()
{
	if (CombatComponent == nullptr) return false; // CombatComponent가 없으면 false 반환
	return (CombatComponent && CombatComponent->bIsAiming); // CombatComponent가 존재하고, Aim 상태인지 확인
}

bool ABaseCharacter::IsLocallyReloading()
{
	if(CombatComponent == nullptr) return false; // CombatComponent가 없으면 false 반환
	return CombatComponent->bLocallyReloading; // 로컬 플레이어가 리로드 중인지 확인합니다.
}

ECombatState ABaseCharacter::GetCombatState() const
{
	if (CombatComponent == nullptr) return ECombatState::ECS_MAX;

	return CombatComponent->CombatState; // CombatComponent의 CombatState를 반환합니다.
}

ABaseWeapon* ABaseCharacter::GetEquippedWeapon() const
{
	if (CombatComponent == nullptr) return nullptr; // CombatComponent가 없으면 nullptr 반환

	return CombatComponent->EquippedWeapon; // CombatComponent의 EquippedWeapon을 반환합니다.
}

ETeam ABaseCharacter::GetTeam()
{
	CharacterState = CharacterState == nullptr ? GetPlayerState<ACharacterState>() : CharacterState; // 플레이어 상태를 ACharacterState로 캐스팅합니다.

	if (CharacterState == nullptr) return ETeam::ET_NoTeam; // CharacterState가 없으면 NoTeam 반환
	return CharacterState->GetTeam(); // CharacterState의 GetTeam 함수를 호출하여 팀을 반환합니다.
}

bool ABaseCharacter::IsHoldingTheFlag() const
{
	if (CombatComponent == nullptr) return false; // CombatComponent가 없으면 false 반환

	return CombatComponent->bHoldingFlag; // CombatComponent의 bHoldingTheFlag 값을 반환합니다.
}


// --------------------------------------------------------------------------------------------------게터 함수들 모음ㅗ

// ------------------------------------------------------------------------------------------------게임 모드 관련 및 유틸ㅜ 

// 리턴 투 메인메뉴 에서 버튼 클릭시 호출
void ABaseCharacter::ServerLeaveGame_Implementation()
{
	MainGameMode = MainGameMode == nullptr ? GetWorld()->GetAuthGameMode<AMainGameMode>() : MainGameMode;
	CharacterState = CharacterState == nullptr ? GetPlayerState<ACharacterState>() : CharacterState; // 플레이어 상태를 ACharacterState로 캐스팅합니다.

	if (MainGameMode && CharacterState)
	{
		MainGameMode->PlayerLeftGame(CharacterState); // 플레이어가 게임을 떠났음을 알립니다.
	}
}

// 선두 차지하거나 아니면 왕관 나이아가라 생성 및 삭제 함수
void ABaseCharacter::MulticastGainedTheLead_Implementation()
{
	if (CrownSystem == nullptr) return; // CrownSystem이 없으면 함수 종료
	
	if (IsValid(CrownComponent) == false) // 크라운 컴포넌트가 없으면 새로 생성
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem, // 크라운 시스템
			GetMesh(), // 메쉬에 부착
			FName(), // 소켓 이름
			GetActorLocation() + FVector(0.f, 0.f, 110.f), // 위치 오프셋
			GetActorRotation(), // 회전 오프셋
			EAttachLocation::KeepWorldPosition, // 위치 고정
			false // 월드 공간에서 스폰
		);
	}
	if (CrownComponent) // 크라운 컴포넌트가 존재하는 경우
	{
		CrownComponent->Activate(); // 크라운 컴포넌트를 활성화합니다.
	}
}

void ABaseCharacter::MulticastLostTheLead_Implementation()
{
	if (CrownComponent) // 크라운 컴포넌트가 존재하는 경우
	{
		CrownComponent->DestroyComponent(); // 크라운 컴포넌트를 제거합니다.
		CrownComponent = nullptr; // 크라운 컴포넌트를 nullptr로 설정합니다.
	}
}

void ABaseCharacter::SetTeamColor(ETeam Team)
{
	switch (Team)
	{
		case ETeam::ET_NoTeam:
			// 팀이 없을 경우 위젯을 숨김
			if (TeamLogoWidget)
			{
				TeamLogoWidget->SetWidget(nullptr);
				TeamLogoWidget->SetVisibility(false);
			}
			break;

		case ETeam::ET_RedTeam:
			if (TeamLogoWidget && RedTeamWidgetClass)
			{
				TeamLogoWidget->SetWidgetClass(RedTeamWidgetClass);
				TeamLogoWidget->SetVisibility(true);
			}
			break;

		case ETeam::ET_BlueTeam:
			if (TeamLogoWidget && BlueTeamWidgetClass)
			{
				TeamLogoWidget->SetWidgetClass(BlueTeamWidgetClass);
				TeamLogoWidget->SetVisibility(true);
			}
			break;
	}
}

// 깃발을 들고 있는지 설정하는 함수
void ABaseCharacter::SetHoldingTheFlag(bool bHolding)
{
	if (CombatComponent == nullptr) return;
	CombatComponent->bHoldingFlag = bHolding; // CombatComponent의 bHoldingFlag 값을 설정합니다.
}

// ------------------------------------------------------------------------------------------------게임 모드 관련 및 유틸ㅗ

// ------------------------------------------------------------------------------------------------캐릭터 메쉬 관련 함수들ㅜ

void ABaseCharacter::OnRep_CharacterSkeletalMeshName()
{
	// OnRep 함수에서는 메시를 설정하는 로직만 남깁니다.
	// 서버로부터 복제된 CharacterSkeletalMeshName을 기반으로 메시를 설정합니다.
	int32 Index = -1;

	if (CharacterSkeletalMeshName == "Alien_SK") Index = 0;
	else if (CharacterSkeletalMeshName == "BasicFemale_SK") Index = 1;
	else if (CharacterSkeletalMeshName == "BasicMale_SK") Index = 2;
	else if (CharacterSkeletalMeshName == "Bear_SK") Index = 3;
	else if (CharacterSkeletalMeshName == "Chemicalman_SK") Index = 4;
	else if (CharacterSkeletalMeshName == "Chicken_SK") Index = 5;
	else if (CharacterSkeletalMeshName == "Cowboy_SK") Index = 6;
	else if (CharacterSkeletalMeshName == "Hero_SK") Index = 7;
	else if (CharacterSkeletalMeshName == "Hoodie_SK") Index = 8;
	else if (CharacterSkeletalMeshName == "Jester_SK") Index = 9;
	else if (CharacterSkeletalMeshName == "Knight_SK") Index = 10;
	else if (CharacterSkeletalMeshName == "Ninja_SK") Index = 11;
	else if (CharacterSkeletalMeshName == "Robot_SK") Index = 12;
	else if (CharacterSkeletalMeshName == "Salaryman_SK") Index = 13;
	else if (CharacterSkeletalMeshName == "Samurai_SK") Index = 14;
	else if (CharacterSkeletalMeshName == "Sniper_SK") Index = 15;
	else if (CharacterSkeletalMeshName == "Soldier_SK") Index = 16;
	else if (CharacterSkeletalMeshName == "Spaceman_SK") Index = 17;
	else if (CharacterSkeletalMeshName == "Terrorist_SK") Index = 18;
	else if (CharacterSkeletalMeshName == "Veteran_SK") Index = 19;

	if (SelectedSkeletalMesh.IsValidIndex(Index) && SelectedSkeletalMesh[Index] && GetMesh() && Index != -1)
	{
		GetMesh()->SetSkeletalMesh(SelectedSkeletalMesh[Index]);
	}
}

void ABaseCharacter::ServerSetCharacterMeshName_Implementation(const FString& NewMeshName)
{
	CharacterSkeletalMeshName = NewMeshName;
	// 서버에서 OnRep 함수를 직접 호출하여 서버 자신의 캐릭터 메시도 설정합니다.
	OnRep_CharacterSkeletalMeshName();
}

void ABaseCharacter::SetCharacterMeshOnMenu(USkeletalMesh* NewMesh)
{
	if (NewMesh && GetMesh())
	{
		// 메뉴 맵에서 캐릭터의 외형을 즉시 변경합니다.
		GetMesh()->SetSkeletalMesh(NewMesh);

		UBaseGameInstance* GameInstance = Cast<UBaseGameInstance>(GetGameInstance());
		if (GameInstance == nullptr) return;

		// 선택한 메시의 이름을 GameInstance에 저장합니다.
		// 이 정보는 이후 게임 맵에 진입할 때 사용됩니다.
		FString MeshName = NewMesh->GetName();
		GameInstance->SelectedSkeletalMesh = MeshName;
	}
}

void ABaseCharacter::SetCharacterMesh()
{
	// 이 함수는 로컬 플레이어 컨트롤러를 가진 캐릭터에 의해서만 호출되어야 합니다.
	if (IsLocallyControlled())
	{
		UBaseGameInstance* GameInstance = Cast<UBaseGameInstance>(GetGameInstance());
		if (GameInstance && !GameInstance->SelectedSkeletalMesh.IsEmpty())
		{
			// 로컬 클라이언트의 GameInstance에 저장된 메시 이름으로 서버에 RPC를 보냅니다.
			ServerSetCharacterMeshName(GameInstance->SelectedSkeletalMesh);
		}
	}
}

void ABaseCharacter::SetCamera()
{
	FString MapName = GetWorld()->GetMapName();
	if (MapName.Contains("GameStartMap"))
	{
		FrontCamera->SetActive(true);
		FollowCamera->SetActive(false);
	}
	else
	{
		FrontCamera->SetActive(false);
		FollowCamera->SetActive(true);
	}
}

// ------------------------------------------------------------------------------------------------캐릭터 메쉬 관련 함수들ㅗ