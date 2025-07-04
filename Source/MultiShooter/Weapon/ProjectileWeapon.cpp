#include "ProjectileWeapon.h"
#include "Projectile.h" // AProjectile 클래스 사용을 위한 헤더 파일
#include "Components/BoxComponent.h" // BoxComponent 사용을 위한 헤더 파일

void AProjectileWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget); // 부모 클래스의 Fire 함수 호출

    if( !HasAuthority() ) // 서버 권한이 없는 경우
    {
        return; // 서버가 아닌 경우 발사하지 않음
    }

    APawn* InstigatorPawn = Cast<APawn>(GetOwner());

    FTransform FirePosition = MuzzlePoint->GetComponentTransform();

    FVector ToTarget = HitTarget - FirePosition.GetLocation();
    FRotator TargetRotation = ToTarget.Rotation();

    if(  ProjectileClass && InstigatorPawn )
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Owner = GetOwner(); // 발사체의 소유자를 현재 무기로 설정
        SpawnParams.Instigator = InstigatorPawn; // 발사체의 인스티게이터를 현재 무기의 인스티게이터로 설정
        UWorld* World = GetWorld();
        if (World)
        {
            AProjectile* Projectile = World->SpawnActor<AProjectile>(ProjectileClass, FirePosition.GetLocation(), TargetRotation, SpawnParams);
        }
    }

}