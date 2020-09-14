#pragma once

#include "CoreMinimal.h"
#include "FGRemoteCallObject.h"
#include "MicroManageEquip.h"
#include "MicroManageUndo.h"
#include "MicroManageRCO.generated.h"

UCLASS()
class MICROMANAGE_API UMicroManageRCO : public UFGRemoteCallObject
{
	GENERATED_BODY()

private:
	void RemoveDuplicateWires(TArray<class AFGBuildableWire*>& Wires);

	void ProcessWires(const TArray<class AFGBuildableWire*>& Wires);

public:
	UPROPERTY(Replicated)
	bool Dummy = true;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Reliable, Server, WithValidation, Category = "MicroManage")
	void ServerTransformActors(AMicroManageEquip* Equip, const TArray<AActor*>& Actors, FMicroManageTransformData TransformData);

	UFUNCTION(Reliable, Server, WithValidation, Category = "MicroManage")
	void ServerUndoAction(AMicroManageEquip* Equip, const FUndoInfo& UndoInfo);

	UFUNCTION(Reliable, Server, WithValidation, Category = "MicroManage")
	void ServerPrepareActors(AMicroManageEquip* Equip, const TArray<AActor*>& Actors);

	UFUNCTION(Reliable, Server, WithValidation, Category = "MicroManage")
	void ServerPaintActors(const TArray<AActor*>& Actors, uint8 ColorSlot);

	UFUNCTION(Reliable, Server, WithValidation, Category = "MicroManage")
	void ServerHandleConnect(AMicroManageEquip* Equip, bool IsConnection, AActor* OutputActor, AActor* InputActor);

public:
	FORCEINLINE ~UMicroManageRCO() = default;
};

