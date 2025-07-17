#include "Projectile.h"
#include "Components/BoxComponent.h" // BoxComponent 헤더 파일을 포함합니다.
#include "Kismet/GameplayStatics.h" // GameplayStatics 헤더 파일을 포함합니다.
#include "Particles/ParticleSystemComponent.h" // ParticleSystemComponent 헤더 파일을 포함합니다.
#include "Particles/ParticleSystem.h" // ParticleSystem 헤더 파일을 포함합니다.
#include "Sound/SoundCue.h" // SoundCue 헤더 파일을 포함합니다.
#include "MultiShooter/Character/BaseCharacter.h" // BaseCharacter 헤더 파일을 포함합니다.
#include "MultiSHooter/MultiShooter.h" // MultiShooter 헤더 파일을 포함합니다. -> 스켈레탈 콜리전 정의 가져오기
#include "NiagaraFunctionLibrary.h" // 나이아가라 기능 라이브러리 헤더 파일
#include "NiagaraComponent.h" // 나이아가라 컴포넌트 사용을 위한 헤더 파일

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

// 이 함수는 언리얼 자체적으로 서버에서 호출 시 자동으로 클라이언트에 복제됨 그레서 아래 임팩트 파티클이랑 사운드를 복제할 필요 없음
void AProjectile::Destroyed() 
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

// 발사체가 생성될 때 나이아가라 트레일 시스템을 생성합니다.
void AProjectile::SpawnTrailSystem()
{
	if (TrailSystem)
    {
        TrailComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            TrailSystem, 
            GetRootComponent(), 
            FName(), 
            GetActorLocation(), 
            GetActorRotation(), 
            EAttachLocation::KeepWorldPosition, 
            false);
    }
}

// 발사체가 파괴되기 전에 타이머를 시작합니다. 특히 로켓 프로젝타일에서 사용
void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(DestroyTimer, this, &AProjectile::DestroyTimerFinished, DestroyTime); // 파괴 타이머 설정
}

void AProjectile::DestroyTimerFinished()
{
    Destroy(); // 파괴 타이머가 끝나면 발사체를 파괴합니다.
}

void AProjectile::ExplodeDamage()
{
    APawn* FiringPawn = GetInstigator(); // 발사체를 발사한 폰을 가져옵니다.

    if( FiringPawn && HasAuthority() )
    {
        AController* FiringController = FiringPawn->GetController(); // 발사체를 발사한 폰의 컨트롤러를 가져옵니다.
        if( FiringController )
        {
            UGameplayStatics::ApplyRadialDamageWithFalloff // 범위 대미지
            (
                this, // 발사체
                Damage, // 피해량
                10.f, // 최소 피해량
                GetActorLocation(), // 발사체의 위치
                DamageInnerRadius, // 내부 반경
                DamageOuterRadius, // 외부 반경
                1.f, // 피해 감소 비율 == 멀어질수록 피해가 감소
                UDamageType::StaticClass(), // 피해 타입
                TArray<AActor*>(), // 무시할 액터 목록 (빈 배열)
                this, // 피해를 주입한 액터
                FiringController // 피해를 준 컨트롤러
            );
        }
    }
}
