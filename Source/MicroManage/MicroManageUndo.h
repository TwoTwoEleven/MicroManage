#pragma once

#include "CoreMinimal.h"
#include "MicroManageSystem.h"
#include "MicroManageUndo.generated.h"

#define MAXUNDO 1000

USTRUCT()
struct MICROMANAGE_API FUndoTransformActor
{
	GENERATED_BODY()

public:
	UPROPERTY()
	AActor* Actor;

	UPROPERTY()
	FTransform Transform;

public:
	FORCEINLINE ~FUndoTransformActor() = default;
};

USTRUCT()
struct MICROMANAGE_API FUndoTransformComponent
{
	GENERATED_BODY()

public:
	UPROPERTY()
	USceneComponent* Component;

	UPROPERTY()
	FTransform Transform;

public:
	FORCEINLINE ~FUndoTransformComponent() = default;
};

USTRUCT()
struct MICROMANAGE_API FUndoColorSlot
{
	GENERATED_BODY()

public:
	UPROPERTY()
	AFGBuildable* Buildable;

	UPROPERTY()
	uint8 ColorSlot;

public:
	FORCEINLINE ~FUndoColorSlot() = default;
};

USTRUCT()
struct MICROMANAGE_API FUndoSelect
{
	GENERATED_BODY()

public:
	UPROPERTY()
	AActor* Actor;

	UPROPERTY()
	bool Select;

public:
	FORCEINLINE ~FUndoSelect() = default;
};

USTRUCT()
struct MICROMANAGE_API FUndoInfo
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TArray<FUndoTransformActor> TransformActors;

	UPROPERTY()
	TArray<FUndoTransformComponent> TransformComponents;

	UPROPERTY()
	TArray<FUndoColorSlot> ColorSlotItems;

	UPROPERTY()
	TArray<FUndoSelect> SelectItems;

	void Clear();

public:
	FORCEINLINE ~FUndoInfo() = default;
};

// UMicroManageUndo -------------------------------------------------------------------------------

UCLASS(BlueprintType)
class MICROMANAGE_API UMicroManageUndo : public UMicroManageComponent
{
	GENERATED_BODY()

private:
	FUndoInfo UndoInfoArr[MAXUNDO];
	int UndoHead = 0;
	int UndoCount = 0;

	void Push();
	void Pop();

public:
	UFUNCTION()
	void PushUndoTransforms(TArray<AActor*>& Actors);

	UFUNCTION()
	void PushUndoColorSlot(TArray<AActor*>& Actors);

	UFUNCTION()
	void PushUndoSelection(TArray<AActor*>& Actors);

	UFUNCTION()
	bool PopUndo(FUndoInfo& UndoInfo);

	UFUNCTION()
	void ClearUndoStack();

public:
	FORCEINLINE ~UMicroManageUndo() = default;
};