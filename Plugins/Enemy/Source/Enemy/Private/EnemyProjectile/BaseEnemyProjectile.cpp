#include "EnemyProjectile/BaseEnemyProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

ABaseEnemyProjectile::ABaseEnemyProjectile()
{
	PrimaryActorTick.bCanEverTick = true;

	// 충돌 컴포넌트 생성 및 루트 컴포넌트로 설정
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	RootComponent = CollisionComp;

	// 외형을 표시할 스태틱 메시 컴포넌트 생성
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent); // 충돌 컴포넌트에 부착

	// 투사체 이동 컴포넌트 생성
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp; // 이동을 적용할 컴포넌트 설정
	ProjectileMovement->bRotationFollowsVelocity = true; // 발사체가 날아가는 방향으로 회전
	ProjectileMovement->ProjectileGravityScale = 0.0f; // 중력 영향을 받지 않도록 설정

	ProjectileMovement->InitialSpeed = ProjectileSpeed; // 초기 속도 설정
	ProjectileMovement->MaxSpeed = ProjectileSpeed; // 최대 속도 설정
	
}

void ABaseEnemyProjectile::BeginPlay()
{
	Super::BeginPlay();

	if ( CollisionComp )
	{
		CollisionComp->OnComponentHit.AddDynamic(this, &ABaseEnemyProjectile::OnHit);
	}
}

void ABaseEnemyProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseEnemyProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	// 자기 자신이나 소유자와 충돌한 경우는 무시합니다.
	if (OtherActor && (OtherActor != this) && (OtherActor != GetOwner()))
	{
		// 충돌한 액터가 "Player" 태그를 가지고 있는지 확인합니다.
		if (OtherActor->ActorHasTag(FName("Player")))
		{
			UE_LOG(LogTemp, Warning, TEXT("Projectile hit Player: %s"), *OtherActor->GetName());
			
		}
		// 충돌 후 투사체를 파괴합니다.
		Destroy();
	}
}

void ABaseEnemyProjectile::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 변경된 프로퍼티의 이름을 가져옵니다.
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	
	// AttackRange 프로퍼티가 변경되었는지 확인합니다.
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ABaseEnemyProjectile, ProjectileSpeed))
	{
		if (ProjectileMovement)
		{
			// AttackRangeSphere의 반지름을 AttackRange 값으로 설정합니다.
			ProjectileMovement->InitialSpeed = ProjectileSpeed;
			ProjectileMovement->MaxSpeed = ProjectileSpeed;
		}
	}
}

