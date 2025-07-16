#include "ProjectileWeapon.h"
#include "Projectile.h" // AProjectile 클래스 사용을 위한 헤더 파일
#include "Components/BoxComponent.h" // BoxComponent 사용을 위한 헤더 파일

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget); // 부모 클래스의 Fire 함수 호출

    APawn* InstigatorPawn = Cast<APawn>(GetOwner());

    FTransform FirePosition = MuzzlePoint->GetComponentTransform();

    FVector ToTarget = HitTarget - FirePosition.GetLocation();
    FRotator TargetRotation = ToTarget.Rotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = GetOwner(); // 발사체의 소유자를 현재 무기로 설정
    SpawnParams.Instigator = InstigatorPawn; // 발사체의 인스티게이터를 현재 무기의 인스티게이터로 설정

    UWorld* World = GetWorld();
	AProjectile* SpawnedProjectile = nullptr;

	if (bUseServerSideRewind == true) // 서버 사이드 리와인드를 사용할 때
    {
        if (InstigatorPawn->HasAuthority()) // 서버
        {
            if (InstigatorPawn->IsLocallyControlled()) // 서버이면서 호스트 는 그냥 복제된거 사용
            {
                SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, FirePosition.GetLocation(), TargetRotation, SpawnParams );
				SpawnedProjectile->bUseServerSideRewind = false; // 로컬에서만 사용
				SpawnedProjectile->Damage = Damage; // 발사체의 피해량 설정
				SpawnedProjectile->HeadShotDamage = HeadShotDamage; // 발사체의 헤드샷 피해량 설정
            }
            else // 서버이지만 로컬 컨트롤러가 아님 즉 서버에서 클라이언트를 처리하는 경우
            {
                SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, FirePosition.GetLocation(), TargetRotation, SpawnParams);
                SpawnedProjectile->bUseServerSideRewind = true;
            }
        }
        else if(InstigatorPawn->HasAuthority() == false) // 클라이언트
        {
            if (InstigatorPawn->IsLocallyControlled()) // 클라이언트이면서 자기가 직접 조종중
            {
                SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, FirePosition.GetLocation(), TargetRotation, SpawnParams);
                SpawnedProjectile->bUseServerSideRewind = true;
				SpawnedProjectile->TraceStart = FirePosition.GetLocation(); // 발사 위치를 설정
				SpawnedProjectile->InitialVelocity =  SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed; // 초기 속도를 설정
            }
            else // 클라이언트이지만 로컬 컨트롤러가 아님 즉 서버에서 클라이언트를 처리하는 경우
            {
                SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, FirePosition.GetLocation(), TargetRotation, SpawnParams);
                SpawnedProjectile->bUseServerSideRewind = false;
            }
        }
    }
	else if (bUseServerSideRewind == false) // 서버 사이드 리와인드를 사용하지 않을 때
    {
        if (InstigatorPawn->HasAuthority()) // 서버에만 
        {
            SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, FirePosition.GetLocation(), TargetRotation, SpawnParams);
            SpawnedProjectile->bUseServerSideRewind = false; // 로컬에서만 사용
            SpawnedProjectile->Damage = Damage; // 발사체의 피해량 설정
            SpawnedProjectile->HeadShotDamage = HeadShotDamage; // 발사체의 헤드샷 피해량 설정
        }
    }
}