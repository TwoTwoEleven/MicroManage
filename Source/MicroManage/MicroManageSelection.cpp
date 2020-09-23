#include "MicroManageSelection.h"
#include "MicroManageConfig.h"
#include "MicroManageAction.h"
#include "MicroManageTransform.h"
#include "MicroManageUndo.h"
#include "MicroManageUI.h"
#include "MicroManageEquip.h"
#include "ProxyInstancedStaticMeshComponent.h"
#include "FGColoredInstanceMeshProxy.h"
#include "FGProductionIndicatorInstanceComponent.h"
#include "FGVehicle.h"
#include "Buildables/FGBuildableWire.h"
#include "FGTargetPoint.h"
#include "FGFactorySettings.h"

void UMicroManageSelection::Init()
{
	Super::Init();
	SelectedMap.Empty();
	AnchorActor = nullptr;
	TargetActor = nullptr;
	MaterialsInitialized = false;
	SelectedMaterials.Empty();
	SelectedMaterials.Insert(UFGFactorySettings::Get()->mDefaultValidPlacementMaterialSimplified, 0);
	SelectedMaterials.Insert(UFGFactorySettings::Get()->mDefaultValidPlacementMaterial, 0);
}

void UMicroManageSelection::SetSelectedMaterial(TArray<UMaterialInterface*> Materials)
{
	// ignore .pak materials for now
}

void UMicroManageSelection::SelectNextMaterial()
{
	System->Config->MMConfig.CurrentSelectedMaterial = (System->Config->MMConfig.CurrentSelectedMaterial + 1) %	SelectedMaterials.Num();
	MaterialsInitialized = false;
	InitializeMaterials();
}

void UMicroManageSelection::InitializeMaterials()
{
	if (MaterialsInitialized) {
		return;
	}

	SelectedMaterial = UMaterialInstanceDynamic::Create(SelectedMaterials[System->Config->MMConfig.CurrentSelectedMaterial], this);
	AnchorMaterial = UMaterialInstanceDynamic::Create(SelectedMaterials[System->Config->MMConfig.CurrentSelectedMaterial], this);
	TargetMaterial = UMaterialInstanceDynamic::Create(SelectedMaterials[System->Config->MMConfig.CurrentSelectedMaterial], this);

	switch (System->Config->MMConfig.CurrentSelectedMaterial) {
		case 0: // Opaque game material
			SelectedMaterial->SetScalarParameterValue(TEXT("MaxOpacity"), 0.1);
			SelectedMaterial->SetScalarParameterValue(TEXT("MinOpacity"), 0.1);

			AnchorMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor::Green);
			AnchorMaterial->SetVectorParameterValue(TEXT("HoloColor"), FLinearColor(0.0, 0.1, 0.0)); // green tint
			AnchorMaterial->SetScalarParameterValue(TEXT("MaxOpacity"), 0.1);
			AnchorMaterial->SetScalarParameterValue(TEXT("MinOpacity"), 0.1);

			TargetMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor::Red);
			TargetMaterial->SetVectorParameterValue(TEXT("HoloColor"), FLinearColor(0.5, 0.0, 0.0)); // red tint
			TargetMaterial->SetScalarParameterValue(TEXT("MaxOpacity"), 0.1);
			TargetMaterial->SetScalarParameterValue(TEXT("MinOpacity"), 0.1);
			break;
		case 1: // Transparent game material
			SelectedMaterial->SetScalarParameterValue(TEXT("Darken"), 0.1);
			SelectedMaterial->SetScalarParameterValue(TEXT("MaxOpacity"), 0.5);
			SelectedMaterial->SetScalarParameterValue(TEXT("MinOpacity"), 0.5);
			SelectedMaterial->SetScalarParameterValue(TEXT("FringeDisplacement_Glitch"), 0.0);

			AnchorMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.0, 0.5, 0.0));
			AnchorMaterial->SetVectorParameterValue(TEXT("HoloColor"), FLinearColor(0.0, 0.5, 0.0)); // green tint
			AnchorMaterial->SetScalarParameterValue(TEXT("Darken"), 0.3);
			AnchorMaterial->SetScalarParameterValue(TEXT("MaxOpacity"), 0.5);
			AnchorMaterial->SetScalarParameterValue(TEXT("MinOpacity"), 0.5);
			AnchorMaterial->SetScalarParameterValue(TEXT("FringeDisplacement_Glitch"), 0.0);

			TargetMaterial->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.5, 0.0, 0.0));
			TargetMaterial->SetVectorParameterValue(TEXT("HoloColor"), FLinearColor(0.5, 0.0, 0.0)); // red tint
			TargetMaterial->SetScalarParameterValue(TEXT("Darken"), 0.3);
			TargetMaterial->SetScalarParameterValue(TEXT("MaxOpacity"), 0.5);
			TargetMaterial->SetScalarParameterValue(TEXT("MinOpacity"), 0.5);
			TargetMaterial->SetScalarParameterValue(TEXT("FringeDisplacement_Glitch"), 0.0);
			break;
	}
	MaterialsInitialized = true;

	for (const auto& Elem : SelectedMap) {
		ResetHologram(Elem.Key);
	}
}

void UMicroManageSelection::ShowHologram(AActor* Actor, FSelectedActorInfo& ActorInfo)
{
	InitializeMaterials();
	ActorInfo.SavedMaterialInterfaces.Reset();

	// process through all UMeshComponents for this actor
	for (const auto& ActorComp : Actor->GetComponentsByClass(UMeshComponent::StaticClass())) {
		UMeshComponent* MeshComp = Cast<UMeshComponent>(ActorComp);

		// store materials for this mesh into the cache for this actor
		FSavedMaterialInterfaces SavedMaterialInterface = FSavedMaterialInterfaces();
		SavedMaterialInterface.MaterialInterfaces = MeshComp->GetMaterials();
		ActorInfo.SavedMaterialInterfaces.Add(MeshComp, SavedMaterialInterface);

		// replace materials for this mesh with hologram
		int Mats = MeshComp->GetNumMaterials();
		if (Actor == AnchorActor) {
			for (int i = 0; i < Mats; i++) {
				MeshComp->SetMaterial(i, AnchorMaterial);
			}
		} else if (Actor == TargetActor) {
			for (int i = 0; i < Mats; i++) {
				MeshComp->SetMaterial(i, TargetMaterial);
			}
		} else {
			for (int i = 0; i < Mats; i++) {
				MeshComp->SetMaterial(i, SelectedMaterial);
			}
		}

		// turn off instancing if it's a special mesh
		auto StaticMeshProxy = Cast<UProxyInstancedStaticMeshComponent>(MeshComp);
		if (StaticMeshProxy) {
			StaticMeshProxy->SetInstanced(false);
		} else {
			auto ColoredMeshProxy = Cast<UFGColoredInstanceMeshProxy>(MeshComp);
			if (ColoredMeshProxy) {
				ColoredMeshProxy->SetInstanced(false);
			} else {
				auto ProdIndInst = Cast<UFGProductionIndicatorInstanceComponent>(MeshComp);
				if (ProdIndInst) {
					ProdIndInst->SetInstanced(false);
				}
			}
		}
	}
}

void UMicroManageSelection::HideHologram(AActor* Actor, FSelectedActorInfo& ActorInfo)
{
	// process through all UMeshComponents for this actor
	for (auto& ActorComp : Actor->GetComponentsByClass(UMeshComponent::StaticClass())) {
		auto MeshComp = Cast<UMeshComponent>(ActorComp);
		// set back to instanced if it's a special mesh type
		auto StaticMeshProxy = Cast<UProxyInstancedStaticMeshComponent>(MeshComp);
		if (StaticMeshProxy) {
			StaticMeshProxy->SetInstanced(true);
		} else {
			auto ColoredMeshProxy = Cast<UFGColoredInstanceMeshProxy>(MeshComp);
			if (ColoredMeshProxy) {
				ColoredMeshProxy->SetInstanced(true);
			} else {
				auto ProdIndInst = Cast<UFGProductionIndicatorInstanceComponent>(MeshComp);
				if (ProdIndInst) {
					ProdIndInst->SetInstanced(true);
				}
			}
		}

		// retrieve materials for this mesh from the cache for this actor
		auto SavedMaterialInterfacePtr = ActorInfo.SavedMaterialInterfaces.Find(MeshComp);
		if (SavedMaterialInterfacePtr) {
			auto SavedMaterialInterface = *SavedMaterialInterfacePtr;
			
			// restore all materials on this mesh
			int Mats = SavedMaterialInterface.MaterialInterfaces.Num();
			for (int i = 0; i < Mats; i++) {
				MeshComp->SetMaterial(i, SavedMaterialInterface.MaterialInterfaces[i]);
			}
		}
	}

	// reset color slot for this actor (e.g. for buggy ramps)
	auto Buildable = Cast<AFGBuildable>(Actor);
	if (Buildable) {
		Buildable->SetColorSlot_Implementation(Buildable->GetColorSlot_Implementation());
	}
}

void UMicroManageSelection::ResetHologram(AActor* Actor)
{
	auto ActorInfoPtr = GetActorInfo(Actor);
	HideHologram(Actor, *ActorInfoPtr);
	auto ActorInfo = FSelectedActorInfo();
	ShowHologram(Actor, ActorInfo);
	ActorInfoPtr->SavedMaterialInterfaces = ActorInfo.SavedMaterialInterfaces;
}

void UMicroManageSelection::RefreshMaterial(AActor* Actor)
{
	// hack to refresh static mesh - This is needed for power indicators and meshes to update properly.
	if (!Contains(Actor)) {
		auto OutlineComp = UFGOutlineComponent::Get(Actor->GetWorld());
		OutlineComp->ShowDismantlePendingMaterial(Actor);
		OutlineComp->HideAllDismantlePendingMaterial();
		// reset color slot for this actor (e.g. for buggy ramps)
		auto Buildable = Cast<AFGBuildable>(Actor);
		if (Buildable) {
			Buildable->SetColorSlot_Implementation(Buildable->GetColorSlot_Implementation());
		}
	}
}

void UMicroManageSelection::RefreshMaterials(const TArray<AActor*>& Actors)
{
	for (auto& Actor : Actors) {
		RefreshMaterial(Actor);
	}
}

FSelectedActorInfo* UMicroManageSelection::GetActorInfo(AActor* Actor)
{
	return SelectedMap.Find(Actor);
}

bool UMicroManageSelection::IsValidActor(AActor* Actor)
{
	if ((Actor != nullptr) && Actor->IsValidLowLevel()) {
		if (Actor->IsA<AFGBuildable>() || Actor->IsA<AFGVehicle>() || Actor->IsA<AFGTargetPoint>()) {
			return true;
		}
	}
	return false;
}

bool UMicroManageSelection::SetMarker(AActor* Actor, AActor** Marker1, AActor** Marker2)
{
	if (!IsValidActor(Actor)) {
		return false;
	}
	bool ClearMarker = (Actor == nullptr) || (Actor == *Marker1);
	if (*Marker1) { // clear Marker1
		auto TempActor = *Marker1;
		*Marker1 = nullptr;
		ResetHologram(TempActor);
	}
	if (ClearMarker) {
		return false;
	}
	if (Actor == *Marker2) { // clear Marker2
		auto TempActor = *Marker2;
		*Marker2 = nullptr;
		ResetHologram(TempActor);
	}
	// set Marker1
	*Marker1 = Actor;
	if (Contains(Actor)) {
		ResetHologram(Actor);
	} else {
		SelectActor(Actor, true);
	}
	return true;
}

bool UMicroManageSelection::SetAnchor(AActor* Actor)
{
	return SetMarker(Actor, &AnchorActor, &TargetActor);
}

bool UMicroManageSelection::SetTarget(AActor* Actor)
{
	return SetMarker(Actor, &TargetActor, &AnchorActor);
}

bool UMicroManageSelection::SelectActor(AActor* Actor, bool Select, bool DeleteFromMap)
{
	if (Select) {
		if (IsValidActor(Actor) && !SelectedMap.Find(Actor)) {
			FSelectedActorInfo ActorInfo = FSelectedActorInfo();
			ShowHologram(Actor, ActorInfo);
			SelectedMap.Add(Actor, ActorInfo);
			return true;
		}
	} else {
		FSelectedActorInfo* ActorInfoPtr = SelectedMap.Find(Actor);
		if (ActorInfoPtr) {
			FSelectedActorInfo ActorInfo = *ActorInfoPtr;
			HideHologram(Actor, ActorInfo);
			if (DeleteFromMap) {
				SelectedMap.Remove(Actor);
			}
			if (Actor == AnchorActor) {
				AnchorActor = nullptr;
			}
			if (Actor == TargetActor) {
				TargetActor = nullptr;
			}
			return true;
		}
	}
	return false;
}

bool UMicroManageSelection::Contains(AActor* Actor)
{
	return SelectedMap.Contains(Actor);
}

int UMicroManageSelection::SelectCount()
{
	return (TargetActor ? SelectedMap.Num() - 1 : SelectedMap.Num());
}

void UMicroManageSelection::SelectedActors(TArray<AActor*>& Actors)
{
	SelectedMap.GenerateKeyArray(Actors);
}

void UMicroManageSelection::SelectedActorsNoTarget(TArray<AActor*>& Actors)
{
	SelectedActors(Actors);
	if (TargetActor) {
		Actors.RemoveSingleSwap(TargetActor);
	}
}

void UMicroManageSelection::GetSelectionOrLineTrace(TArray<AActor*>& Actors)
{
	Actors.Empty();
	if (SelectCount() == 0) {
		AActor* Actor = LineTraceFromPlayer();
		if (IsValidActor(Actor)) {
			Actors.Add(Actor);
		}
		System->Action->MakeActorsMovable(Actors);
	} else {
		SelectedActorsNoTarget(Actors);
	}
}

// Selects buildables that are inside the cube formed by Anchor and Target. Calculates 6 planes
// that define the cube with an additional delta component (PlaneDelta) for the distance outside
// the cube that is still considered valid.  The planes of the cube are oriented so that positive
// distance calculations are facing the center of the cube.

// Delta: For pivot location mode, this delta will just be a small
// value (default 0.5m) to prevent slight variations from excluding desired objects.  For sides mode,
// the delta will be the 0.5m plus the additional distance from the origin point of the buildable and
// the side.

// CenterOnPlane is a boolean calculated for each plane that will be true if the center point between
// Target and Anchor is within +/- delta.  When CenterOnPlane is true (usually for walls or buildables
// along the same axis), a buildable must be within delta distance from the plane to be added.
// When CenterOnPlane is false, the buildable must be on the positive distance side of all planes
// (within delta) to be added.

void UMicroManageSelection::AddAnchorTargetBoxToSelection(bool UseSides)
{
	struct FCubeSide
	{
		FPlane Plane;
		float Delta;
		bool CenterOnPlane;
	public:
		FCubeSide() {}
		FCubeSide(const FVector& CubeFarLoc, const FVector& CornerLoc, const FVector& CornerNormal, const float InDelta)
			: Plane(FPlane(CornerLoc, CornerNormal))
			, Delta(InDelta)
		{
			const float Dist = Plane.PlaneDot(CubeFarLoc);
			if (Dist < 0) {
				Plane *= -1.f; // flip the plane
			}
			CenterOnPlane = FMath::Abs(Dist) < Delta;
		}
	};

	if (!AnchorActor || !TargetActor) {
		System->UI->ShowPopup(TITLE_REQUIRES_ANCHOR_AND_TARGET, BODY_REQUIRES_ANCHOR_AND_TARGET);
		return;
	}

	// initialize Anchor, Target, Center locations and Anchor, Target deltas
	FVector AnchorLoc;
	FVector TargetLoc;
	FVector AnchorDelta;
	FVector TargetDelta;
	System->Transform->GetActorOriginAndSize(AnchorActor, AnchorLoc, AnchorDelta);
	System->Transform->GetActorOriginAndSize(TargetActor, TargetLoc, TargetDelta);
	if (!UseSides) {
		AnchorDelta = FVector::ZeroVector;
		TargetDelta = FVector::ZeroVector;
	}
	AnchorDelta = (AnchorDelta / 2.f) + System->Config->MMConfig.SelectionTolerance;
	TargetDelta = (TargetDelta / 2.f) + System->Config->MMConfig.SelectionTolerance;
	FVector CenterLoc = (AnchorLoc + TargetLoc) / 2.f;

	// initialize cube sides
	TArray<FCubeSide> CubeSides;
	FQuat Quat = AnchorActor->GetActorQuat();
	CubeSides.Add(FCubeSide(TargetLoc, AnchorLoc, Quat.GetAxisX(), AnchorDelta.X));
	CubeSides.Add(FCubeSide(TargetLoc, AnchorLoc, Quat.GetAxisY(), AnchorDelta.Y));
	CubeSides.Add(FCubeSide(TargetLoc, AnchorLoc, Quat.GetAxisZ(), AnchorDelta.Z));
	Quat = TargetActor->GetActorQuat();
	CubeSides.Add(FCubeSide(AnchorLoc, TargetLoc, Quat.GetAxisX(), TargetDelta.X));
	CubeSides.Add(FCubeSide(AnchorLoc, TargetLoc, Quat.GetAxisY(), TargetDelta.Y));
	CubeSides.Add(FCubeSide(AnchorLoc, TargetLoc, Quat.GetAxisZ(), TargetDelta.Z));

	// process through all actors looking for any that are within defined cube
	FVector WorkerLoc;
	FVector BoxExtent;
	TArray<AActor*> AddedActors;
	for (TObjectIterator<AFGBuildable> Worker; Worker; ++Worker) {
		if (Contains(*Worker) || Worker->IsA<AFGBuildableWire>()) {
			continue;
		}
		Worker->GetActorBounds(false, WorkerLoc, BoxExtent);
		bool InsideCube = true;
		for (auto CubeSide : CubeSides) {
			float Distance = CubeSide.Plane.PlaneDot(WorkerLoc);
			if ((Distance < -CubeSide.Delta) || (CubeSide.CenterOnPlane && (Distance > CubeSide.Delta))) {
				InsideCube = false;
				break;
			}
		}
		if (InsideCube) {
			AddedActors.Add(*Worker);
		}
	}

	// push selection undo (client), select all added actors (client) and then prepare those actors to be movable (server)
	System->Undo->PushUndoSelection(AddedActors);
	for (const auto& Actor : AddedActors) {
		SelectActor(Actor);
	}
	System->Action->MakeActorsMovable(AddedActors);

	// toggle (deselect) AnchorActor and TargetActor
	SetAnchor(AnchorActor);
	SetTarget(TargetActor);
}

void UMicroManageSelection::SelectClear(bool ConfirmClicked)
{
	if (ConfirmClicked) {
		AnchorActor = nullptr;
		TargetActor = nullptr;
		for (auto& Elem : SelectedMap) {
			SelectActor(Elem.Key, false, false);
		}
		SelectedMap.Empty();
	}
}

void UMicroManageSelection::SaveSelection()
{
	SelectedActorsNoTarget(SavedSelection);
	SavedAnchor = AnchorActor;
	SavedTarget = TargetActor;
}

void UMicroManageSelection::LoadSelection()
{
	SelectClear();
	for (auto& Actor : SavedSelection) {
		SelectActor(Actor);
	}
	SetAnchor(SavedAnchor);
	SetTarget(SavedTarget);
}

AActor* UMicroManageSelection::LineTraceFromPlayer()
{
	FVector Start = System->GetLocalController()->PlayerCameraManager->GetCameraLocation();
	FVector End = Start + (System->GetLocalController()->PlayerCameraManager->GetActorForwardVector() *
		(System->Config->MMConfig.MaxTargetRangeMeters * 100.0));
	FHitResult HitResult;
	FCollisionQueryParams TraceParams(TEXT("MMTrace"), false, System->GetLocalController()->GetPawn());
	if (System->GetWorld()->
		LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, TraceParams)) {
		return HitResult.GetActor();
	}
	return nullptr;
}


