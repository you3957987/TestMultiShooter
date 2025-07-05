#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"


void AProjectileBullet:: OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{

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