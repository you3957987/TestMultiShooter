#include "ShotGun.h"
#include "MultiShooter/Character/BaseCharacter.h" // 캐릭터 클래스 사용을 위한 헤더 파일
#include "Kismet/GameplayStatics.h" // 게임 관련 유틸리티 함수들을 사용하기 위한 헤더 파일
#include "Particles/ParticleSystemComponent.h" // 파티클 시스템을 사용하기 위한 헤더 파일
#include "Sound/SoundCue.h" // 사운드 큐를 사용하기 위한 헤더 파일

void AShotGun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
    ABaseWeapon::Fire(FVector()); // 부모 클래스의 Fire 함수를 호출합니다.

    APawn* OwnerPawn = Cast<APawn>(GetOwner()); // 무기의 소유자를 Pawn으로 캐스팅합니다.
    if (!OwnerPawn) return; // 소유자가 Pawn이 아닌 경우 함수를 종료합니다.

    AController* OwnerController = OwnerPawn->GetController(); // 소유자의 컨트롤러를 가져옵니다.
    if (MuzzlePoint)
    {
        const FVector Start = MuzzlePoint->GetComponentLocation(); // 총구 위치를 가져옵니다.

        TMap< ABaseCharacter*, uint32 > HitMap; // 히트된 캐릭터와 히트 횟수를 저장할 맵

        for (auto HitTarget : HitTargets)
        {
            FHitResult FireHit;
            WeaponTraceHit(Start, HitTarget, FireHit); // 무기 트레이스 히트 함수를 호출하여 충돌 결과를 얻습니다.

            ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(FireHit.GetActor()); // 충돌한 액터가 ABaseCharacter인지 확인
            if (BaseCharacter )
            {
                if (HitMap.Contains(BaseCharacter)) // 이미 히트된 캐릭터인 경우
                {
                    HitMap[BaseCharacter]++; // 히트 횟수를 증가시킵니다.
                }
                else // 처음 히트된 캐릭터인 경우
                {
                    HitMap.Emplace(BaseCharacter, 1); // 캐릭터와 히트 횟수를 추가합니다.
                }
                if (ImpactParticle) // 충돌 이펙트 파티클이 설정되어 있는 경우
                {
                    UGameplayStatics::SpawnEmitterAtLocation(
                        GetWorld(), // 현재 월드
                        ImpactParticle, // 생성할 파티클
                        FireHit.ImpactPoint, // 충돌 지점
                        FireHit.ImpactNormal.Rotation() // 충돌 방향의 회전값
                    );
                }
                if (HitSound) // 충돌 사운드가 설정되어 있는 경우
                {
                    UGameplayStatics::PlaySoundAtLocation(
                        this, // 현재 액터
                        HitSound, // 재생할 사운드
                        FireHit.ImpactPoint, // 사운드를 재생할 위치
                        .5f,
                        FMath::RandRange(-.5f, 5.f) // 랜덤 피치 조정
                    );
                }
            }
        }
        for (auto HitPair : HitMap) // 히트된 캐릭터와 히트 횟수를 순회합니다.
        {
            if (HitPair.Key && HasAuthority() && OwnerController) // 대미지 주는거는 서버에서만 실행
            {
                UGameplayStatics::ApplyDamage(
                    HitPair.Key, // 피해를 입힐 대상
					Damage * HitPair.Value, // 피해량 * 히트 횟수
                    OwnerController, // 피해를 입히는 인스티게이터의 컨트롤러
                    this, // 피해를 입히는 액터 (현재 무기)
                    UDamageType::StaticClass() // 피해 유형
                );
            }
        }
    }
}

void AShotGun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
    if (MuzzlePoint == nullptr) return;

    const FVector Start = MuzzlePoint->GetComponentLocation(); // 총구 위치를 가져옵니다.

    const FVector ToTargetNormalized = (HitTarget - Start).GetSafeNormal(); // 타겟 방향 벡터를 정규화합니다.
    const FVector SphereCenter = Start + ToTargetNormalized * DinstanceToSphere; // 구의 중심을 계산합니다.

    for (uint32 i = 0; i < NumberOfPellets; i++) // 산탄 갯수만큼 반복
    {
        const FVector RandomScatter = FMath::VRand() * FMath::RandRange(0.f, SphereRadius); // 랜덤한 산탄 효과를 위해 무작위 벡터를 생성합니다.
        const FVector EndLoc = SphereCenter + RandomScatter; // 구의 중심에 랜덤 산탄 벡터를 더하여 최종 위치를 계산합니다.
        FVector ToEndLoc = EndLoc - Start; // 시작 위치에서 최종 위치까지의 벡터를 계산합니다.
		ToEndLoc = Start + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size(); // 최종 위치를 계산합니다.

        HitTargets.Add(TraceEndWithScatter(ToEndLoc));

        /*
        DrawDebugSphere(
            GetWorld(), // 현재 월드
            SphereCenter, // 구의 중심
            SphereRadius, // 구의 반지름
            12, // 구의 세그먼트 수
            FColor::Red, // 구의 색상
            true
        );
        DrawDebugSphere(
            GetWorld(), // 현재 월드
            EndLoc, // 최종 위치
            4.f, // 구의 반지름
            12, // 구의 세그먼트 수
            FColor::Blue, // 구의 색상
            true
        );
        DrawDebugLine(
            GetWorld(), // 현재 월드
            Start, // 시작 위치
            FVector(Start + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()), // 최종 위치
            FColor::Green, // 선의 색상
            true
        );
        */
    }
}

