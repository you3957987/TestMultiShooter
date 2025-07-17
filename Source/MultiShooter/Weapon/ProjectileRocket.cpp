#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h" // 게임 플레이 관련 유틸리티 함수 사용을 위한 헤더 파일
#include "Sound/SoundCue.h" // 사운드 큐 사용을 위한 헤더 파일
#include "Components/BoxComponent.h" // 박스 컴포넌트 사용을 위한 헤더 파일
#include "NiagaraComponent.h" // 나이아가라 컴포넌트 사용을 위한 헤더 파일
#include "Components/AudioComponent.h" // 오디오 컴포넌트 사용을 위한 헤더 파일
#include "RocketMovementComponent.h" // 로켓 이동 컴포넌트 헤더 파일

AProjectileRocket::AProjectileRocket()
{
    ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh")); // 로켓 메쉬 컴포넌트 생성
    ProjectileMesh->SetupAttachment(RootComponent); // 루트 컴포넌트에 첨부

    ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 쿼리 및 물리 충돌 활성화

    RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent")); // 로켓 이동 컴포넌트 생성
    RocketMovementComponent->bRotationFollowsVelocity = true; // 발사체가 이동 방향을 따라 회전하도록 설정
    RocketMovementComponent->SetIsReplicated(true); // 로켓 이동 컴포넌트를 네트워크에서 복제 가능하도록 설정합니다.
}

void AProjectileRocket::BeginPlay()
{
    Super::BeginPlay(); // 부모 클래스의 BeginPlay 호출

    if( !HasAuthority() )
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit); // 충돌 시 OnHit 함수를 호출하도록 설정합니다.
	}

    SpawnTrailSystem(); // 나이아가라 트레일 시스템을 생성합니다.
    
    if ( ProjectileLoop && SoundAttenuation )
    {
        ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
            ProjectileLoop,
            GetRootComponent(),
            FName(),
            GetActorLocation(),
            EAttachLocation::KeepWorldPosition,
            false, // 사운드가 루트 컴포넌트에 첨부됩니다.
            1.0f, // 볼륨
            1.0f, // 피치
            0.0f, // 시작 시간
            SoundAttenuation, // 사운드 감쇠 설정
            (USoundConcurrency*)nullptr, // 사운드 동시성 설정
            false // 루프 여부
        );
    }
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if( OtherActor == GetOwner() ) return; // 발사체의 소유자와 충돌한 경우 무시합니다.
    
	ExplodeDamage(); // 폭발 피해를 입히는 함수 호출 -> 프로젝타일.h에서 선언됨. 즉 부모 클래스의 함수입니다.
    //Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit); // 부모 클래스의 OnHit 호출 == 발사체 파괴 !!!!

    StartDestroyTimer(); // 파괴 타이머를 시작합니다. -> 프로젝타일.h 즉 부모에서 선언됨

    if( ImpactParticle )
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ImpactParticle,
			GetActorTransform() // 충돌 지점에 파티클을 생성합니다.
		);
	}
	if(  ImpactSound )
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			ImpactSound,
			GetActorLocation() // 충돌 지점에서 사운드를 재생합니다.
		);
	}
    if( ProjectileMesh ) ProjectileMesh->SetVisibility(false); // 로켓 메쉬를 숨깁니다. (로켓이 파괴되었음을 나타냅니다.)

    if( CollisionBox ) CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 충돌 박스를 비활성화합니다. (로켓이 파괴되었음을 나타냅니다.)

    if( TrailComponent ) TrailComponent->Deactivate(); // 나이아가라 트레일 컴포넌트를 비활성화합니다. (로켓이 파괴되었음을 나타냅니다.)

    if( ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying() ) ProjectileLoopComponent->Stop(); // 발사체 루프 사운드를 중지합니다.

}

void AProjectileRocket::Destroyed() // 부모 디스트로이 호출 안하고 디스트로이 타이머로 처리!
{
    //Super::Destroyed(); // 부모 클래스의 Destroyed 호출 안하기!!!!!
}

