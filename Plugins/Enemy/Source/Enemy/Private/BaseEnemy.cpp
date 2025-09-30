#include "BaseEnemy.h"
#include "Kismet/GameplayStatics.h" // UGameplayStatics 사용을 위한 헤더 파일
#include "Components/SphereComponent.h" // USphereComponent 사용을 위한 헤더 파일

ABaseEnemy::ABaseEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	AttackRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackRangeSphere"));
	AttackRangeSphere->SetupAttachment(RootComponent); // 루트 컴포넌트

	DetectRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AcceptRangeSphere"));
	DetectRangeSphere->SetupAttachment(RootComponent); // 루트 컴포넌트

	ChaseRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ChaseRangeSphere"));
	ChaseRangeSphere->SetupAttachment(RootComponent); // 루트 컴포넌트

	AttackPoint = CreateDefaultSubobject<USceneComponent>(TEXT("AttackPoint"));
	AttackPoint->SetupAttachment(RootComponent); // 루트 컴포넌트
	
	AttackRangePointSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackRangePointSphere"));
	AttackRangePointSphere->SetupAttachment(AttackPoint); // AttackPoint에 부착
	
	Tags.Add(FName("Enemy")); // 적 캐릭터 태그 추가 -> 이걸 이용해서 프로젝트에서 플러그인 에너미 접근. 매우 중요!!!!
}

void ABaseEnemy::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth; // 시작할 때 체력을 최대 체력으로 설정
	
}

void ABaseEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PollInit();
}

void ABaseEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseEnemy::PollInit()
{
	if ( bTargetInitalize == false )
	{
		TargetCharacter = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0); // 월드에서 첫 번째 플레이어 캐릭터를 가져옵니다.
		if ( TargetCharacter ) // 캐릭터가 유효한지 확인합니다.
		{
			bTargetInitalize = true; // 캐릭터가 유효하면 초기화 플래그를 true로 설정합니다.
		}
	}
}

void ABaseEnemy::Attack()
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy Attack") );
}

float ABaseEnemy::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	float DamageToApply = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	UE_LOG(LogTemp, Warning, TEXT("Enemy Take Damage : %f"), DamageToApply);
	
	return DamageToApply;
}

#if WITH_EDITOR
void ABaseEnemy::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	// 변경된 프로퍼티의 이름을 가져옵니다.
	const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	// AttackRange 프로퍼티가 변경되었는지 확인합니다.
	if (PropertyName == GET_MEMBER_NAME_CHECKED(ABaseEnemy, AttackRange))
	{
		if (AttackRangeSphere)
		{
			// AttackRangeSphere의 반지름을 AttackRange 값으로 설정합니다.
			AttackRangeSphere->SetSphereRadius(AttackRange);
		}
	}
	if ( PropertyName == GET_MEMBER_NAME_CHECKED(ABaseEnemy, DetectRange) )
	{
		if ( DetectRangeSphere )
		{
			DetectRangeSphere->SetSphereRadius(DetectRange);
		}
	}
	if ( PropertyName == GET_MEMBER_NAME_CHECKED(ABaseEnemy, ChaseRange) )
	{
		if ( ChaseRangeSphere )
		{
			ChaseRangeSphere->SetSphereRadius(ChaseRange);
		}
	}
}
#endif

