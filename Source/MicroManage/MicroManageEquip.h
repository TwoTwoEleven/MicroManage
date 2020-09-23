#pragma once

#include "CoreMinimal.h"
#include "Equipment/FGEquipment.h"
#include "FGCharacterPlayer.h"
#include "FGPlayerController.h"
#include "MicroManageTransform.h"
#include "MicroManageUndo.h"
#include "MicroManageEquip.generated.h"

class UMicroManageSystem;

UCLASS()
class MICROMANAGE_API AMicroManageEquip : public AFGEquipment
{
	GENERATED_BODY()
	
public:	
	AMicroManageEquip();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:
	UMicroManageSystem* System;

	bool ManagerEquipped = false;

	void SetupMicroManageSystem();

public:
	bool IsLocal = false;

	UFUNCTION(Reliable, NetMulticast, Category = "Micro Manage")
	void MulticastShowPopup(const FGuid& Id, const FString& Title, const FString& Body);

	UFUNCTION(Reliable, NetMulticast, Category = "Micro Manage")
	void MulticastTransformActors(const TArray<AActor*>& Actors, FMicroManageTransformData TransformData);

	UFUNCTION(Reliable, NetMulticast, Category = "Micro Manage")
	void MulticastUndoTransforms(const FUndoInfo& UndoInfo);

	UFUNCTION(Reliable, NetMulticast, Category = "Micro Manage")
	void MulticastRefreshMaterials(const TArray<AActor*>& Actors);

	UFUNCTION(Reliable, NetMulticast, Category = "Micro Manage")
	void MulticastUnEquip();

	virtual void Equip(AFGCharacterPlayer* Character) override;

	virtual void UnEquip() override;

public:
	FORCEINLINE ~AMicroManageEquip() = default;
};
