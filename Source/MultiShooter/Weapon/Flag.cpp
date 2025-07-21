#include "Flag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "MultiShooter/Character/BaseCharacter.h"


AFlag::AFlag()
{
	FlagPole = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagPole"));
	SetRootComponent(FlagPole);
	FlagPole->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); // 플래그 폴은 충돌을 무시하도록 설정
	FlagPole->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 플래그 폴은 충돌을 비활성화

	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));
	FlagMesh->SetupAttachment(FlagPole); // 플래그 메쉬를 플래그 폴에 첨부
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); // 플래그 메쉬는 충돌을 무시하도록 설정
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 플래그 메쉬는 충돌을 비활성화

	GetCollisionSphere()->SetupAttachment(FlagPole); // 충돌 구를 플래그 메쉬에 첨부
	GetPickUpWidget()->SetupAttachment(FlagPole); // 위젯 컴포넌트를 플래그 메쉬에 첨부
}

void AFlag::BeginPlay()
{
	Super::BeginPlay();
	// 플래그 메쉬의 초기 위치를 저장합니다.
	InitialTransform = FlagMesh->GetComponentTransform();
}

void AFlag::OnEquipped()
{
	ShowPickUpWidget(false); // 무기가 장착되면 위젯을 숨깁니다.
	GetCollisionSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	FlagPole->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	FlagPole->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	// 플래그 존의 스피어 컴포넌트와 겹치도록 설정

	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

void AFlag::OnDropped()
{
	if (HasAuthority()) // 서버에서만 실행
	{
		GetCollisionSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // 충돌을 활성화합니다.
	}

	// 1. 현재 위치에서 아래로 라인 트레이스
	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0, 0, 1000.f); // 1000cm 아래로

	FHitResult HitResult;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		Start,
		End,
		ECollisionChannel::ECC_Visibility,
		Params
	);

	if (bHit)
	{
		// 2. 지면에 맞게 위치 이동 (Z축만 맞추기)
		FVector NewLocation = GetActorLocation();
		NewLocation.Z = HitResult.ImpactPoint.Z;
		SetActorLocationAndRotation(NewLocation, FRotator::ZeroRotator); // 회전은 월드 기준(0,0,0)
	}
	else
	{
		// 만약 트레이스 실패 시, 회전만 정렬
		SetActorRotation(FRotator::ZeroRotator);
	}

	FlagPole->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FlagPole->SetCollisionResponseToAllChannels(ECR_Block);
	FlagPole->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	FlagPole->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	FlagMesh->SetCollisionResponseToAllChannels(ECR_Block);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

void AFlag::Dropped()
{
	SetWeaponState(EWeaponState::EWS_Dropped); // 무기 상태를 떨어진 상태로 설정합니다.

	FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, true);
	FlagPole->DetachFromComponent(DetachmentRules); // 무기를 월드 트랜스폼을 유지하면서 분리합니다.
	SetOwner(nullptr); // 무기의 소유자를 nullptr로 설정합니다. (플레이어가 죽었으므로)
	BaseOwnerCharacter = nullptr; // 캐릭터를 nullptr로 설정합니다. (플레이어가 죽었으므로)
	BaseOwnerContoller = nullptr; // 플레이어 컨트롤러를 nullptr로 설정합니다. (플레이어가 죽었으므로)
}

// 플레이어가 깃발을 구역에 가져다 두면 처리되는 함수 = 깃발 캐릭터에서 디태치 및 원위치로
void AFlag::ResetFlag()
{
	ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
	if (Character)
	{
		Character->SetHoldingTheFlag(false); // 깃발을 들고 있지 않도록 설정
		Character->SetOverlappingWeapon(nullptr); // 겹치는 무기를 nullptr로 설정
		Character->GetCharacterMovement()->MaxWalkSpeed = 600;// 이속을 다시 원위치로
	}

	if (HasAuthority() == false) return; // 이후는 서버에서만 실행합니다.

	SetWeaponState(EWeaponState::EWS_Intital); // 무기 상태를 초기 상태로 설정합니다.
	GetCollisionSphere()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); // 쿼리 전용으로 설정합니다.
	GetCollisionSphere()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECR_Overlap); // Pawn 채널에 대해서는 충돌 응답을 겹침으로 설정합니다.


	FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, true);
	FlagPole->DetachFromComponent(DetachmentRules); // 무기를 월드 트랜스폼을 유지하면서 분리합니다.
	SetOwner(nullptr); // 무기의 소유자를 nullptr로 설정합니다. (플레이어가 죽었으므로)
	BaseOwnerCharacter = nullptr; // 캐릭터를 nullptr로 설정합니다. (플레이어가 죽었으므로)
	BaseOwnerContoller = nullptr; // 플레이어 컨트롤러를 nullptr로 설정합니다. (플레이어가 죽었으므로)

	SetActorTransform(InitialTransform); // 초기 위치로 되돌립니다.
}
