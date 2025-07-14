#pragma once

#include "CoreMinimal.h"
#include "PickUp.h"
#include "GrenadePickUp.generated.h"

UCLASS()
class MULTISHOOTER_API AGrenadePickUp : public APickUp
{
	GENERATED_BODY()

	int32 GrenadeAmount = 1; // È¹µæÇÒ ¼ö·ùÅº °³¼ö

protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult); // ¿À¹ö·¦ ½ÃÀÛ ÇÔ¼ö
	
};
