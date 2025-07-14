#include "GrenadePickUp.h"
#include "MultiShooter/Character/BaseCharacter.h" // 캐릭터 클래스 헤더 파일
#include "MultiShooter/CharacterComponent/CombatComponent.h" // 전투 컴포넌트 헤더 파일

void AGrenadePickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(OtherActor); // 오버랩된 액터가 캐릭터인지 확인
    if (BaseCharacter)
    {
        UCombatComponent* CombatComponent = BaseCharacter->GetCombatComponent(); // 캐릭터의 전투 컴포넌트를 가져옵니다.
        if (CombatComponent)
        {
			CombatComponent->PickUpGrenade(GrenadeAmount); // 전투 컴포넌트의 수류탄 획득 함수 호출
        }
    }
    Destroy(); // 아이템 획득 후 파괴
}

