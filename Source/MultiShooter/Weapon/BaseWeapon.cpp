#include "BaseWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h" // 네트워크 관련 헤더 파일
#include "Components/WidgetComponent.h" // 위젯 컴포넌트 헤더 파일
#include "MultiShooter/Character/BaseCharacter.h" // 캐릭터 헤더 파일
#include "OutBullet.h" // 총알 헤더 파일
#include "Kismet/GameplayStatics.h" // 게임 관련 유틸리티 함수들을 사용하기 위한 헤더 파일

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

void ABaseWeapon::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) // 이 함수는 서버에서만 실행. 서버만 오버랩 되게 설정함
{
	//UE_LOG(LogTemp, Warning, TEXT("OnSphereOverlap called with OtherActor: %s"), *OtherActor->GetName());
	ABaseCharacter* Character = Cast<ABaseCharacter>(OtherActor);
	if( Character )
	{
		//PickUpWidget->SetVisibility(true); // 위젯을 보이도록 설정합니다.
		Character->SetOverlappingWeapon(this); // 캐릭터에게 현재 무기를 설정합니다.
	}
}

void ABaseWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex) // 오버랩 종료 함수 
{
	ABaseCharacter* Character = Cast<ABaseCharacter>(OtherActor);
	if( Character )
	{
		//PickUpWidget->SetVisibility(false); // 위젯을 숨깁니다.
		Character->SetOverlappingWeapon(nullptr); // 캐릭터의 오버랩 무기를 nullptr로 설정합니다.
	}
}

void ABaseWeapon::ShowPickUpWidget(bool bShow)
{
	if( PickUpWidget )
	{
		PickUpWidget->SetVisibility(bShow); // 위젯의 가시성을 설정합니다.
	}
}

void ABaseWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 무기 상태를 네트워크에서 복제합니다.
	DOREPLIFETIME(ABaseWeapon, WeaponState);
}

void ABaseWeapon::OnRep_WeaponState() // 무기 상태가 변경될 때 호출되는 함수. 클라이언트에서만 실행
{
    switch (WeaponState)
    {
        case EWeaponState::EWS_Equiped:
            ShowPickUpWidget(false); // 무기가 장착되면 위젯을 숨깁니다.
            CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 충돌을 비활성화합니다.
            break;
        case EWeaponState::EWS_Dropped:
            ShowPickUpWidget(true); // 무기가 떨어지면 위젯을 보이도록 설정합니다.
            break;
        default:
            break;
    }	
}

void ABaseWeapon::SetWeaponState(EWeaponState NewState) // 무기 상태를 설정하는 함수
{
	WeaponState = NewState; // 새로운 상태로 설정합니다. 이때 변수 변경하면서 클라이언트에서도 똑같은 함수 실행
	switch (WeaponState)
    {
        case EWeaponState::EWS_Equiped:
            ShowPickUpWidget(false); // 무기가 장착되면 위젯을 숨깁니다.
            CollisionSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 충돌을 비활성화합니다.
            break;
        case EWeaponState::EWS_Dropped:
            ShowPickUpWidget(true); // 무기가 떨어지면 위젯을 보이도록 설정합니다.
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
}

