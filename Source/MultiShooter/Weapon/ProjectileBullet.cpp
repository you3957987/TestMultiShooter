#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h" // 발사체 이동 컴포넌트 헤더 파일


AProjectileBullet::AProjectileBullet()
{
    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true; // 발사체가 이동 방향을 따라 회전하도록 설정합니다.
    ProjectileMovementComponent->SetIsReplicated(true); // 발사체 이동 컴포넌트를 네트워크에서 복제 가능하도록 설정합니다.
}

void AProjectileBullet:: OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if( OtherActor == GetOwner() ) return; // 발사체의 소유자와 충돌한 경우 무시합니다.
    
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()); // 발사체의 소유자를 캐릭터로 캐스팅
    if (OwnerCharacter)
    {
        AController* OwnerController = OwnerCharacter->GetController(); // 캐릭터의 컨트롤러를 가져옴
        if (OwnerController)
        {
            UGameplayStatics::ApplyDamage( 
                OtherActor, // 피해를 입힐 대상
                Damage, // 피해량
                OwnerController, // 피해를 입히는 캐릭터의 컨트롤러
                this, // 피해를 입히는 액터 (이 경우 발사체)
                UDamageType::StaticClass() // 피해 타입
            );
        }
    }
    Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit); // 부모 클래스의 OnHit 호출 이떄 부모에서는 디스트로이가 있으므로 이거는 마지막에!!!!!
}