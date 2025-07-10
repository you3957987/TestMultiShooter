#include "ProjectileGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h" // 발사체 이동 컴포넌트 사용을 위한 헤더 파일
#include "Kismet/GameplayStatics.h" // 사운드 재생을 위한 Kismet 유틸리티 함수 사용
#include "Sound/SoundCue.h" // 사운드 큐 사용을 위한 헤더 파일

AProjectileGrenade::AProjectileGrenade()
{
    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh")); // 로켓 메쉬 컴포넌트 생성
    ProjectileMesh->SetupAttachment(RootComponent); // 루트 컴포넌트에 첨부
    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 쿼리 및 물리 충돌 활성화

    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true; // 발사체가 이동 방향을 따라 회전하도록 설정합니다.
    ProjectileMovementComponent->SetIsReplicated(true); // 발사체 이동 컴포넌트를 네트워크에서 복제 가능하도록 설정합니다.

    ProjectileMovementComponent->bShouldBounce = true; // 발사체가 벽에 부딪히면 튕기도록 설정합니다.
}

void AProjectileGrenade::BeginPlay()
{
    AActor::BeginPlay(); // 부모 클래스의 BeginPlay 호출

    StartDestroyTimer(); // 파괴 타이머 시작
    SpawnTrailSystem(); // 발사체의 궤적을 나타내는 파티클 시스템 생성

    if( ProjectileMovementComponent )
    {
        ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce); // 발사체가 벽에 부딪힐 때 호출되는 함수 바인딩
    }
}

void AProjectileGrenade::OnBounce(const FHitResult& Hit, const FVector& ImpactVelocity)
{
    if (BounceCount < 3 && BounceSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, BounceSound, GetActorLocation());
        BounceCount++;
    }
}

void AProjectileGrenade::Destroyed()
{
    ExplodeDamage(); // 폭발 피해를 입히는 함수 호출
    
    Super::Destroyed(); // 부모 클래스의 Destroyed 호출
}

