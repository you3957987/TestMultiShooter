#include "LagCompensationComponent.h"
#include "MultiShooter/Character/BaseCharacter.h" // ABaseCharacter 클래스 사용을 위한 헤더 파일
#include "MultiShooter/Weapon/BaseWeapon.h" // ABaseWeapon 클래스 사용을 위한 헤더 파일
#include "Components/BoxComponent.h"
//#include "DrawDebugHelpers.h" // 디버그 박스 그리기를 위한 헤더 파일
#include "Kismet/GameplayStatics.h" // 게임 관련 유틸리티 함수들을 사용하기 위한 헤더 파일
#include "MultiShooter/MultiShooter.h" // 멀티슈터 헤더 파일을 포함합니다. ->  콜리전 정의 = 히트박스 가져오기

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SetFramePackageOnTick(); // 틱마다 프레임 패키지를 설정하는 함수를 호출합니다.
}

// 틱 마다 프레임 패키지를 설정하는 함수
void ULagCompensationComponent::SetFramePackageOnTick()
{
	if (Character == nullptr || !Character->HasAuthority()) return; // 캐릭터가 nullptr이거나 권한이 없는 경우 함수 종료

	if (FrameHistory.Num() <= 1) // 프레임 히스토리가 1개 이하인 경우
	{
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame);
		FrameHistory.AddHead(ThisFrame); // 현재 프레임 패키지를 리스트의 맨 앞에 추가합니다.
	}
	else
	{
		float HistoryLengh = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time; // 프레임 히스토리의 시간 차 계산
		while (HistoryLengh > MaxRecordTime) // 시간 차가 최대 기록 시간을 초과하면 끝쪽 기록 계속 삭제
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail()); // 시간 차가 최대 기록 시간을 초과하면 가장 오래된 프레임 패키지를 제거합니다.
			HistoryLengh = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time; // 다시 시간 차 계산
		}
		FFramePackage ThisFrame;
		SaveFramePackage(ThisFrame); // 현재 프레임 패키지를 저장합니다.
		FrameHistory.AddHead(ThisFrame); // 현재 프레임 패키지를 리스트의 맨 앞에 추가합니다.

		//ShowFramePackage(ThisFrame, FColor::Red); // 현재 프레임 패키지를 디버그 박스로 표시합니다.
	}
}

// 프레임 패키지에다가 시간이랑 히트박스 정보를 저장하는 함수
void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<ABaseCharacter>(GetOwner()) : Character; // 캐릭터가 nullptr이면 GetOwner()로 캐릭터를 가져옵니다.
	if (Character)
	{
		Package.Time = GetWorld()->GetTimeSeconds(); // 현재 월드의 시간을 패키지에 저장합니다.
		Package.Character = Character; // 현재 캐릭터를 패키지에 저장합니다.

		for (auto& BoxPair : Character->HitCollisionBoxes)
		{
			FBoxInformation BoxInformation;
			BoxInformation.Location = BoxPair.Value->GetComponentLocation(); // 박스 컴포넌트의 위치를 가져옵니다.
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation(); // 박스 컴포넌트의 회전을 가져옵니다.
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent(); // 박스 컴포넌트의 크기를 가져옵니다.
			Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation); // 박스 정보를 패키지에 추가합니다.
		}
	}
}

// 서버 사이드 리와인드 결과를 바탕으로 대미지를 적용하는 함수  서버는 그냥 어플라이 대미지로 하고 클라는 이걸로 대미지 입힘
void ULagCompensationComponent::ServerScoreRequest_Implementation(ABaseCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, 
	const FVector_NetQuantize& HitLocation, float HitTime)
{
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	if (Character && HitCharacter && Character->GetEquippedWeapon() && Confirm.bHitConfirmed)
	{
		const float Damage = Confirm.bHeadShot ? 
			Character->GetEquippedWeapon()->GetHeadShotDamage() : 
			Character->GetEquippedWeapon()->GetDamage(); // 헤드샷 여부에 따라 대미지를 결정합니다.

		UGameplayStatics::ApplyDamage(
			HitCharacter, // 피해를 입힐 대상
			Damage, // 무기의 대미지
			Character->Controller, // 피해를 입히는 인스티게이터의 컨트롤러
			Character->GetEquippedWeapon(), // 피해를 입히는 액터 (현재 무기)
			UDamageType::StaticClass() // 피해 타입 (기본 피해 타입)
		);
	}
}

void ULagCompensationComponent::ProjectileServerScoreRequest_Implementation(ABaseCharacter* HitCharacter, 
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	FServerSideRewindResult Confirm = ProjectileServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime);

	if (Character && HitCharacter && Character->GetEquippedWeapon() && Confirm.bHitConfirmed)
	{
		const float Damage = Confirm.bHeadShot ?
			Character->GetEquippedWeapon()->GetHeadShotDamage() :
			Character->GetEquippedWeapon()->GetDamage(); // 헤드샷 여부에 따라 대미지를 결정합니다.


		UGameplayStatics::ApplyDamage(
			HitCharacter, // 피해를 입힐 대상
			Damage, // 무기의 대미지
			Character->Controller, // 피해를 입히는 인스티게이터의 컨트롤러
			Character->GetEquippedWeapon(), // 피해를 입히는 액터 (현재 무기)
			UDamageType::StaticClass() // 피해 타입 (기본 피해 타입)
		);
	}
}

void ULagCompensationComponent::ShotgunServerScoreRequest_Implementation(const TArray<ABaseCharacter*>& HitCharacters, 
	const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	FShotgunServerSideRewindResult Confirm = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);

	for (auto& HitCharacter : HitCharacters)
	{
		if (HitCharacter == nullptr || HitCharacter->GetEquippedWeapon() == nullptr || Character == nullptr ) continue;
		// 캐릭터나 장착된 무기가 nullptr인 경우 건너뜁니다.
		float TotalDamage = 0.f; // 총 대미지를 저장할 변수
		if (Confirm.HeadShot.Contains(HitCharacter))
		{
			float HeadShotDamage = Confirm.HeadShot[HitCharacter] * Character->GetEquippedWeapon()->GetHeadShotDamage(); // 헤드샷 대미지 계산
			TotalDamage += HeadShotDamage; // 총 대미지에 헤드샷 대미지를 추가합니다.
		}
		if (Confirm.BodyShot.Contains(HitCharacter))
		{
			float BodyShotDamage = Confirm.BodyShot[HitCharacter] * Character->GetEquippedWeapon()->GetDamage(); // 바디샷 대미지 계산
			TotalDamage += BodyShotDamage; // 총 대미지에 바디샷 대미지를 추가합니다.
		}
		
		UGameplayStatics::ApplyDamage(
			HitCharacter, // 피해를 입힐 대상
			TotalDamage, // 무기의 대미지
			Character->Controller, // 피해를 입히는 인스티게이터의 컨트롤러
			HitCharacter->GetEquippedWeapon(), // 피해를 입히는 액터 (현재 무기)
			UDamageType::StaticClass() // 피해 타입 (기본 피해 타입)
		);
	}
}

// 프레임 데이터를 이용한 시간 되감기 == 193 강의. 
// 시간을 되돌려서 타격 시간을 보강할 수 있게 하는거
// 만약 핑이 안좋아서 클라가 쏜 타격 정보를 서버가 받았을 떄 그 자리에 이미 캐릭터가 없다면 서버는 타격을 무시하므로
// 서버가 시간을 돌려서 히트된 캐릭터의 프레임을 찾아서 그 프레임에 맞춰서 히트된 캐릭터를 되돌리는 함수 -> GetFrameToCheck()에서 상세히 있음
FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABaseCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);

	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation); // 최종적으로 히트 확인!!!!!
}

// 발사체용 서버 사이드 리와인드 함수
FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(ABaseCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, 
	const FVector_NetQuantize100& IntialVelocity, float HitTime)
{
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);

	return ProjectileConfirmHit(FrameToCheck, HitCharacter, TraceStart,IntialVelocity, HitTime);
}

// 샷건용 서버 사이드 리와인드 함수
FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunServerSideRewind(const TArray<ABaseCharacter*> HitCharacters, 
	const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	TArray<FFramePackage> FramesToCheck; // 히트된 캐릭터들의 프레임 패키지를 저장할 배열

	for (ABaseCharacter* HitCharacter : HitCharacters)
	{
		FramesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime)); // 히트된 캐릭터들의 프레임 패키지를 가져와서 배열에 추가합니다.
	}

	return ShotgunConfirmHit(FramesToCheck,TraceStart, HitLocations);
}

// FFramePackage를 반환함
FFramePackage ULagCompensationComponent::GetFrameToCheck(ABaseCharacter* HitCharacter, float HitTime)
{
	bool bReturn =
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComponent() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail() == nullptr;
	if (bReturn) return FFramePackage();
	// HitCharacter가 nullptr이거나, LagCompensationComponent가 nullptr이거나, 프레임 히스토리가 비어있으면 함수 종료

	FFramePackage FrameToCheck; // 히트된 캐릭터의 프레임 패키지를 저장할 변수
	bool bShouldInterpolate = true; // 히트된 캐릭터의 프레임 패키지를 보간할지 여부를 저장할 변수

	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComponent()->FrameHistory; // 히트된 캐릭터의 프레임 히스토리를 가져옵니다.
	const float OldesthistoryTime = History.GetTail()->GetValue().Time; // 가장 오래된 프레임의 시간을 가져옵니다.
	const float NewestHistoryTime = History.GetHead()->GetValue().Time; // 가장 최근 프레임의 시간을 가져옵니다.
	if (HitTime < OldesthistoryTime)
	{
		// HitTime이 가장 오래된 프레임의 시간보다 이전이면 함수 종료 즉 너무 오랴되서 시간 되감기를 허지 않음
		return FFramePackage();
	}
	if (OldesthistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue(); // HitTime이 가장 오래된 프레임의 시간과 같으면 가장 오래된 프레임 패키지를 가져옵니다.
		bShouldInterpolate = false; // HitTime이 가장 오래된 프레임의 시간과 같으면 보간하지 않습니다.
	}
	if (NewestHistoryTime <= HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue(); // HitTime이 가장 최근 프레임의 시간보다 크거나 같으면 가장 최근 프레임 패키지를 가져옵니다.
		bShouldInterpolate = false; // HitTime이 가장 최근 프레임의 시간과 같으면 보간하지 않습니다.
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Yonger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Yonger;
	while (Older->GetValue().Time > HitTime)
	{
		// 오래된 시간 Older < 히트 시간 < 더 최근 시간 Yonger 이 될 때까지 반복
		if (Older->GetNextNode() == nullptr) break; // 다음 노드가 없으면 반복 종료
		Older = Older->GetNextNode(); // HitTime보다 오래된 프레임 패키지를 찾습니다.
		if (Older->GetValue().Time > HitTime)
		{
			Yonger = Older; // HitTime보다 오래된 프레임 패키지를 찾으면 Yonger를 업데이트합니다.
		}
	}
	if (Older->GetValue().Time == HitTime) // 가능성은 매우 낮지만, 프레임 체크를 찾긴함
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false; // HitTime이 프레임 패키지의 시간과 같으면 보간하지 않습니다.
	}

	if (bShouldInterpolate == true) // 과거 프레임과 현재 프레임 사이를 보간하기
	{
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Yonger->GetValue(), HitTime);
	}
	FrameToCheck.Character = HitCharacter; // 히트된 캐릭터를 프레임 패키지에 저장합니다.
	return FrameToCheck;
}

// 과거 프레임과 현재 프레임 사이를 보간하는 함수
FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time; // 두 프레임 사이의 시간 차이를 계산합니다.
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0, 1); 
	// HitTime과 두 프레임 사이의 시간 차이를 이용하여 보간 비율을 계산합니다.

	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;

	for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key; // 현재 박스 이름을 가져옵니다.

		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName]; // OlderFrame에서 현재 박스 정보를 찾습니다.
		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName]; // YoungerFrame에서 현재 박스 정보를 찾습니다.

		FBoxInformation InterpBoxInfo;
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent; // 박스 크기는 YoungerFrame의 크기를 그대로 사용합니다.

		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo); // 보간된 박스 정보를 패키지에 추가합니다.
	}

	return InterpFramePackage;
}

// 히트 확인할 프레임 정보랑 캐릭터, 트레이스 시작 위치, 히트 위치를 받아서 히트 확인하는 함수
FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package, ABaseCharacter* HitCharacter, 
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if(HitCharacter == nullptr) return FServerSideRewindResult();

	FFramePackage CurrentFrame;
	CacheBoxPosition(HitCharacter, CurrentFrame); // 현재 위치의 박스 정보를 저장
	MoveBoxes(HitCharacter, Package); // 히트된 캐릭터의 박스 컴포넌트를 패키지의 위치로 이동
	EnabledCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision); // 일단 캐릭터 메쉬 충돌 비활성화. 충돌은 오로지 히트 박스 콜리전에서만

	// 헤드샷 확인 위헤 일단 헤드부터 콜리전 활성화
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 헤드 박스의 충돌을 활성화합니다.
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block); // 헤드 박스가 트레이스 채널에 반응하도록 설정합니다.

	FHitResult ConfirmHitResult; // 히트 결과를 저장할 변수
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f; // 트레이스 끝 위치 계산
	UWorld* World = GetWorld(); // 현재 월드를 가져옵니다.
	if (World)
	{
		World->LineTraceSingleByChannel(
			ConfirmHitResult, // 히트 결과를 저장할 변수
			TraceStart, // 트레이스 시작 위치
			TraceEnd, // 트레이스 끝 위치
			ECC_HitBox // 트레이스 채널
		);
		if (ConfirmHitResult.bBlockingHit) // 헤드샷 발동!!!!
		{
			if (ConfirmHitResult.Component.IsValid())
			{
				/*
				UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
				if (Box)
				{
					DrawDebugBox(World, // 디버그 박스를 그릴 월드
						Box->GetComponentLocation(), // 박스의 위치
						Box->GetScaledBoxExtent(), // 박스의 크기
						FQuat(Box->GetComponentRotation()), // 박스의 회전
						FColor::Red, // 박스 색상
						false, // 지속 시간 없음
						5.f // 5초 동안 표시
					); // 디버그 박스를 그립니다.
				}
				*/
			}

			ResetHitBoxes(HitCharacter, CurrentFrame); // 히트된 캐릭터의 박스 콜리전을 노 콜리전으로 되돌립니다.
			EnabledCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics); // 캐릭터 메쉬 충돌을 활성화합니다.
			return FServerSideRewindResult{ true, true }; // 히트 확인, 헤드샷도 확인
		}
		else // 머리에는 안맞아서 다른 부위도 확인
		{
			for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
			{
				if (HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 박스 컴포넌트의 충돌을 일시적으로 활성화합니다.
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block); // 박스 컴포넌트가 트레이스 채널에 반응하도록 설정합니다.
				}
			}
			World->LineTraceSingleByChannel(
				ConfirmHitResult, // 히트 결과를 저장할 변수
				TraceStart, // 트레이스 시작 위치
				TraceEnd, // 트레이스 끝 위치
				ECC_HitBox // 트레이스 채널
			);
			if (ConfirmHitResult.bBlockingHit) // 몸샷 발동!!!!
			{
				if (ConfirmHitResult.Component.IsValid())
				{
					/*
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if (Box)
					{
						DrawDebugBox(World, // 디버그 박스를 그릴 월드
							Box->GetComponentLocation(), // 박스의 위치
							Box->GetScaledBoxExtent(), // 박스의 크기
							FQuat(Box->GetComponentRotation()), // 박스의 회전
							FColor::Blue, // 박스 색상
							false, // 지속 시간 없음
							5.f // 5초 동안 표시
						); // 디버그 박스를 그립니다.
					}
					*/
				}

				ResetHitBoxes(HitCharacter, CurrentFrame); // 히트된 캐릭터의 박스 콜리전을 노 콜리전으로 되돌립니다.
				EnabledCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics); // 캐릭터 메쉬 충돌을 활성화합니다.
				return FServerSideRewindResult{ true, false }; // 히트 확인, 헤드샷은 아님
			}
		}
	}
	ResetHitBoxes(HitCharacter, CurrentFrame); // 히트된 캐릭터의 박스 콜리전을 노 콜리전으로 되돌립니다.
	EnabledCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics); // 캐릭터 메쉬 충돌을 활성화합니다.
	return FServerSideRewindResult{ false, false }; // 히트 아님, 헤드샷은 아님
}

FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(const FFramePackage& Package, ABaseCharacter* HitCharacter, 
	const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	if (HitCharacter == nullptr) return FServerSideRewindResult();

	FFramePackage CurrentFrame;
	CacheBoxPosition(HitCharacter, CurrentFrame); // 현재 위치의 박스 정보를 저장
	MoveBoxes(HitCharacter, Package); // 히트된 캐릭터의 박스 컴포넌트를 패키지의 위치로 이동
	EnabledCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision); // 일단 캐릭터 메쉬 충돌 비활성화. 충돌은 오로지 히트 박스 콜리전에서만

	// 헤드샷 확인 위헤 일단 헤드부터 콜리전 활성화
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 헤드 박스의 충돌을 활성화합니다.
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block); // 헤드 박스가 트레이스 채널에 반응하도록 설정합니다.

	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithCollision = true; // 충돌을 포함한 경로 예측
	PathParams.MaxSimTime = MaxRecordTime; // 최대 시뮬레이션 시간
	PathParams.LaunchVelocity = InitialVelocity; // 발사체의 초기 속도
	PathParams.StartLocation = TraceStart; // 발사체의 시작 위치
	PathParams.SimFrequency = 15.f; // 시뮬레이션 주파수
	PathParams.ProjectileRadius = 5.f; // 발사체의 반지름
	PathParams.TraceChannel = ECC_HitBox; // 트레이스 채널 설정
	PathParams.ActorsToIgnore.Add(GetOwner()); // 발사체가 무시할 액터 목록에 현재 오너를 추가합니다.
	//PathParams.DrawDebugTime = 5.f; // 디버그 정보를 5초 동안 표시합니다.
	//PathParams.DrawDebugType = EDrawDebugTrace::ForDuration; // 디버그 트레이스 타입 설정

	FPredictProjectilePathResult PathResult;

	UGameplayStatics::PredictProjectilePath(
		this, // 현재 월드
		PathParams, // 발사체 경로 예측 파라미터
		PathResult // 발사체 경로 예측 결과
	);

	if (PathResult.HitResult.bBlockingHit) // 헤드샷 발동!
	{
		if (PathResult.HitResult.Component.IsValid())
		{
			/*
			UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
			if (Box)
			{
				DrawDebugBox(
					GetWorld(), // 디버그 박스를 그릴 월드
					Box->GetComponentLocation(), // 박스의 위치
					Box->GetScaledBoxExtent(), // 박스의 크기
					FQuat(Box->GetComponentRotation()), // 박스의 회전
					FColor::Blue, // 박스 색상
					false, // 지속 시간 없음
					5.f // 5초 동안 표시
				); // 디버그 박스를 그립니다.
			}
			*/
		}
		ResetHitBoxes(HitCharacter, CurrentFrame); // 히트된 캐릭터의 박스 콜리전을 노 콜리전으로 되돌립니다.
		EnabledCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics); // 캐릭터 메쉬 충돌을 활성화합니다.
		return FServerSideRewindResult{ true, true }; // 히트 확인, 헤드샷 맞음

	}
	else // 헤드샷은 아님
	{
		for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 박스 컴포넌트의 충돌을 일시적으로 활성화합니다.
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block); // 박스 컴포넌트가 트레이스 채널에 반응하도록 설정합니다.
			}
		}
		UGameplayStatics::PredictProjectilePath( // 다시 발사체 경로 예측
			this, // 현재 월드
			PathParams, // 발사체 경로 예측 파라미터
			PathResult // 발사체 경로 예측 결과
		);
		if (PathResult.HitResult.bBlockingHit) // 헤드샷 발동!
		{ 
			if (PathResult.HitResult.Component.IsValid())
			{
				/*
				UBoxComponent* Box = Cast<UBoxComponent>(PathResult.HitResult.Component);
				if (Box)
				{
					DrawDebugBox(
						GetWorld(), // 디버그 박스를 그릴 월드
						Box->GetComponentLocation(), // 박스의 위치
						Box->GetScaledBoxExtent(), // 박스의 크기
						FQuat(Box->GetComponentRotation()), // 박스의 회전
						FColor::Blue, // 박스 색상
						false, // 지속 시간 없음
						5.f // 5초 동안 표시
					); // 디버그 박스를 그립니다.
				}
				*/
			}
			ResetHitBoxes(HitCharacter, CurrentFrame); // 히트된 캐릭터의 박스 콜리전을 노 콜리전으로 되돌립니다.
			EnabledCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics); // 캐릭터 메쉬 충돌을 활성화합니다.
			return FServerSideRewindResult{ true, false }; // 히트 확인, 헤드샷은 아님
		}
	}

	ResetHitBoxes(HitCharacter, CurrentFrame); // 히트된 캐릭터의 박스 콜리전을 노 콜리전으로 되돌립니다.
	EnabledCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics); // 캐릭터 메쉬 충돌을 활성화합니다.
	return FServerSideRewindResult{ false, false }; // 히트 아님, 헤드샷은 아님
}

// 샷건용 히트 확인 함수. 그냥 총알 하나하나 히트 확인하는 거니깐 그냥 컨펌 힛 함수 보는게 좋을듯
FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunConfirmHit(const TArray<FFramePackage>& Packages, const FVector_NetQuantize& TraceStart, 
	const TArray<FVector_NetQuantize>& HitLocations)
{
	for (auto& Frame : Packages)
	{
		if (Frame.Character == nullptr) return FShotgunServerSideRewindResult();
	}

	FShotgunServerSideRewindResult ShotgunResult; // 샷건 히트 확인 결과를 저장할 변수
	TArray < FFramePackage > CurrentFrames; // 현재 프레임 패키지를 저장할 배열

	for (auto& Frame : Packages)
	{
		FFramePackage CurrentFrame;
		CurrentFrame.Character = Frame.Character; // 현재 프레임 패키지에 캐릭터를 저장합니다.
		CacheBoxPosition(Frame.Character, CurrentFrame); // 현재 위치의 박스 정보를 저장
		MoveBoxes(Frame.Character, Frame); // 히트된 캐릭터의 박스 컴포넌트를 패키지의 위치로 이동
		EnabledCharacterMeshCollision(Frame.Character, ECollisionEnabled::NoCollision); // 일단 캐릭터 메쉬 충돌 비활성화. 충돌은 오로지 히트 박스 콜리전에서만
		CurrentFrames.Add(CurrentFrame); // 현재 프레임 패키지를 배열에 추가합니다.
	}

	for (auto& Frame : Packages)
	{
		// 헤드샷 확인 위헤 일단 헤드부터 콜리전 활성화
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 헤드 박스의 충돌을 활성화합니다.
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block); // 헤드 박스가 트레이스 채널에 반응하도록 설정합니다.
	}

	// 헤드샷 확인을 위해 각 히트 위치에 대해 트레이스를 수행합니다.
	UWorld* World = GetWorld(); // 현재 월드를 가져옵니다.
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult; // 히트 결과를 저장할 변수
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f; // 트레이스 끝 위치 계산
		if (World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult, // 히트 결과를 저장할 변수
				TraceStart, // 트레이스 시작 위치
				TraceEnd, // 트레이스 끝 위치
				ECC_HitBox // 트레이스 채널
			);
			ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(ConfirmHitResult.GetActor()); // 히트된 액터를 캐릭터로 변환합니다.
			if (BaseCharacter)
			{
				if (ConfirmHitResult.Component.IsValid())
				{
					/*
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if (Box)
					{
						DrawDebugBox(
							GetWorld(), // 디버그 박스를 그릴 월드
							Box->GetComponentLocation(), // 박스의 위치
							Box->GetScaledBoxExtent(), // 박스의 크기
							FQuat(Box->GetComponentRotation()), // 박스의 회전
							FColor::Red, // 박스 색상
							false, // 지속 시간 없음
							5.f // 5초 동안 표시
						); // 디버그 박스를 그립니다.
					}
					*/
				}

				if (ShotgunResult.HeadShot.Contains(BaseCharacter))
				{
					ShotgunResult.HeadShot[BaseCharacter]++; // 이미 헤드샷이 기록된 캐릭터라면 헤드샷 횟수를 증가시킵니다.
				}
				else
				{
					ShotgunResult.HeadShot.Emplace(BaseCharacter, 1); // 새로운 캐릭터라면 헤드샷 횟수를 1로 초기화합니다.
				}
			}
		}
	}
	// 헤드샷 확인 후 헤드는 비활성화, 나머지는 활성화
	for (auto& Frame : Packages)
	{
		for (auto& HitBoxPair : Frame.Character->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 박스 컴포넌트의 충돌을 일시적으로 활성화합니다.
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block); // 박스 컴포넌트가 트레이스 채널에 반응하도록 설정합니다.
			}
		}
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 헤드 박스의 충돌을 비활성화합니다.
	}
	// 몸샷 확인
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult; // 히트 결과를 저장할 변수
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f; // 트레이스 끝 위치 계산
		if (World)
		{
			World->LineTraceSingleByChannel(
				ConfirmHitResult, // 히트 결과를 저장할 변수
				TraceStart, // 트레이스 시작 위치
				TraceEnd, // 트레이스 끝 위치
				ECC_HitBox // 트레이스 채널
			);
			ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(ConfirmHitResult.GetActor()); // 히트된 액터를 캐릭터로 변환합니다.
			if (BaseCharacter)
			{
				if (ConfirmHitResult.Component.IsValid())
				{
					/*
					UBoxComponent* Box = Cast<UBoxComponent>(ConfirmHitResult.Component);
					if (Box)
					{
						DrawDebugBox(World, // 디버그 박스를 그릴 월드
							Box->GetComponentLocation(), // 박스의 위치
							Box->GetScaledBoxExtent(), // 박스의 크기
							FQuat(Box->GetComponentRotation()), // 박스의 회전
							FColor::Blue, // 박스 색상
							false, // 지속 시간 없음
							5.f // 5초 동안 표시
						); // 디버그 박스를 그립니다.
					}
					*/
				}

				if (ShotgunResult.BodyShot.Contains(BaseCharacter))
				{
					ShotgunResult.BodyShot[BaseCharacter]++; // 이미 헤드샷이 기록된 캐릭터라면 헤드샷 횟수를 증가시킵니다.
				}
				else
				{
					ShotgunResult.BodyShot.Emplace(BaseCharacter, 1); // 새로운 캐릭터라면 헤드샷 횟수를 1로 초기화합니다.
				}
			}
		}
	}

	for (auto& Frame : Packages)
	{
		ResetHitBoxes(Frame.Character, Frame); // 히트된 캐릭터의 박스 콜리전을 노 콜리전으로 되돌립니다.
		EnabledCharacterMeshCollision(Frame.Character, ECollisionEnabled::QueryAndPhysics); // 캐릭터 메쉬 충돌을 활성화합니다.
	}

	return ShotgunResult;
}

// 현재 위치의 박스 정보를 저장
void ULagCompensationComponent::CacheBoxPosition(ABaseCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr) return; // HitCharacter가 nullptr이면 함수 종료
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation(); // 박스 컴포넌트의 위치를 가져옵니다.
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation(); // 박스 컴포넌트의 회전을 가져옵니다.
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent(); // 박스 컴포넌트의 크기를 가져옵니다.
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo); // 박스 정보를 프레임 패키지에 추가합니다.
		}
	}
}

// 히트된 캐릭터의 박스 컴포넌트 위치가 있는 패키지로, 히트된 캐릭터의 박스 컴포넌트를 이동시키는 함수
void ULagCompensationComponent::MoveBoxes(ABaseCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return;
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			const FBoxInformation* BoxValue = Package.HitBoxInfo.Find(HitBoxPair.Key);
			if (BoxValue)
			{
				HitBoxPair.Value->SetWorldLocation(BoxValue->Location);
				HitBoxPair.Value->SetWorldRotation(BoxValue->Rotation);
				HitBoxPair.Value->SetBoxExtent(BoxValue->BoxExtent);
			}
		}
	}
}

// 히트 박스 콜리전의 콜리전을 다시 노 콜리전으로 초기화하는 함수
void ULagCompensationComponent::ResetHitBoxes(ABaseCharacter* HitCharacter, const FFramePackage& Package)
{
    if (HitCharacter == nullptr) return;
    for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
    {
        if (HitBoxPair.Value != nullptr)
        {
            const FBoxInformation* BoxValue = Package.HitBoxInfo.Find(HitBoxPair.Key);
            if (BoxValue)
            {
                HitBoxPair.Value->SetWorldLocation(BoxValue->Location);
                HitBoxPair.Value->SetWorldRotation(BoxValue->Rotation);
                HitBoxPair.Value->SetBoxExtent(BoxValue->BoxExtent);
                HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            }
        }
    }
}

// 캐릭터 메쉬( 이거는 히트박스 콜리전 아님 )의 충돌을 활성화/비활성화하는 함수
void ULagCompensationComponent::EnabledCharacterMeshCollision(ABaseCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled); // 캐릭터 메쉬의 충돌을 활성화/비활성화합니다.
	}
}

// 내 캐릭터 박스 프레임을 보여주는 디버그 함수
void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, FColor Color)
{
	for (auto& BoxInfo : Package.HitBoxInfo)
	{
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color, // 박스의 색상
			false, // 지속적으로 표시하지 않음
			4.f // 지속 시간 (초 단위)
		);
	}
}

