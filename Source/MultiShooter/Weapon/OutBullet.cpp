#include "OutBullet.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AOutBullet::AOutBullet()
{
	PrimaryActorTick.bCanEverTick = false; // 이 액터는 매 프레임마다 업데이트할 필요가 없으므로 false로 설정합니다.

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root")); // 루트 컴포넌트 생성
	SetRootComponent(Root); // 루트 컴포넌트를 설정합니다.

	BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
	BulletMesh->SetupAttachment(Root); // 루트 컴포넌트에 첨부합니다.
	BulletMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); // 카메라 채널에 대해 충돌 무시
	BulletMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore); // 카메라 채널에 대해 충돌 무시
	BulletMesh->SetSimulatePhysics(true); // 물리 시뮬레이션 활성화
	BulletMesh->SetEnableGravity(true); // 중력 활성화
	BulletMesh->SetUseCCD(true); // 연속 충돌 감지 활성화
	BulletMesh->SetNotifyRigidBodyCollision(true); // 강체 충돌 알림 활성화

	ImpulsePwoer = 50.f; // 초기 임펄스 파워 설정

	SetLifeSpan(5.0f); // 5초 후 자동 파괴

	IsSoundPlayed = false; // 소리가 재생되었는지 여부 초기화
}

void AOutBullet::BeginPlay()
{
	Super::BeginPlay();

	BulletMesh->AddImpulse( -GetActorForwardVector() * ImpulsePwoer); // 액터의 전방 방향으로 임펄스를 추가합니다.
	BulletMesh->OnComponentHit.AddDynamic(this, &AOutBullet::OnHit); // Hit 이벤트에 대한 바인딩

}

void AOutBullet::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AOutBullet::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if( BulletSound && !IsSoundPlayed ) // 총알 소리가 설정되어 있고 아직 재생되지 않았다면
	{
		UGameplayStatics::PlaySoundAtLocation(this, BulletSound, GetActorLocation()); // 총알 소리 재생
		IsSoundPlayed = true; // 소리가 재생되었음을 표시 소리는 한번만
	}
}



