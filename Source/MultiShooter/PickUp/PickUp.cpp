#include "PickUp.h"
#include "Kismet/GameplayStatics.h" // 사운드 재생을 위한 Kismet 유틸리티 함수 사용
#include "Sound/SoundCue.h" // 사운드 큐 사용을 위한 헤더 파일
#include "Components/SphereComponent.h" // SphereComponent 헤더 파일을 포함합니다.
#include "MultiShooter/Weapon/WeaponType.h" // 무기 타입을 정의한 헤더 파일
#include "NiagaraComponent.h" // 나이아가라 컴포넌트 헤더 파일
#include "NiagaraFunctionLibrary.h" // 나이아가라 유틸리티 함수 헤더 파일

APickUp::APickUp()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true; // 이 액터는 네트워크에서 복제됩니다.

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent")); // 루트 컴포넌트 생성

	OverlapShere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere")); // 오버랩 스피어 컴포넌트 생성
	OverlapShere->SetupAttachment(RootComponent); // 루트 컴포넌트에 첨부
	OverlapShere->SetSphereRadius(150.f); // 오버랩 스피어의 반지름 설정
	OverlapShere->SetCollisionEnabled (ECollisionEnabled::QueryOnly); // 쿼리 전용으로 설정
	OverlapShere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); // 모든 채널에 대해 충돌 응답을 무시합니다.
	OverlapShere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap); // Pawn 채널에 대해서는 충돌 응답을 겹침으로 설정합니다.
	OverlapShere->AddLocalOffset(FVector(0.f, 0.f, 85.f)); // 오버랩 스피어를 약간 위로 이동시킵니다.

	PickUpMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickUpMesh")); // 아이템 획득 메쉬 컴포넌트 생성
	PickUpMesh->SetupAttachment(OverlapShere); // 루트 컴포넌트에 첨부
	PickUpMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 충돌을 비활성화합니다. 나중에 필요할 때 활성화할 수 있습니다.
	PickUpMesh->SetRenderCustomDepth(true); // 커스텀 깊이 스텐실 값을 활성화합니다. 윤곽선 색상을 설정하기 위해 사용됩니다.
	PickUpMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE); // 무기 메

	PickUpEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickUpEffectComponent")); // 체력 회복 이펙트 컴포넌트 생성
    PickUpEffectComponent->SetupAttachment(RootComponent); // 루트 컴포넌트에 첨부합니다.
}

// 생성 되자마자 캘릭터 있으면 오버랩 이벤트 바인딩도 전에 사라지므로 타이머 설정!
void APickUp::BeginPlay()
{
	Super::BeginPlay();

	if( HasAuthority() )
	{
		GetWorldTimerManager().SetTimer(BindOverlapTimer, 
			this, 
			&APickUp::FinishBindOverlapTimer, 
			BindOverlapTime); 
		// 오버랩 타이머를 설정합니다. 일정 시간 후에 FinishBindOverlapTimer 함수를 호출합니다.
	}
}

void APickUp::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if( PickUpMesh )
	{
		PickUpMesh->AddLocalRotation(FRotator(0.f, BaseTurnRate * DeltaTime, 0.f)); // 아이템 메쉬를 회전시킵니다.
	}

}

// 픽업 상속 받는 자식들에서 구현
void APickUp::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	
}

// 타이머가 끝나면 오버랩 이벤트를 바인딩합니다.
void APickUp::FinishBindOverlapTimer()
{
	if( HasAuthority() )
	{
		OverlapShere->OnComponentBeginOverlap.AddDynamic(this, &APickUp::OnSphereOverlap); // 오버랩 시작 이벤트에 대한 바인딩
	}
}

// 오버라이드
void APickUp::Destroyed()
{
	Super::Destroyed();

	if (PickUpSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, PickUpSound, GetActorLocation()); // 아이템 획득 사운드 재생
	}
	if (PickUpEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation( this, PickUpEffect, GetActorLocation(), GetActorRotation()); // 나이아가라 이펙트 생성
    }
}

