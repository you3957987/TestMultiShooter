#include "BaseWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h" // 네트워크 관련 헤더 파일
#include "Components/WidgetComponent.h" // 위젯 컴포넌트 헤더 파일
#include "MultiShooter/Character/BaseCharacter.h" // 캐릭터 헤더 파일
#include "OutBullet.h" // 총알 헤더 파일
#include "Kismet/GameplayStatics.h" // 게임 관련 유틸리티 함수들을 사용하기 위한 헤더 파일
#include "MultiShooter/PlayerController/BasePC.h" // 플레이어 컨트롤러 헤더 파일
#include "MultiShooter/Character/BaseAnimInstance.h" // 캐릭터 애니메이션 인스턴스 헤더 파일
#include "Kismet/KismetMathLibrary.h" // 수학 관련 유틸리티 함수들을 사용하기 위한 헤더 파일

ABaseWeapon::ABaseWeapon()
{
	PrimaryActorTick.bCanEverTick = false;// 무기 클래스는 매 프레임마다 업데이트할 필요가 없으므로 false로 설정합니다.

	bReplicates = true; // 이 무기는 네트워크에서 복제됩니다.
	bAlwaysRelevant = true; // 이 무기는 항상 네트워크에서 관련성이 있습니다.
	SetReplicatingMovement(true); // 이 무기의 이동을 네트워크에서 복제합니다.

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh); // 무기 메쉬 컴포넌트를 루트 컴포넌트로 설정합니다.

	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block); // 모든 채널에 대해 충돌 응답을 차단합니다.
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Ignore); // Pawn 채널에 대해서는 충돌 응답을 무시합니다.
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 충돌을 비활성화합니다. 나중에 필요할 때 활성화할 수 있습니다.

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE); // 무기 메쉬의 커스텀 깊이 스텐실 값을 설정합니다. 윤곽선 색상을 설정하기 위해 사용됩니다.
	//#define CUSTOM_DEPTH_PURPLE 250 #define CUSTOM_DEPTH_BLUE 251  CUSTOM_DEPTH_BROWN 252
	WeaponMesh->MarkRenderStateDirty(); // 렌더 상태를 업데이트하여 커스텀 깊이 스텐실 값이 적용되도록 합니다.
	EnableCustomDepth(true); // 윤곽선을 활성화합니다. 이 함수는 BaseWeapon.h에서 정의되어 있습니다.

	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetupAttachment(RootComponent);

	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore); // 모든 채널에 대해 충돌 응답을 무시합니다.
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 쿼리 전용으로 설정합니다.

	PickUpWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickUpWidget"));
	PickUpWidget->SetupAttachment(RootComponent); // 위젯 컴포넌트를 루트 컴포넌트에 첨부합니다.

	MuzzlePoint = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzlePoint")); // 총구 위치를 나타내는 컴포넌트 생성
	MuzzlePoint->SetupAttachment(WeaponMesh); // 

	BulletOutPoint = CreateDefaultSubobject<USceneComponent>(TEXT("BulletOutPoint")); // 탄피 배출 위치를 나타내는 컴포넌트 생성
	BulletOutPoint->SetupAttachment(WeaponMesh); // 무기 메쉬에 첨부합니다.
}

void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();

	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 쿼리 전용으로 설정합니다.
	CollisionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Overlap); // Pawn 채널에 대해서는 충돌 응답을 겹침으로 설정합니다.

	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ABaseWeapon::OnSphereBeginOverlap); // 겹침 시작 이벤트에 대한 바인딩
	CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &ABaseWeapon::OnSphereEndOverlap); // 겹침 종료 이벤트에 대한 바인딩
	
	if (PickUpWidget)
	{
		PickUpWidget->SetVisibility(false); // 위젯을 초기에는 보이지 않도록 설정합니다.
	}

}

void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 무기 상태를 네트워크에서 복제합니다.
	DOREPLIFETIME(ABaseWeapon, WeaponState);
}

void ABaseWeapon::OnRep_Owner() // 소유자가 변경될 때 호출되는 함수
{
	Super::OnRep_Owner(); // 부모 클래스의 OnRep_Owner 함수를 호출합니다.

	if( Owner == nullptr ) // 만약 소유자가 nullptr인 경우, 즉 플레이어가 죽은 경우
	{
		Character = nullptr; // 캐릭터를 nullptr로 설정합니다. (플레이어가 죽었으므로)
		PlayerController = nullptr; // 플레이어 컨트롤러를 nullptr로 설정합니다. (플레이어가 죽었으므로)
	}
	else
	{
		Character = Character == nullptr ? Cast<ABaseCharacter>(GetOwner()) : Character; // 캐릭터가 nullptr인 경우 현재 무기의 소유자를 캐스팅합니다.
		if( Character && Character->GetEquippedWeapon() && Character->GetEquippedWeapon() == this) // 캐릭터가 존재하고, 현재 무기가 캐릭터의 장착된 무기인 경우
		{
			SetHUDAmmo();
		}
	}
}

void ABaseWeapon::SpendAmmo() 
{
	Ammo = FMath::Clamp( Ammo - 1 , 0, MaxAmmo ); // 총알 개수를 감소시킵니다.
	SetHUDAmmo();
	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo); // 서버에서 클라이언트로 총알 개수를 업데이트합니다.
	}
	else
	{
		SequnceAmmo++; // 서버에서 처리되지 않은 요청의 개수를 증가시킵니다.
	}
}

void ABaseWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return; // 서버에서 실행되지 않도록 합니다.
	Ammo = ServerAmmo; // 클라이언트에서 서버의 총알 개수를 업데이트합니다.
	SequnceAmmo--; // 처리되지 않은 요청의 개수를 감소시킵니다.
	Ammo -= SequnceAmmo; // 서버에서 처리되지 않은 요청의 개수를 감소시킵니다.
	SetHUDAmmo(); // HUD에 총알 개수를 설정합니다.
}

void ABaseWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) return; // 서버에서 실행되지 않도록 합니다.
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MaxAmmo); // 총알 개수를 증가시키고 최대값을 초과하지 않도록 합니다.
	SetHUDAmmo(); // HUD에 총알 개수를 설정합니다.
}

void ABaseWeapon::AddAmmo(int32 AmmoToAdd) // 총알을 추가하는 함수
{
	Ammo = FMath::Clamp( Ammo + AmmoToAdd, 0, MaxAmmo ); // 총알 개수를 증가시키고 최대값을 초과하지 않도록 합니다.

	SetHUDAmmo();
	ClientAddAmmo(AmmoToAdd); // 클라이언트에서 총알을 추가합니다.
}

bool ABaseWeapon::IsEmptyAmmo() 
{
	return Ammo <= 0; // 총알 개수가 0 이하이면 true를 반환합니다.
}

bool ABaseWeapon::IsFull()
{
	return Ammo == MaxAmmo;	 // 총알 개수가 최대값과 같으면 true를 반환합니다.
}

void ABaseWeapon::SetHUDAmmo()
{
    Character = Character == nullptr ? Cast<ABaseCharacter>(GetOwner()) : Character; // 캐릭터가 nullptr인 경우 현재 무기의 소유자를 캐스팅합니다.
    if (Character)
    {
        PlayerController = PlayerController == nullptr ? Cast<ABasePC>(Character->GetController()) : PlayerController; // 플레이어 컨트롤러가 nullptr인 경우 캐릭터의 컨트롤러를 캐스팅합니다.
        if (PlayerController)
        {
            PlayerController->SetHUDAmmo(Ammo); // 플레이어 컨트롤러의 HUD에 총알 개수를 설정합니다.
        }
    }
}

void ABaseWeapon::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ABaseCharacter* OverlappingCharacter = Cast<ABaseCharacter>(OtherActor);
    if( OverlappingCharacter )
    {
        //PickUpWidget->SetVisibility(true);
        OverlappingCharacter->SetOverlappingWeapon(this);
    }
}

void ABaseWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,  UPrimitiveComponent* OtherComp,  int32 OtherBodyIndex)
{
    ABaseCharacter* OverlappingCharacter = Cast<ABaseCharacter>(OtherActor);
    if( OverlappingCharacter )
    {
        //PickUpWidget->SetVisibility(false);
        OverlappingCharacter->SetOverlappingWeapon(nullptr);
    }
}

void ABaseWeapon::ShowPickUpWidget(bool bShow)
{
	if( PickUpWidget )
	{
		PickUpWidget->SetVisibility(bShow); // 위젯의 가시성을 설정합니다.
	}
}

void ABaseWeapon::SetWeaponState(EWeaponState NewState) // 무기 상태를 설정하는 함수
{
	EWeaponState LastState = WeaponState;
	WeaponState = NewState;

	OnWeaponStateSet();

	if (LastState == EWeaponState::EWS_Intital && NewState != EWeaponState::EWS_Intital)
	{
		OnEquippedDelegate.Broadcast();
	}
}

void ABaseWeapon::OnRep_WeaponState() // 무기 상태가 변경될 때 호출되는 함수. 클라이언트에서만 실행
{
    OnWeaponStateSet(); // 무기 상태가 변경되었을 때 호출되는 함수를 실행합니다.	
}

void ABaseWeapon::OnWeaponStateSet()
{
	switch (WeaponState) // on_rep_WeaponState 호출
    {
        case EWeaponState::EWS_Equiped:
            OnEquipped(); // 무기가 장착될 때 호출되는 함수를 실행합니다.
            break;
        case EWeaponState::EWS_Dropped:
			OnDropped(); // 무기가 드롭될 때 호출되는 함수를 실행합니다.
            break;
		case EWeaponState::EWS_EquippedSecondary:
			OnEquippedSecondary(); // 보조 무기가 장착될 때 호출되는 함수를 실행합니다.
			break;
        default:
            break;
    }
}

void ABaseWeapon::OnEquipped() // 무기가 장착될 때 호출되는 함수
{
	ShowPickUpWidget(false); // 무기가 장착되면 위젯을 숨깁니다.
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); 
	WeaponMesh->SetSimulatePhysics(false); 
	WeaponMesh->SetEnableGravity(false); 
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnableCustomDepth(false); // 윤곽선을 비활성화합니다.
}

void ABaseWeapon::OnDropped() // 무기가 드롭될 때 호출되는 함수
{
	if( HasAuthority() ) // 서버에서만 실행
	{
		CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // 충돌을 활성화합니다.
	}
	WeaponMesh->SetSimulatePhysics(true); // 물리 시뮬레이션을 활성화합니다.
	WeaponMesh->SetEnableGravity(true); // 중력을 활성화합니다.
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 쿼리 및 물리 충돌을 활성화합니다.
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block); // 모든 채널에 대해 충돌 응답을 차단합니다.
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); 

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE); // 무기 메쉬의 커스텀 깊이 스텐실 값을 설정합니다. 윤곽선 색상을 설정하기 위해 사용됩니다.
	WeaponMesh->SetRenderCustomDepth(true); // << 이 줄 추가!
    WeaponMesh->MarkRenderStateDirty(); // 렌더 상태를 업데이트하여 커스텀 깊이 스텐실 값이 적용되도록 합니다.
}

void ABaseWeapon::OnEquippedSecondary() // 보조 무기가 장착될 때 호출되는 함수
{
	ShowPickUpWidget(false); // 무기가 장착되면 위젯을 숨깁니다.
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); 
	WeaponMesh->SetSimulatePhysics(false); 
	WeaponMesh->SetEnableGravity(false); 
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	EnableCustomDepth(true); // 윤곽선을 활성화합니다.
	// 보조 무기 윤곽선 설정
    WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BROWN); // 무기 메쉬의 커스텀 깊이 스텐실 값을 설정합니다. 윤곽선 색상을 설정하기 위해 사용됩니다.
	WeaponMesh->MarkRenderStateDirty(); // 렌더 상태를 업데이트하여 커스텀 깊이 스텐실 값이 적용되도록 합니다.
}

void ABaseWeapon::Fire(const FVector& HitTarget) // 발사 함수, 타겟 위치를 인자로 받음
{
	//이거는 자식에서 제대로 구현함. 강의에서는 발사 애니메이션으로 머즐 포인트 생성

	if( OutBulletClass ) // 이거 총알 아니라 탄피임
	{
		FTransform FirePosition = BulletOutPoint->GetComponentTransform();

		if(  OutBulletClass )
		{
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<AOutBullet>(OutBulletClass, FirePosition.GetLocation() , FirePosition.GetRotation().Rotator() );
			}
		}
	}
	if( MuzzleFlash )
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, MuzzlePoint->GetComponentTransform(), true); // 총구 플래시 이펙트 생성
	}
	SpendAmmo(); // 총알을 소모합니다.
}

void ABaseWeapon::Dropped() // 플레이어 죽음
{
	SetWeaponState(EWeaponState::EWS_Dropped); // 무기 상태를 떨어진 상태로 설정합니다.

	FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachmentRules); // 무기를 월드 트랜스폼을 유지하면서 분리합니다.
	SetOwner(nullptr); // 무기의 소유자를 nullptr로 설정합니다. (플레이어가 죽었으므로)
	Character = nullptr; // 캐릭터를 nullptr로 설정합니다. (플레이어가 죽었으므로)
	PlayerController = nullptr; // 플레이어 컨트롤러를 nullptr로 설정합니다. (플레이어가 죽었으므로)
}

void ABaseWeapon::EnableCustomDepth(bool bEnable) // 윤곽선을 활성화하거나 비활성화하는 함수
{
	if( WeaponMesh )
	{
		WeaponMesh->SetRenderCustomDepth(bEnable); // 무기 메쉬의 커스텀 깊이 렌더링을 설정합니다.
	}
}

FVector ABaseWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	if (MuzzlePoint == nullptr) return FVector();

	const FVector Start = MuzzlePoint->GetComponentLocation(); // 총구 위치를 가져옵니다.

	const FVector ToTargetNormalized = (HitTarget - Start).GetSafeNormal(); // 타겟 방향 벡터를 정규화합니다.
	const FVector SphereCenter = Start + ToTargetNormalized * DinstanceToSphere; // 구의 중심을 계산합니다.

	const FVector RandomScatter = FMath::VRand() * FMath::RandRange(0.f, SphereRadius); // 랜덤한 산탄 효과를 위해 무작위 벡터를 생성합니다.
	const FVector EndLoc = SphereCenter + RandomScatter; // 구의 중심에 랜덤 산탄 벡터를 더하여 최종 위치를 계산합니다.
	const FVector ToEndLoc = EndLoc - Start; // 시작 위치에서 최종 위치까지의 벡터를 계산합니다.

	/*
	DrawDebugSphere(
		GetWorld(), // 현재 월드
		SphereCenter, // 구의 중심
		SphereRadius, // 구의 반지름
		12, // 구의 세그먼트 수
		FColor::Red, // 구의 색상
		true
	);

	DrawDebugSphere(
		GetWorld(), // 현재 월드
		EndLoc, // 최종 위치
		4.f, // 구의 반지름
		12, // 구의 세그먼트 수
		FColor::Orange, // 구의 색상
		true
	);

	DrawDebugLine(
		GetWorld(), // 현재 월드
		Start, // 시작 위치
		FVector(Start + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size() ), // 최종 위치
		FColor::Green, // 선의 색상
		true
	);
	*/

	return FVector(Start + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()); // 현재는 산탄을 사용하지 않으므로, 기본적으로 (0,0,0)을 반환합니다.
}
