#include "MicroManageRCO.h"
#include "MicroManageSystem.h"
#include "MicroManageAction.h"
#include "SML/util/Logging.h"
#include "Net/UnrealNetwork.h"
#include "FGBuildableWire.h"
#include "FGFactoryConnectionComponent.h"
#include "FGCircuitConnectionComponent.h"

void UMicroManageRCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMicroManageRCO, Dummy);
}

void UMicroManageRCO::RemoveDuplicateWires(TArray<AFGBuildableWire*>& Wires)
{
	Algo::Sort(Wires);
	int Idx = Wires.Num() - 1;
	while (Idx > 0) {
		if (Wires[Idx] == Wires[Idx - 1]) {
			Wires.RemoveAtSwap(Idx);
		}
		Idx--;
	}
}

void UMicroManageRCO::ProcessWires(const TArray<AFGBuildableWire*>& Wires)
{
	// update all wires endpoints connecting to any circuit connections
	for (const auto& Wire : Wires) {
		if ((Wire != nullptr) && !Wire->IsValidLowLevel()) continue;
		UFGCircuitConnectionComponent* Cxn0 = Wire->GetConnection(0);
		UFGCircuitConnectionComponent* Cxn1 = Wire->GetConnection(1);

		// create new wire based on old wire
		FActorSpawnParameters Params;
		Params.Instigator = Wire->Instigator;
		Params.Owner = Wire->GetOwner();
		const auto& NewWire = Wire->GetWorld()->SpawnActor<AFGBuildableWire>(Wire->GetClass(), Wire->GetActorTransform(), Params);
		NewWire->SetBuiltWithRecipe(Wire->GetBuiltWithRecipe());

		// remove old wire
		Wire->Disconnect();
		Wire->GetWorld()->DestroyActor(Wire);

		// connect up new wire
		NewWire->Connect(Cxn0, Cxn1);
		Cxn0->AddConnection(NewWire);
		Cxn1->AddConnection(NewWire);
		NewWire->UpdateWireMesh();
	}
}

void UMicroManageRCO::ServerTransformActors_Implementation(AMicroManageEquip* Equip, const TArray<AActor*>& Actors, FMicroManageTransformData TransformData)
{
	// get all connected wires
	TArray<AFGBuildableWire*> Wires;
	for (const auto& Actor : Actors) {
		for (const auto& CircuitCxnComp : Actor->GetComponentsByClass(UFGCircuitConnectionComponent::StaticClass())) {
			Cast<UFGCircuitConnectionComponent>(CircuitCxnComp)->GetWires(Wires);
		}
	}
	RemoveDuplicateWires(Wires);

	// process the transform across all actors
	Equip->MulticastTransformActors(Actors, TransformData);

	// update the wire positions attached to all the actors
	ProcessWires(Wires);
}

bool UMicroManageRCO::ServerTransformActors_Validate(AMicroManageEquip* Equip, const TArray<AActor*>& Actors, FMicroManageTransformData TransformData)
{
	return true;
}

void UMicroManageRCO::ServerUndoAction_Implementation(AMicroManageEquip* Equip, const FUndoInfo& UndoInfo)
{
	if (UndoInfo.ColorSlotItems.Num() > 0) {
		for (const auto& UndoItem : UndoInfo.ColorSlotItems) {
			UndoItem.Buildable->SetColorSlot_Implementation(UndoItem.ColorSlot);
		}
	} else if (UndoInfo.TransformActors.Num() > 0) {
		// get all connected wires while on server
		TArray<AFGBuildableWire*> Wires;
		for (const auto& UndoActor : UndoInfo.TransformActors) {
			for (auto CircuitCxnComp : UndoActor.Actor->GetComponentsByClass(UFGCircuitConnectionComponent::StaticClass())) {
				Cast<UFGCircuitConnectionComponent>(CircuitCxnComp)->GetWires(Wires);
			}
		}
		RemoveDuplicateWires(Wires);

		// process the reverse transform across all undo items
		Equip->MulticastUndoTransforms(UndoInfo);

		// update the wire positions attached to all the undo items while on server
		ProcessWires(Wires);
	}
}

bool UMicroManageRCO::ServerUndoAction_Validate(AMicroManageEquip* Equip, const FUndoInfo& UndoInfo)
{
	return true;
}

void UMicroManageRCO::ServerPrepareActors_Implementation(AMicroManageEquip* Equip, const TArray<AActor*>& Actors)
{
	// make all characters fly
	TArray<TTuple<ACharacter*, EMovementMode>> CharacterInfo;
	for (FConstPlayerControllerIterator Iterator = Equip->GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {
		ACharacter* Character = Iterator->Get()->GetCharacter();
		UCharacterMovementComponent* CharacterMovement = Character->GetCharacterMovement();
		auto MovementMode = CharacterMovement->MovementMode;
		bool IsFlying = CharacterMovement->bCheatFlying;
		if (!IsFlying) {
			CharacterInfo.Emplace(Character, MovementMode);
			CharacterMovement->bCheatFlying = true;
			CharacterMovement->SetMovementMode(MOVE_Flying);
		}
	}

	// process through all actors
	for (const auto& Actor : Actors) {
		// save factory connections
		TMap<UFGFactoryConnectionComponent*, UFGFactoryConnectionComponent*> FactoryConnections;
		for (const auto& ActorComp : Actor->GetComponentsByClass(UFGFactoryConnectionComponent::StaticClass())) {
			auto FactoryCxnComp = Cast<UFGFactoryConnectionComponent>(ActorComp);
			if (FactoryCxnComp->IsConnected()) {
				UFGFactoryConnectionComponent* Cxn = FactoryCxnComp->GetConnection();
				FactoryConnections.Add(FactoryCxnComp, Cxn);
			}
		}

		Actor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
		Actor->GetRootComponent()->SetMobility(EComponentMobility::Static);

		// reset factory connections
		for (const auto& Elem : FactoryConnections) {
			if (!Elem.Key->IsConnected()) {
				Elem.Key->SetConnection(Elem.Value);
			}
		}
	}

	// set all characters back to original movement mode
	for (const auto& CharMoveMode : CharacterInfo) {
		UCharacterMovementComponent* CharacterMovement = CharMoveMode.Key->GetCharacterMovement();
		CharacterMovement->bCheatFlying = false;
		CharacterMovement->SetMovementMode(CharMoveMode.Value);
	}

	// multi-cast out to all characters to redraw any changed objects
	Equip->MulticastRefreshMaterials(Actors);
}

bool UMicroManageRCO::ServerPrepareActors_Validate(AMicroManageEquip* Equip, const TArray<AActor*>& Actors)
{
	return true;
}

void UMicroManageRCO::ServerPaintActors_Implementation(const TArray<AActor*>& Actors, uint8 ColorSlot)
{
	// paint all buildables on server and colors will replicate on all clients
	for (const auto& Actor : Actors) {
		AFGBuildable* Buildable = Cast<AFGBuildable>(Actor);
		if (Buildable) {
			Buildable->SetColorSlot_Implementation(ColorSlot);
		}
	}
}

bool UMicroManageRCO::ServerPaintActors_Validate(const TArray<AActor*>& Actors, uint8 ColorSlot)
{
	return true;
}

void UMicroManageRCO::ServerHandleConnect_Implementation(AMicroManageEquip* Equip, bool IsConnection, AActor* OutputActor, AActor* InputActor)
{
	FString Title;
	FString Body;
	if (IsConnection) {
		UMicroManageSystem::Get()->Action->MakeConnection(OutputActor, InputActor, Title, Body);
	} else {
		UMicroManageSystem::Get()->Action->BreakConnection(OutputActor, InputActor, Title, Body);
	}
	Equip->MulticastShowPopup(Equip, Title, Body);
}

bool UMicroManageRCO::ServerHandleConnect_Validate(AMicroManageEquip* Equip, bool IsConnection, AActor* OutputActor, AActor* InputActor)
{
	return true;
}
