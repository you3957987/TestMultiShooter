#include "HitScanWeapon.h"
#include "MultiShooter/Character/BaseCharacter.h" // 캐릭터 클래스 사용을 위한 헤더 파일
#include "MultiShooter/PlayerController/BasePC.h" // 플레이어 컨트롤러 클래스 사용을 위한 헤더 파일
#include "Particles/ParticleSystemComponent.h" // 파티클 시스템을 사용하기 위한 헤더 파일
#include "Sound/SoundCue.h" // 사운드 큐를 사용하기 위한 헤더 파일
#include "Kismet/GameplayStatics.h" // 게임 관련 유틸리티 함수들을 사용하기 위한 헤더 파일
#include "WeaponType.h" // 무기 타입 정의를 위한 헤더 파일
#include "MultiShooter/CharacterComponent/LagCompensationComponent.h" // 지연 보상 컴포넌트 헤더 파일

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
    Super::Fire(HitTarget); // 부모 클래스의 Fire 함수를 호출합니다.

    APawn* OwnerPawn = Cast<APawn>(GetOwner()); // 무기의 소유자를 Pawn으로 캐스팅합니다.
    if (!OwnerPawn) return; // 소유자가 Pawn이 아닌 경우 함수를 종료합니다.

    AController* OwnerController = OwnerPawn->GetController(); // 소유자의 컨트롤러를 가져옵니다.

    if( MuzzlePoint )
    {
        FVector Start = MuzzlePoint->GetComponentLocation(); // 총구 위치를 가져옵니다.

        FHitResult FireHit;
        WeaponTraceHit(Start, HitTarget, FireHit); // 무기 트레이스 히트 함수를 호출하여 충돌 결과를 얻습니다.

        ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(FireHit.GetActor()); // 충돌한 액터가 ABaseCharacter인지 확인
        if (BaseCharacter && OwnerController )
        {
            bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();

            if (HasAuthority() && bCauseAuthDamage) // 서버는 시간 되감기가 필요 없음
            {
				const float TotalDamage = FireHit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage; // 헤드샷 여부에 따라 대미지를 결정합니다.
    
                UGameplayStatics::ApplyDamage(
                    BaseCharacter, // 피해를 입힐 대상
                    TotalDamage, // 피해량
                    OwnerController, // 피해를 입히는 인스티게이터의 컨트롤러
                    this, // 피해를 입히는 액터 (현재 무기)
                    UDamageType::StaticClass() // 피해 유형
                );
            }
            if(HasAuthority() == false && bUseServerSideRewind == true ) // 클라쪽은 시간 되감기가 필요
            {
				BaseOwnerCharacter = BaseOwnerCharacter == nullptr ? Cast<ABaseCharacter>(OwnerPawn) : BaseOwnerCharacter; 
				BaseOwnerContoller = BaseOwnerContoller == nullptr ? Cast<ABasePC>(OwnerController) : BaseOwnerContoller; // 플레이어 컨트롤러가 nullptr이면 캐스팅합니다.
                if (BaseOwnerCharacter && BaseOwnerContoller && BaseOwnerCharacter->GetLagCompensationComponent() && BaseOwnerCharacter->IsLocallyControlled())
                {
                    BaseOwnerCharacter->GetLagCompensationComponent()->ServerScoreRequest(
                        BaseCharacter, // 피해를 입힐 대상
                        Start, // 트레이스 시작 위치
                        HitTarget, // 충돌 지점
                        BaseOwnerContoller->GetServerTime() - BaseOwnerContoller->SingleTripTime // 현재 시간. 서버 시간에 요청을 보낸 시간만 뺴줌( 왕복 시간 아님 )
					);
                }
            }
        }
        if(ImpactParticle) // 충돌 이펙트 파티클이 설정되어 있는 경우
        {
            UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(), // 현재 월드
                ImpactParticle, // 생성할 파티클
                FireHit.ImpactPoint, // 충돌 지점
                FireHit.ImpactNormal.Rotation() // 충돌 방향의 회전값
            );
        }
        if( HitSound ) // 충돌 사운드가 설정되어 있는 경우
        {
            UGameplayStatics::PlaySoundAtLocation(
                this, // 현재 액터
                HitSound, // 재생할 사운드
                FireHit.ImpactPoint // 사운드를 재생할 위치
            );
        }
    }
}

void AHitScanWeapon::WeaponTraceHit( const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit )
{
    UWorld* World = GetWorld(); // 현재 월드를 가져옵니다.
    if (World)
    {   
        FVector End = TraceStart + ( HitTarget - TraceStart ) * 1.25f; //  트레이스 끝점을 계산합니다. 

        World->LineTraceSingleByChannel(
                OutHit, // Hit 결과를 저장할 변수
                TraceStart, // 시작 위치
                End, // 끝 위치
                ECollisionChannel::ECC_Visibility // 충돌 채널
            );
        FVector BeamEnd = End; // 빔의 끝 위치를 타겟 위치로 설정
        if( OutHit.bBlockingHit ) // 충돌이 발생한 경우
        {
            BeamEnd = OutHit.ImpactPoint; // 빔의 끝 위치를 충돌 지점으로 설정
        }
        else
        {
			OutHit.ImpactPoint = End; // 충돌이 없으면 끝 위치를 임팩트 포인트로 설정
        }
        if( BeamParticle ) // 빔 파티클이 설정되어 있는 경우
        {
            UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
                World, // 현재 월드
                BeamParticle, // 생성할 파티클
                TraceStart, // 총구 위치에서 시작
                FRotator::ZeroRotator, // 회전값은 기본값으로 설정
                true // 월드 공간에서 파티클을 생성
            );
            if( Beam )
            {
                Beam->SetVectorParameter(FName("Target"), BeamEnd); // 빔 파티클의 타겟 위치를 설정합니다.
            }
        }
    }
}
