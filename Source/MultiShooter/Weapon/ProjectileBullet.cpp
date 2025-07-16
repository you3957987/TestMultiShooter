#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "MultiShooter/Character/BaseCharacter.h" // ABaseCharacter 클래스 사용을 위한 헤더 파일
#include "MultiShooter/PlayerController/BasePC.h" // ABasePC 클래스 사용을 위한 헤더 파일
#include "MultiShooter/CharacterComponent/LagCompensationComponent.h" // ULagCompensationComponent 클래스 사용을 위한 헤더 파일
#include "GameFramework/ProjectileMovementComponent.h" // 발사체 이동 컴포넌트 헤더 파일


AProjectileBullet::AProjectileBullet()
{
    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true; // 발사체가 이동 방향을 따라 회전하도록 설정합니다.
    ProjectileMovementComponent->SetIsReplicated(true); // 발사체 이동 컴포넌트를 네트워크에서 복제 가능하도록 설정합니다.
	ProjectileMovementComponent->InitialSpeed = InitialSpeed; // 발사체의 초기 속도를 설정합니다.
	ProjectileMovementComponent->MaxSpeed = InitialSpeed; // 발사체의 최대 속도를 설정합니다.
}

void AProjectileBullet::BeginPlay()
{
    Super::BeginPlay(); // 부모 클래스의 BeginPlay 호출
    
    /*
	FPredictProjectilePathParams PathParams; // 발사체 경로 예측 파라미터 구조체 생성
	PathParams.bTraceWithChannel = true; // 채널을 사용하여 경로 예측
	PathParams.bTraceWithCollision = true; // 충돌을 사용하여 경로 예측
	PathParams.DrawDebugTime = 5.0f; // 디버그용 경로 예측 시간 설정
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration; // 디버그 경로 예측 타입 설정
	PathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed; // 발사체의 초기 속도를 설정합니다.
	PathParams.MaxSimTime = 4.0f; // 최대 시뮬레이션 시간 설정
	PathParams.ProjectileRadius = 5.f; // 발사체의 반지름 설정
	PathParams.SimFrequency = 30.f; // 시뮬레이션 빈도 설정 == 높으면 더 정확한 예측이 가능하지만 성능에 영향을 줄 수 있습니다.
	PathParams.StartLocation = GetActorLocation(); // 발사체의 시작 위치 설정
	PathParams.TraceChannel = ECollisionChannel::ECC_Visibility; // 경로 예측에 사용할 충돌 채널 설정
	PathParams.ActorsToIgnore.Add(this); // 발사체 자신을 경로 예측에서 무시하도록 설정

	FPredictProjectilePathResult PathResult; // 발사체 경로 예측 결과 구조체 생성 

	UGameplayStatics::PredictProjectilePath(
        this, // 발사체 액터
        PathParams, // 경로 예측 파라미터
        PathResult // 경로 예측 결과
    );
    */
    
}

void AProjectileBullet:: OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
    FVector NormalImpulse, const FHitResult& Hit)
{
    if( OtherActor == GetOwner() ) return; // 발사체의 소유자와 충돌한 경우 무시합니다.
    
    ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(GetOwner()); // 발사체의 소유자를 캐릭터로 캐스팅
    if (OwnerCharacter)
    {
        ABasePC* OwnerController = Cast<ABasePC>( OwnerCharacter->Controller); // 캐릭터의 컨트롤러를 가져옴
        if (OwnerController)
        {
            if (OwnerCharacter->HasAuthority() && bUseServerSideRewind == false)
            {
                const float TotalDamage = Hit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage; // 헤드샷 여부에 따라 대미지를 결정합니다.


                UGameplayStatics::ApplyDamage(
                    OtherActor, // 피해를 입힐 대상
                    TotalDamage, // 피해량
                    OwnerController, // 피해를 입히는 캐릭터의 컨트롤러
                    this, // 피해를 입히는 액터 (이 경우 발사체)
                    UDamageType::StaticClass() // 피해 타입
                );
				Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit); // 부모 클래스의 OnHit 호출
				return; // 부모 클래스의 OnHit 호출 후 함수 종료
            }

			ABaseCharacter* HitCharacter = Cast<ABaseCharacter>(OtherActor); // 충돌한 액터를 캐릭터로 캐스팅
            if (HitCharacter && bUseServerSideRewind == true && OwnerCharacter->GetLagCompensationComponent() && OwnerCharacter->IsLocallyControlled())
            {
                OwnerCharacter->GetLagCompensationComponent()->ProjectileServerScoreRequest(
                    HitCharacter, // 히트된 캐릭터
                    TraceStart, // 발사체의 시작 위치
                    InitialVelocity, // 발사체의 초기 속도
					OwnerController->GetServerTime() - OwnerController->SingleTripTime // 현재 시간 (서버 시간에서 왕복 시간을 뺀 값)
				); // 서버 사이드 리와인드 요청
            }
        }
    }
    Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit); // 부모 클래스의 OnHit 호출 이떄 부모에서는 디스트로이가 있으므로 이거는 마지막에!!!!!
}

// 에디터 bp에서 프로퍼티가 변경되었을 때 호출되는 함수
#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent); // 부모 클래스의 PostEditChangeProperty 호출

	FName PropertyName = PropertyChangedEvent.Property != nullptr ? PropertyChangedEvent.Property->GetFName() : NAME_None; // 변경된 프로퍼티의 이름을 가져옵니다.
    if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed)) // InitialSpeed 프로퍼티의 이름을 가져옵니다.
    {
        if (ProjectileMovementComponent)
        {
            ProjectileMovementComponent->InitialSpeed = InitialSpeed; // 발사체의 초기 속도를 설정합니다.
            ProjectileMovementComponent->MaxSpeed = InitialSpeed; // 발사체의 최대 속도를 설정합니다.
        }
    }
}
#endif 
