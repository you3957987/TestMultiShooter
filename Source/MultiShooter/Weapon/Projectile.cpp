#include "Projectile.h"
#include "Components/BoxComponent.h" // BoxComponent 헤더 파일을 포함합니다.
#include "GameFramework/ProjectileMovementComponent.h" // ProjectileMovementComponent 헤더 파일을 포함합니다.
#include "Kismet/GameplayStatics.h" // GameplayStatics 헤더 파일을 포함합니다.
#include "Particles/ParticleSystemComponent.h" // ParticleSystemComponent 헤더 파일을 포함합니다.
#include "Particles/ParticleSystem.h" // ParticleSystem 헤더 파일을 포함합니다.
#include "Sound/SoundCue.h" // SoundCue 헤더 파일을 포함합니다.
#include "MultiShooter/Character/BaseCharacter.h" // BaseCharacter 헤더 파일을 포함합니다.
#include "MultiSHooter/MultiShooter.h" // MultiShooter 헤더 파일을 포함합니다. -> 스켈레탈 콜리전 정의 가져오기

AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true; // 이 액터가 네트워크에서 복제되도록 설정합니다.
	
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox); // 충돌 박스 컴포넌트를 루트 컴포넌트로 설정합니다.

	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic); // 충돌 객체 유형을 설정합니다.
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 쿼리 및 물리 충돌을 활성화합니다.
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore); // 모든 채널에 대해 충돌 응답을 무시합니다.
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECR_Block); // 가시성 채널에 대해서는 충돌 응답을 차단합니다.
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECR_Block); // 월드 정적 채널에 대해서는 충돌 응답을 차단합니다.
	CollisionBox->SetCollisionResponseToChannel( ECC_SkeletalMesh, ECR_Block); // 폰 채널에 대해서는 충돌 응답을 차단합니다.

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true; // 발사체가 이동 방향을 따라 회전하도록 설정합니다.

}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	if( TraceParticle )
	{
		TraceParticleComponent = UGameplayStatics::SpawnEmitterAttached(
			TraceParticle,
			CollisionBox, // 충돌 박스에 파티클을 부착합니다.
			FName(),
			GetActorLocation(), // 현재 액터의 위치에 파티클을 생성합니다.
			GetActorRotation(), // 현재 액터의 회전으로 파티클을 생성합니다.
			EAttachLocation::KeepWorldPosition // 월드 위치를 유지하면서 부착합니다.
		);
	}

	if( HasAuthority() )
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit); // 충돌 시 OnHit 함수를 호출하도록 설정합니다.
	}
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::OnHit( UPrimitiveComponent* HitComp,  AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit )
{ 

	/*
	ABaseCharacter* HitCharacter = Cast<ABaseCharacter>(OtherActor);

	if( HitCharacter ) // 충돌한 액터가 ABaseCharacter이고, 로컬 플레이어가 조종하는 캐릭터인 경우
	{
		HitCharacter->MulticastPlayHitReactMontage(); // 적에게 맞았을 때 애니메이션 몽타주를 재생합니다. 이때 멀티캐스트 함수 호출
	}
	*/

	Destroy(); // 충돌 후 발사체를 파괴합니다.
}

void AProjectile::Destroyed() // 이 함수는 언리얼 자체적으로 서버에서 호출 시 자동으로 클라이언트에 복제됨 그레서 아래 임팩트 파티클이랑 사운드를 복제할 필요 없음
{
	Super::Destroyed();

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
}

