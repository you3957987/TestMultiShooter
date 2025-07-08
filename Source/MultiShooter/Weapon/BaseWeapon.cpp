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

ABaseWeapon::ABaseWeapon()
{
	PrimaryActorTick.bCanEverTick = false;// 무기 클래스는 매 프레임마다 업데이트할 필요가 없으므로 false로 설정합니다.

	bReplicates = true; // 이 무기는 네트워크에서 복제됩니다.
	bAlwaysRelevant = true; // 이 무기는 항상 네트워크에서 관련성이 있습니다.

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh); // 무기 메쉬 컴포넌트를 루트 컴포넌트로 설정합니다.

	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block); // 모든 채널에 대해 충돌 응답을 차단합니다.
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Ignore); // Pawn 채널에 대해서는 충돌 응답을 무시합니다.
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 충돌을 비활성화합니다. 나중에 필요할 때 활성화할 수 있습니다.

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

	PickUpWidget->SetVisibility(false); // 위젯을 초기에는 보이지 않도록 설정합니다.

	if( HasAuthority() ) // 서버에서만 실행되는 초기화 작업. 모든 무기는 서버에서만 관리
	{
		CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 쿼리 전용으로 설정합니다.
		CollisionSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Overlap); // Pawn 채널에 대해서는 충돌 응답을 겹침으로 설정합니다.

		CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &ABaseWeapon::OnSphereBeginOverlap); // 겹침 시작 이벤트에 대한 바인딩
		CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &ABaseWeapon::OnSphereEndOverlap); // 겹침 종료 이벤트에 대한 바인딩
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
	// 총알 개수를 네트워크에서 복제합니다.
	DOREPLIFETIME(ABaseWeapon, Ammo);
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
		SetHUDAmmo();
	}

}

void ABaseWeapon::OnRep_WeaponState() // 무기 상태가 변경될 때 호출되는 함수. 클라이언트에서만 실행
{
    switch (WeaponState)
    {
        case EWeaponState::EWS_Equiped:
            ShowPickUpWidget(false); // 무기가 장착되면 위젯을 숨깁니다.
            CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 충돌을 비활성화합니다.
			CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); 
			WeaponMesh->SetSimulatePhysics(false); 
			WeaponMesh->SetEnableGravity(false); 
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); 
            break;
        case EWeaponState::EWS_Dropped:
			WeaponMesh->SetSimulatePhysics(true); // 물리 시뮬레이션을 활성화합니다.
			WeaponMesh->SetEnableGravity(true); // 중력을 활성화합니다.
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 쿼리 및 물리 충돌을 활성화합니다.
            break;
        default:
            break;
    }	
}

void ABaseWeapon::OnRep_Ammo() 
{
    SetHUDAmmo();
}

void ABaseWeapon::SpendAmmo() 
{
	Ammo = FMath::Clamp( Ammo - 1 , 0, MaxAmmo ); // 총알 개수를 감소시킵니다.

	SetHUDAmmo();
}

void ABaseWeapon::AddAmmo(int32 AmmoToAdd) // 총알을 추가하는 함수
{
	Ammo = FMath::Clamp( Ammo - AmmoToAdd, 0, MaxAmmo ); // 총알 개수를 증가시키고 최대값을 초과하지 않도록 합니다.

	SetHUDAmmo();
}

bool ABaseWeapon::IsEmptyAmmo() 
{
	return Ammo <= 0; // 총알 개수가 0 이하이면 true를 반환합니다.
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

void ABaseWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
    int32 OtherBodyIndex)
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
	WeaponState = NewState; // 새로운 상태로 설정합니다. 이때 변수 변경하면서 클라이언트에서도 똑같은 함수 실행
	switch (WeaponState)
    {
        case EWeaponState::EWS_Equiped:
            ShowPickUpWidget(false); // 무기가 장착되면 위젯을 숨깁니다.
            CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); 
			WeaponMesh->SetSimulatePhysics(false); 
			WeaponMesh->SetEnableGravity(false); 
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); 
            break;
        case EWeaponState::EWS_Dropped:
            if( HasAuthority() ) // 서버에서만 실행
			{
				CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // 충돌을 활성화합니다.
			}
			WeaponMesh->SetSimulatePhysics(true); // 물리 시뮬레이션을 활성화합니다.
			WeaponMesh->SetEnableGravity(true); // 중력을 활성화합니다.
			WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 쿼리 및 물리 충돌을 활성화합니다.
            break;
        default:
            break;
    }
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


