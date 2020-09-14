#pragma once

#include "CoreMinimal.h"
#include "MicroManageSystem.h"
#include "MicroManageAction.generated.h"

UCLASS(BlueprintType)
class MICROMANAGE_API UMicroManageAction : public UMicroManageComponent
{
	GENERATED_BODY()

public:

	void PerformUndo();

	void PrepareTransform(const FVector& Loc, const FRotator& Rot, const FVector& Scale);

	void MoveSelectionToTarget(bool IgnoreTranslation = false);

	UFUNCTION()
	void PerformMove(bool ConfirmClicked);

	void PrepareMove();

	void MakeActorsMovable(TArray<AActor*>& Actors);

	void MakeActorMovable(AActor* Actor);

	void MakeConnection(AActor* OutputActor, AActor* InputActor, FString& Title, FString& Body);

	void BreakConnection(AActor* OutputActor, AActor* InputActor, FString& Title, FString& Body);

	void RemoveIndicator();

	void SelectActor(AActor* Actor, bool Select);

	void SetSameScale();

	void SetSamePaint();

public:
	FORCEINLINE ~UMicroManageAction() = default;
};
