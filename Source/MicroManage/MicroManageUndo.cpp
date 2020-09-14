#include "MicroManageUndo.h"
#include "MicroManageSelection.h"

void FUndoInfo::Clear()
{
	ColorSlotItems.Empty();
	TransformActors.Empty();
	TransformComponents.Empty();
	SelectItems.Empty();
}

void UMicroManageUndo::ClearUndoStack()
{
	while (UndoCount > 0) {
		UndoInfoArr[UndoHead].Clear();
		Pop();
	}
}

void UMicroManageUndo::Pop()
{
	UndoCount--;
	UndoHead = (UndoHead + (MAXUNDO - 1)) % MAXUNDO;
}

bool UMicroManageUndo::PopUndo(FUndoInfo& UndoInfo)
{
	if (UndoCount > 0) {
		UndoInfo = UndoInfoArr[UndoHead];
		Pop();
		return true;
	}
	return false;
}

void UMicroManageUndo::Push()
{
	UndoCount = FMath::Min(UndoCount + 1, MAXUNDO);
	UndoHead = (UndoHead + 1) % MAXUNDO;
}

void UMicroManageUndo::PushUndoTransforms(TArray<AActor*>& Actors)
{
	Push();
	UndoInfoArr[UndoHead].Clear();
	UndoInfoArr[UndoHead].TransformActors.Reserve(Actors.Num());
	for (const auto& Actor : Actors) {
		// push all "root" components to TransformActors/Components
		for (const auto& ActorComp : Actor->GetComponentsByClass(USceneComponent::StaticClass())) {
			USceneComponent* SceneComp = Cast<USceneComponent>(ActorComp);
			if (SceneComp && !SceneComp->GetAttachParent()) {
				if (SceneComp == Actor->GetRootComponent()) {
					FUndoTransformActor UndoTransformActor;
					UndoTransformActor.Actor = Actor;
					UndoTransformActor.Transform = SceneComp->GetComponentTransform();
					UndoInfoArr[UndoHead].TransformActors.Add(UndoTransformActor);
				} else {
					FUndoTransformComponent UndoTransformComponent;
					UndoTransformComponent.Component = SceneComp;
					UndoTransformComponent.Transform = SceneComp->GetComponentTransform();
					UndoInfoArr[UndoHead].TransformComponents.Add(UndoTransformComponent);
				}
			}
		}
	}
}

void UMicroManageUndo::PushUndoColorSlot(TArray<AActor*>& Actors)
{
	Push();
	UndoInfoArr[UndoHead].Clear();
	UndoInfoArr[UndoHead].ColorSlotItems.Reserve(Actors.Num());
	for (const auto& Actor : Actors) {
		AFGBuildable* Buildable = Cast<AFGBuildable>(Actor);
		if (Buildable) {
			FUndoColorSlot UndoColorSlot;
			UndoColorSlot.Buildable = Buildable;
			UndoColorSlot.ColorSlot = Buildable->GetColorSlot_Implementation();
			UndoInfoArr[UndoHead].ColorSlotItems.Add(UndoColorSlot);
		}
	}
}

void UMicroManageUndo::PushUndoSelection(TArray<AActor*>& Actors)
{
	auto AddUndoSelect = [&](AActor* Actor)
	{
		FUndoSelect UndoSelect;
		UndoSelect.Actor = Actor;
		UndoSelect.Select = (Actor != nullptr) && System->Selection->Contains(Actor);
		UndoInfoArr[UndoHead].SelectItems.Add(UndoSelect);
	};

	Push();
	UndoInfoArr[UndoHead].Clear();
	UndoInfoArr[UndoHead].SelectItems.Reserve(Actors.Num() + 2);
	AddUndoSelect(System->Selection->AnchorActor);
	AddUndoSelect(System->Selection->TargetActor);
	for (const auto& Actor : Actors) {
		AddUndoSelect(Actor);
	}
}
