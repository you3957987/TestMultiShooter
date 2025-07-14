#include "LagCompensationComponent.h"
#include "MultiShooter/Character/BaseCharacter.h" // ABaseCharacter 클래스 사용을 위한 헤더 파일
#include "Components/BoxComponent.h"
#include "DrawDebugHelpers.h" // 디버그 박스 그리기를 위한 헤더 파일
#include "Components/BoxComponent.h" 

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

		ShowFramePackage(ThisFrame, FColor::Red); // 현재 프레임 패키지를 디버그 박스로 표시합니다.
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
// 프레임 패키지에다가 시간이랑 히트박스 정보를 저장하는 함수
void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	Character = Character == nullptr ? Cast<ABaseCharacter>(GetOwner()) : Character; // 캐릭터가 nullptr이면 GetOwner()로 캐릭터를 가져옵니다.
	if (Character)
	{
		Package.Time = GetWorld()->GetTimeSeconds(); // 현재 월드의 시간을 패키지에 저장합니다.

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
// 프레임 데이터를 이용한 시간 되감기 == 193 강의. 
// 시간을 되돌려서 타격 시간을 보강할 수 있게 하는거
// 만약 핑이 안좋아서 클라가 쏜 타격 정보를 서버가 받았을 떄 그 자리에 이미 캐릭터가 없다면 서버는 타격을 무시하므로
// 서버가 시간을 돌려서 히트된 캐릭터의 프레임을 찾아서 그 프레임에 맞춰서 히트된 캐릭터를 되돌리는 함수
FServerSideRewindResult ULagCompensationComponent::ServerSiedRewind(ABaseCharacter* HitCharacter, const FVector_NetQuantize& TraceStart,
	const FVector_NetQuantize& HitLocation, float HitTime)
{
	bool bReturn = 
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComponent() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail() == nullptr;
	if (bReturn) return FServerSideRewindResult(); 
	// HitCharacter가 nullptr이거나, LagCompensationComponent가 nullptr이거나, 프레임 히스토리가 비어있으면 함수 종료

	FFramePackage FrameToCheck; // 히트된 캐릭터의 프레임 패키지를 저장할 변수
	bool bShouldInterpolate = true; // 히트된 캐릭터의 프레임 패키지를 보간할지 여부를 저장할 변수

	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComponent()->FrameHistory; // 히트된 캐릭터의 프레임 히스토리를 가져옵니다.
	const float OldesthistoryTime = History.GetTail()->GetValue().Time; // 가장 오래된 프레임의 시간을 가져옵니다.
	const float NewestHistoryTime = History.GetHead()->GetValue().Time; // 가장 최근 프레임의 시간을 가져옵니다.
	if  ( HitTime < OldesthistoryTime)
	{
		// HitTime이 가장 오래된 프레임의 시간보다 이전이면 함수 종료 즉 너무 오랴되서 시간 되감기를 허지 않음
		return FServerSideRewindResult();
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

	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation); // 최종적으로 히트 확인!!!!!
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
FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package, ABaseCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	if(HitCharacter == nullptr) return FServerSideRewindResult();

	FFramePackage CurrentFrame;
	CacheBoxPosition(HitCharacter, CurrentFrame); // 현재 위치의 박스 정보를 저장
	MoveBoxes(HitCharacter, Package); // 히트된 캐릭터의 박스 컴포넌트를 패키지의 위치로 이동
	EnabledCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision); // 일단 캐릭터 메쉬 충돌 비활성화. 충돌은 오로지 히트 박스 콜리전에서만

	// 헤드샷 확인 위헤 일단 헤드부터 콜리전 활성화
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 헤드 박스의 충돌을 활성화합니다.
	HeadBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); // 헤드 박스가 트레이스 채널에 반응하도록 설정합니다.

	FHitResult ConfirmHitResult; // 히트 결과를 저장할 변수
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f; // 트레이스 끝 위치 계산
	UWorld* World = GetWorld(); // 현재 월드를 가져옵니다.
	if (World)
	{
		World->LineTraceSingleByChannel(
			ConfirmHitResult, // 히트 결과를 저장할 변수
			TraceStart, // 트레이스 시작 위치
			TraceEnd, // 트레이스 끝 위치
			ECC_Visibility // 트레이스 채널
		);
		if (ConfirmHitResult.bBlockingHit) // 헤드샷 발동!!!!
		{
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
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); // 박스 컴포넌트가 트레이스 채널에 반응하도록 설정합니다.
				}
			}
			World->LineTraceSingleByChannel(
				ConfirmHitResult, // 히트 결과를 저장할 변수
				TraceStart, // 트레이스 시작 위치
				TraceEnd, // 트레이스 끝 위치
				ECC_Visibility // 트레이스 채널
			);
			if (ConfirmHitResult.bBlockingHit) // 몸샷 발동!!!!
			{
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
	if (HitCharacter == nullptr) return; // HitCharacter가 nullptr이면 함수 종료
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location); // 박스 컴포넌트의 위치를 패키지의 위치로 설정합니다.
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation); // 박스 컴포넌트의 회전을 패키지의 회전으로 설정합니다.
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent); // 박스 컴포넌트의 크기를 패키지의 크기로 설정합니다.
		}
	}
}
// 히트 박스 콜리전의 콜리전을 다시 노 콜리전으로 초기화하는 함수
void ULagCompensationComponent::ResetHitBoxes(ABaseCharacter* HitCharacter, const FFramePackage& Package)
{
	if (HitCharacter == nullptr) return; // HitCharacter가 nullptr이면 함수 종료
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (HitBoxPair.Value != nullptr)
		{
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location); // 박스 컴포넌트의 위치를 패키지의 위치로 설정합니다.
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation); // 박스 컴포넌트의 회전을 패키지의 회전으로 설정합니다.
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent); // 박스 컴포넌트의 크기를 패키지의 크기로 설정합니다.
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 박스 컴포넌트의 충돌을 비활성화합니다.
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


