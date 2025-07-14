#include "InvisiblePickUp.h"
#include "MultiShooter/Character/BaseCharacter.h" // 캐릭터 클래스 헤더 파일
#include "MultiShooter/CharacterComponent/BuffComponent.h" // 전투 컴포넌트 헤더 파일

AInvisiblePickUp::AInvisiblePickUp()
{
	bReplicates = true; // 이 픽업 아이템은 네트워크에서 복제됩니다.
}

void AInvisiblePickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(OtherActor); // 오버랩된 액터가 캐릭터인지 확인
	if (BaseCharacter)
	{
		UBuffComponent* BuffComponent = BaseCharacter->GetBuffComponent(); // 캐릭터의 버프 컴포넌트를 가져옵니다.
		if (BuffComponent)
		{
			
		}
	}
	Destroy(); // 아이템 획득 후 파괴
}

