#include "FlagZone.h"
#include "Components/BoxComponent.h"
#include "MultiShooter/TypeHeader/Team.h"
#include "MultiShooter/Weapon/Flag.h"
#include "MultiShooter/GameMode/FlagGameMode.h"

AFlagZone::AFlagZone()
{
	PrimaryActorTick.bCanEverTick = false;

	ZoneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ZoneMesh"));
	SetRootComponent(ZoneMesh);

	ZoneBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ZoneBox"));
	ZoneBox->SetupAttachment(ZoneMesh); // ZoneSphere를 ZoneMesh에 첨부
}

void AFlagZone::BeginPlay()
{
	Super::BeginPlay();
	
	if (ZoneBox)
	{
		ZoneBox->OnComponentBeginOverlap.AddDynamic(this, &AFlagZone::OnBoxOverlapBegin); // 콜리전 컴포넌트의 겹침 시작 이벤트에 함수 바인딩
	}
}

void AFlagZone::OnBoxOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFlag* OverlappingFlag = Cast<AFlag>(OtherActor);
	if (OverlappingFlag && OverlappingFlag->GetTeam() != Team) // 캐릭터가 상대 존에 깃발 가지고 들어감
	{
		AFlagGameMode* FlagGameMode = Cast<AFlagGameMode>(GetWorld()->GetAuthGameMode());
		if (FlagGameMode)
		{
			FlagGameMode->FlagCaptured(OverlappingFlag, this); // 깃발 게임 모드에 깃발이 잡혔음을 알림
		}
		OverlappingFlag->ResetFlag(); // 깃발을 초기 위치로 리셋
	}
}



