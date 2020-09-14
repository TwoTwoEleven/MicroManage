#include "MicroManageAction.h"
#include "MicroManageUndo.h"
#include "MicroManageSelection.h"
#include "MicroManageRCO.h"
#include "MicroManageTransform.h"
#include "MicroManageConfig.h"
#include "MicroManageUI.h"
#include "FGGameState.h"
#include "FGConnectionComponent.h"
#include "FGBuildableConveyorBase.h"
#include "FGFactoryConnectionComponent.h"
#include "FGBuildableRailroadTrack.h"
#include "FGRailroadTrackConnectionComponent.h"
#include "FGBuildablePipeBase.h"
#include "FGPipeConnectionComponent.h"
#include "FGBuildablePipeline.h"
#include "FGPipeNetwork.h"
#include "FGPipeSubsystem.h"

void UMicroManageAction::PerformUndo()
{
	FUndoInfo UndoInfo;
	if (System->Undo->PopUndo(UndoInfo)) {
		if (UndoInfo.SelectItems.Num() > 0) {
			int Idx = 0;
			for (const auto& UndoItem : UndoInfo.SelectItems) {
				if (Idx > 1) {
					System->Selection->SelectActor(UndoItem.Actor, UndoItem.Select);
				} else if (Idx == 1) {
					if (UndoItem.Actor != System->Selection->TargetActor) {
						System->Selection->SetTarget(UndoItem.Actor);
					}
				} else { // == 0
					if (UndoItem.Actor != System->Selection->AnchorActor) {
						System->Selection->SetAnchor(UndoItem.Actor);
					}
				}
				Idx++;
			}
		} else {
			System->MMRCO->ServerUndoAction(System->Manager, UndoInfo);
		}
		UndoInfo.Clear();
	}
}

void UMicroManageAction::PrepareTransform(const FVector& Loc, const FRotator& Rot, const FVector& Scale)
{
	// get actor(s) to perform transform on
	TArray <AActor*> Actors;
	System->Selection->GetSelectionOrLineTrace(Actors);
	if (Actors.Num() == 0) {
		return;
	}
	System->Undo->PushUndoTransforms(Actors);

	// Initialize TransformData with current settings
	FMicroManageTransformData TransformData;
	TransformData.Loc = Loc;
	TransformData.Rot = Rot;
	TransformData.Scale = Scale;
	if (System->Config->MMConfig.IsScaleLockedLR) {
		TransformData.Scale.Y = 1.0f;
	}
	if (System->Config->MMConfig.IsScaleLockedTB) {
		TransformData.Scale.Z = 1.0f;
	}
	if (System->Config->MMConfig.IsScaleLockedFB) {
		TransformData.Scale.X = 1.0f;
	}
	TransformData.Anchor = System->Selection->AnchorActor;
	TransformData.Target = System->Selection->TargetActor;
	TransformData.GroupMode = System->Config->MMConfig.IsGrouped;
	TransformData.ViewRelative = System->Config->MMConfig.IsViewBased;
	if (TransformData.ViewRelative) {
		TransformData.ViewVector = System->Manager->GetCameraViewVector();
	}
	if (TransformData.GroupMode) {
		TransformData.PivotLoc = System->Transform->CalculatePivotLoc(Actors, TransformData.Anchor, TransformData.Target);
	}
	System->Transform->CalculateTransformData(TransformData);

	// call server TransformItem to be broadcast to all clients
	System->MMRCO->ServerTransformActors(System->Manager, Actors, TransformData);
}

void UMicroManageAction::MoveSelectionToTarget(bool IgnoreTranslation)
{
	if (!System->Selection->AnchorActor || !System->Selection->TargetActor) {
		System->UI->ShowPopup(TITLE_REQUIRES_ANCHOR_AND_TARGET, BODY_REQUIRES_ANCHOR_AND_TARGET);
		return;
	}

	// get actor(s) to perform transform on
	TArray <AActor*> Actors;
	System->Selection->GetSelectionOrLineTrace(Actors);
	if (Actors.Num() == 0) {
		return;
	}
	System->Undo->PushUndoTransforms(Actors);

	// Initialize TransformData with current settings
	FMicroManageTransformData TransformData;
	TransformData.Loc = FVector::ZeroVector;
	TransformData.Rot = FRotator::ZeroRotator;
	TransformData.Scale = FVector::OneVector;
	TransformData.Anchor = System->Selection->AnchorActor;
	TransformData.Target = System->Selection->TargetActor;
	TransformData.GroupMode = System->Config->MMConfig.IsGrouped;
	TransformData.ViewRelative = false;

	float MoveDist = FVector::Dist2D(TransformData.Anchor->GetActorLocation(), TransformData.Target->GetActorLocation());
	if (!System->Config->MMConfig.WarningShownForLargeMoveLag && (Actors.Num() > 500) && !IgnoreTranslation && (MoveDist > 100.f)) {
		System->Config->MMConfig.WarningShownForLargeMoveLag = true;
		System->Config->SaveMicroManageConfig();
		System->UI->ShowPopup(TITLE_WARNING_LARGE_MOVE, BODY_WARNING_LARGE_MOVE);
	}

	if (TransformData.GroupMode) {
		// precalculate Pivot data
		TransformData.AnchorQuat = System->Selection->AnchorActor->GetActorQuat();
		TransformData.PivotLoc = System->Transform->CalculatePivotLoc(Actors, TransformData.Anchor, TransformData.Target);

		System->Transform->CalculateTargetPivot(TransformData);
	}
	TransformData.IsLoc = !IgnoreTranslation;
	TransformData.SetSame = true;

	// call server TransformItem to be broadcast to all clients
	System->MMRCO->ServerTransformActors(System->Manager, Actors, TransformData);
}

void UMicroManageAction::MakeActorsMovable(TArray<AActor*>& Actors)
{
	// process through all Actors removing any that are already movable prepared
	TArray<AActor*> ActorsToPrepare;
	for (const auto& Actor : Actors) {
		if (Actor->IsA<AFGBuildable>()) {
			for (const auto& SceneComp : Actor->GetRootComponent()->GetAttachChildren()) {
				if (SceneComp->Mobility != EComponentMobility::Movable) {
					ActorsToPrepare.Add(Actor);
					break;
				}
			}
		}
	}

	// prepare applicable actors on the server
	if (ActorsToPrepare.Num() > 0) {
		System->MMRCO->ServerPrepareActors(System->Manager, ActorsToPrepare);
	}
}

void UMicroManageAction::PerformMove(bool ConfirmClicked)
{
	if (ConfirmClicked) {
		MoveSelectionToTarget(false);
	}
}

void UMicroManageAction::PrepareMove()
{
	if (!System->Selection->AnchorActor || !System->Selection->TargetActor) {
		System->UI->ShowPopup(TITLE_REQUIRES_ANCHOR_AND_TARGET, BODY_REQUIRES_ANCHOR_AND_TARGET);
		return;
	}

	float MoveDist = FVector::Dist2D(System->Selection->AnchorActor->GetActorLocation(), 
		System->Selection->TargetActor->GetActorLocation());

	// Always show warning until everyone is used to it
	//System->Config->MMConfig.WarningShownForLargeMoveLag = true;

	if (System->Config->MMConfig.WarningShownForLargeMoveLag && (System->Selection->SelectCount() > 500) && (MoveDist > 100.f)) {
		//System->Config->MMConfig.WarningShownForLargeMoveLag = false;
		//System->Config->SaveMicroManageConfig();
		System->UI->ShowConfirm(TITLE_WARNING_LARGE_MOVE, BODY_WARNING_LARGE_MOVE, this, "PerformMove");
	} else {
		PerformMove(true);
	}
}

void UMicroManageAction::MakeActorMovable(AActor* Actor)
{
	TArray<AActor*> Actors;
	Actors.Add(Actor);
	MakeActorsMovable(Actors);
}

void UMicroManageAction::SelectActor(AActor* Actor, bool Select)
{
	if (System->Selection->SelectActor(Actor, Select) && Select) {
		MakeActorMovable(Actor);
	}
}

void UMicroManageAction::MakeConnection(AActor* OutputActor, AActor* InputActor, FString& Title, FString& Body)
{
	if (!OutputActor || !InputActor) {
		Title = TEXT(TITLE_REQUIRES_ANCHOR_AND_TARGET);
		Body = TEXT(BODY_REQUIRES_ANCHOR_AND_TARGET);
		return;
	}

	// factory connection lambda
	auto GetAvailableFactoryCxn = [](bool IsOutput, AActor* Actor) -> UFGFactoryConnectionComponent*
	{
		auto ValidFactoryConnectionDirection = [](bool IsOutput, UFGFactoryConnectionComponent* FactoryCxn) -> bool
		{
			EFactoryConnectionDirection FactoryConnectionDirection = FactoryCxn->GetDirection();
			return !FactoryCxn->IsConnected() && ((FactoryConnectionDirection == EFactoryConnectionDirection::FCD_ANY) ||
				(FactoryConnectionDirection == (IsOutput ? EFactoryConnectionDirection::FCD_OUTPUT : EFactoryConnectionDirection::FCD_INPUT)));
		};

		auto Conveyor = Cast<AFGBuildableConveyorBase>(Actor);
		if (Conveyor) {
			UFGFactoryConnectionComponent* ConveyorCxn = IsOutput ? Conveyor->GetConnection1() : Conveyor->GetConnection0();
			return !ConveyorCxn->IsConnected() ? ConveyorCxn : nullptr;
		} else {
			for (const auto& ActorComp : Actor->GetComponentsByClass(UFGFactoryConnectionComponent::StaticClass())) {
				auto FactoryCxn = Cast<UFGFactoryConnectionComponent>(ActorComp);
				if (ValidFactoryConnectionDirection(IsOutput, FactoryCxn)) {
					return FactoryCxn;
				}
			}
			return nullptr;
		}
	};

	// factory connection check
	UFGFactoryConnectionComponent* OutputFactoryCxn = GetAvailableFactoryCxn(true, OutputActor);
	if (OutputFactoryCxn) {
		UFGFactoryConnectionComponent* InputFactoryCxn = GetAvailableFactoryCxn(false, InputActor);
		if (InputFactoryCxn) {
			OutputFactoryCxn->SetConnection(InputFactoryCxn);
			Title = TEXT(TITLE_CONNECTION_MADE);
			Body = FString::Printf(TEXT(BODY_CONNECTION_MADE), TEXT("Conveyor"), *OutputFactoryCxn->GetName(), *InputFactoryCxn->GetName());
			return;
		}
	}

	// rail connection lambda
	auto GetAvailableRailCxn = [](AActor* Actor) -> UFGRailroadTrackConnectionComponent*
	{
		auto Rail = Cast<AFGBuildableRailroadTrack>(Actor);
		if (Rail) {
			UFGRailroadTrackConnectionComponent* RailCxn0 = Rail->GetConnection(0);
			UFGRailroadTrackConnectionComponent* RailCxn1 = Rail->GetConnection(1);
			return (RailCxn0->GetConnections().Num() <= RailCxn1->GetConnections().Num()) ? RailCxn0 : RailCxn1;
		}
		return nullptr;
	};

	// rail connection check
	UFGRailroadTrackConnectionComponent* OutputRailCxn = GetAvailableRailCxn(OutputActor);
	UFGRailroadTrackConnectionComponent* InputRailCxn = GetAvailableRailCxn(InputActor);
	if (OutputRailCxn || InputRailCxn) {
		if (OutputRailCxn && InputRailCxn) {
			OutputRailCxn->AddConnection(InputRailCxn);
			Title = TEXT(TITLE_CONNECTION_MADE);
			Body = FString::Printf(TEXT(BODY_CONNECTION_MADE), TEXT("Rail"), *OutputRailCxn->GetName(), *InputRailCxn->GetName());
			return;
		}
	}

	// pipe connection lambda
	auto GetAvailablePipeCxn = [](bool IsOutput, AActor* Actor) -> UFGPipeConnectionComponentBase*
	{
		auto ValidPipeConnectionType = [](bool IsOutput, UFGPipeConnectionComponentBase* PipeCxn) -> bool
		{
			EPipeConnectionType PipeConnectionType = PipeCxn->GetPipeConnectionType();
			return !PipeCxn->IsConnected() && ((PipeConnectionType == EPipeConnectionType::PCT_ANY) ||
				(PipeConnectionType == (IsOutput ? EPipeConnectionType::PCT_PRODUCER : EPipeConnectionType::PCT_CONSUMER)));
		};

		auto Pipe = Cast<AFGBuildablePipeBase>(Actor);
		if (Pipe) {
			if (ValidPipeConnectionType(IsOutput, Pipe->GetConnection0())) {
				return Pipe->GetConnection0();
			}
			if (ValidPipeConnectionType(IsOutput, Pipe->GetConnection1())) {
				return Pipe->GetConnection1();
			}
			return nullptr;
		} else {
			for (const auto& ActorComp : Actor->GetComponentsByClass(UFGPipeConnectionComponentBase::StaticClass())) {
				auto PipeCxn = Cast<UFGPipeConnectionComponentBase>(ActorComp);
				if (ValidPipeConnectionType(IsOutput, PipeCxn)) {
					return PipeCxn;
				}
			}
			return nullptr;
		}
	};

	// pipe connection check
	UFGPipeConnectionComponentBase* OutputPipeCxn = GetAvailablePipeCxn(true, OutputActor);
	if (OutputPipeCxn) {
		UFGPipeConnectionComponentBase* InputPipeCxn = GetAvailablePipeCxn(false, InputActor);
		if (InputPipeCxn && OutputPipeCxn->CheckCompatibility(InputPipeCxn, nullptr)) {
			OutputPipeCxn->SetConnection(InputPipeCxn);
			auto OutputFluidPipeCxn = Cast<UFGPipeConnectionComponent>(OutputPipeCxn);
			auto InputFluidPipeCxn = Cast<UFGPipeConnectionComponent>(InputPipeCxn);
			if (OutputFluidPipeCxn && InputFluidPipeCxn) {
				AFGPipeSubsystem* PipeSubsystem = AFGPipeSubsystem::Get(System->Manager->GetWorld());
				int32 OutputNetworkID = OutputFluidPipeCxn->GetPipeNetworkID();
				int32 InputNetworkID = InputFluidPipeCxn->GetPipeNetworkID();
				if (OutputNetworkID != InputNetworkID) { // merge and rebuild pipe networks
					AFGPipeNetwork* OutputNetwork = PipeSubsystem->FindPipeNetwork(OutputNetworkID);
					AFGPipeNetwork* InputNetwork = PipeSubsystem->FindPipeNetwork(InputNetworkID);
					if (OutputNetwork->IsValidLowLevel() && InputNetwork->IsValidLowLevel()) {
						OutputNetwork->MergeNetworks(InputNetwork);
						// reacquire networks after merge
						OutputNetwork = PipeSubsystem->FindPipeNetwork(OutputFluidPipeCxn->GetPipeNetworkID());
						InputNetwork = PipeSubsystem->FindPipeNetwork(InputFluidPipeCxn->GetPipeNetworkID());
					}
					if (OutputNetwork->IsValidLowLevel()) {
						OutputNetwork->MarkForFullRebuild();;
					} else if (InputNetwork->IsValidLowLevel()) {
						InputNetwork->MarkForFullRebuild();;
					}
				}
			}

			Title = TEXT(TITLE_CONNECTION_MADE);
			Body = FString::Printf(TEXT(BODY_CONNECTION_MADE), TEXT("Pipe"), *OutputPipeCxn->GetName(), *InputPipeCxn->GetName());
			return;
		}
	}

	Title = TEXT(TITLE_CONNECTION_NOT_FOUND);
	Body = TEXT(BODY_CONNECTION_NOT_FOUND);
}

void UMicroManageAction::BreakConnection(AActor* OutputActor, AActor* InputActor, FString& Title, FString& Body)
{
	if (!OutputActor || !InputActor) {
		Title = TEXT(TITLE_REQUIRES_ANCHOR_AND_TARGET);
		Body = TEXT(BODY_REQUIRES_ANCHOR_AND_TARGET);
		return;
	}

	int32 Disconnects = 0;
	// disconnect all shared connections between output and input
	for (const auto& ActorComp : OutputActor->GetComponentsByClass(UFGConnectionComponent::StaticClass())) {
		// factory connection check
		auto FactoryCxn = Cast<UFGFactoryConnectionComponent>(ActorComp);
		if (FactoryCxn) {
			if (FactoryCxn->IsConnected() && (FactoryCxn->GetConnection()->GetOuterBuildable() == InputActor)) {
				++Disconnects;
				FactoryCxn->ClearConnection();
			}
			continue;
		}
		// rail connection check
		auto OutputRailCxn = Cast<UFGRailroadTrackConnectionComponent>(ActorComp);
		if (OutputRailCxn) {
			for (const auto& InputRailCxn : OutputRailCxn->GetConnections()) {
				if (InputRailCxn->GetTrack() == InputActor) {
					++Disconnects;
					OutputRailCxn->RemoveConnection(InputRailCxn);
				}
			}
			continue;
		}
		// pipe connection check
		auto OutputPipeCxn = Cast<UFGPipeConnectionComponentBase>(ActorComp);
		if (OutputPipeCxn && OutputPipeCxn->IsConnected()) {
			for (const auto& InputPipeCxn : InputActor->GetComponentsByClass(UFGPipeConnectionComponentBase::StaticClass())) {
				if (OutputPipeCxn->GetConnection() == InputPipeCxn) {
					++Disconnects;
					OutputPipeCxn->ClearConnection();
					auto OutputFluidPipeCxn = Cast<UFGPipeConnectionComponent>(OutputPipeCxn);
					auto InputFluidPipeCxn = Cast<UFGPipeConnectionComponent>(InputPipeCxn);
					if (OutputFluidPipeCxn && InputFluidPipeCxn) {
						AFGPipeSubsystem* PipeSubsystem = AFGPipeSubsystem::Get(System->Manager->GetWorld());
						PipeSubsystem->FindPipeNetwork(OutputFluidPipeCxn->GetPipeNetworkID())->MarkForFullRebuild();;
						PipeSubsystem->FindPipeNetwork(InputFluidPipeCxn->GetPipeNetworkID())->MarkForFullRebuild();;
					}
					break;
				}
			}
		}
	}

	if (Disconnects == 0) {
		Title = TEXT(TITLE_CONNECTION_NOT_FOUND);
		Body = TEXT(BODY_CONNECTION_NOT_FOUND);
	} else {
		Title = TEXT(TITLE_DISCONNECTION_MADE);
		Body = FString::Printf(TEXT(BODY_DISCONNECTION_MADE), Disconnects, (Disconnects == 1) ? TEXT("") : TEXT("s"));
	}
}

void UMicroManageAction::RemoveIndicator()
{
	auto Pipe = Cast<AFGBuildablePipeline>(System->Selection->AnchorActor);
	if (Pipe) {
		Pipe->mFlowIndicatorMinimumPipeLength = 100000.f;
	}
}

void UMicroManageAction::SetSameScale()
{
	if (!System->Selection->TargetActor) {
		System->UI->ShowPopup(TITLE_REQUIRES_TARGET, BODY_REQUIRES_TARGET);
		return;
	}

	FVector Scale = System->Selection->TargetActor->GetActorScale();
	if (System->Config->MMConfig.IsGrouped) {
		if (System->Selection->AnchorActor) {
			Scale /= System->Selection->AnchorActor->GetActorScale();
		}
		System->Action->PrepareTransform(FVector(0.f), FRotator(0.f), FVector(Scale));
	} else {
		// get actors to act on
		TArray<AActor*> Actors;
		System->Selection->SelectedActorsNoTarget(Actors);
		if (Actors.Num() == 0) {
			return;
		}

		// save undo information
		System->Undo->PushUndoTransforms(Actors);

		// transform the scale of all actors individually
		for (const auto& Actor : Actors) {
			FTransform Transform = Actor->GetRootComponent()->GetComponentTransform();
			Transform.SetScale3D(Scale);
			System->Transform->TransformActor(Actor, Transform);
		}
	}
}

void UMicroManageAction::SetSamePaint()
{
	if (!System->Selection->TargetActor) {
		System->UI->ShowPopup(TITLE_REQUIRES_TARGET, BODY_REQUIRES_TARGET);
		return;
	}
	AFGBuildable* TargetBuildable = Cast<AFGBuildable>(System->Selection->TargetActor);
	if (!TargetBuildable) {
		return;
	}

	// get actors to act on
	TArray<AActor*> Actors;
	System->Selection->SelectedActorsNoTarget(Actors);
	if (Actors.Num() == 0) {
		return;
	}

	// save undo information
	System->Undo->PushUndoColorSlot(Actors);

	// paint all buildables on server to propogate out to clients
	System->MMRCO->ServerPaintActors(Actors, TargetBuildable->GetColorSlot_Implementation());
}
