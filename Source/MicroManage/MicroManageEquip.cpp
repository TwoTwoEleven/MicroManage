#include "MicroManageEquip.h"
#include "MicroManageRCO.h"
#include "MicroManageSelection.h"
#include "MicroManageSystem.h"
#include "MicroManageUI.h"
#include "MicroManageInput.h"

AMicroManageEquip::AMicroManageEquip()
{
	bOnlyRelevantToOwner = true;
	bNetUseOwnerRelevancy = true;
	bReplicates = true;

	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//mAttachmentClass = AMicroManageAttachment::StaticClass();
	mEquipmentSlot = EEquipmentSlot::ES_ARMS;
	mAttachSocket = TEXT("hand_rSocket");
}

void AMicroManageEquip::BeginPlay()
{
	Super::BeginPlay();

	SetupMicroManageSystem();
	System->AddActiveEquipment(this);
}

void AMicroManageEquip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMicroManageEquip::SetupMicroManageSystem()
{
	if (!System) {
		System = UMicroManageSystem::Get();
	}
}

// Equip/UnEquip functions ------------------------------------------------------------------------

void AMicroManageEquip::Equip(AFGCharacterPlayer* Character)
{
	IsLocal = Character->IsLocallyControlled();
	SetupMicroManageSystem();
	System->AddActiveEquipment(this);
	if (IsLocal) {
		ManagerEquipped = true;
		System->UI->ShowMMWidget();
		System->Input->Attach(this);
	}
}

void AMicroManageEquip::MulticastUnEquip_Implementation()
{
	SetupMicroManageSystem();
	System->RemoveActiveEquipment(this);
	if (IsLocal && ManagerEquipped) {
		System->Input->Detach();
		System->UI->HideMMWidget();
		System->Selection->SelectClear();
		ManagerEquipped = false;
	}
}

void AMicroManageEquip::UnEquip()
{
	// UnEquip and ShouldSaveState are sent only to server.  multicast to allow clients to locally process their own unequip.
	if (Role == ENetRole::ROLE_Authority) {
		MulticastUnEquip();
	}
}

// Multicast action functions ---------------------------------------------------------------------

void AMicroManageEquip::MulticastShowPopup_Implementation(const FGuid& Id, const FString& Title, const FString& Body)
{
	if (System->IsIdMatch(Id)) {
		System->UI->ShowPopup(Title, Body);
	}
}

void AMicroManageEquip::MulticastTransformActors_Implementation(const TArray<AActor*>& Actors, FMicroManageTransformData TransformData)
{
	System->Transform->ProcessTransform(Actors, TransformData);
}

void AMicroManageEquip::MulticastUndoTransforms_Implementation(const FUndoInfo& UndoInfo)
{
	for (const auto& UndoComponent : UndoInfo.TransformComponents) {
		if (UndoComponent.Component->IsValidLowLevel()) {
			System->Transform->TransformComponent(UndoComponent.Component, UndoComponent.Transform);
		}
	}
	for (const auto& UndoActor : UndoInfo.TransformActors) {
		if (UndoActor.Actor->IsValidLowLevel()) {
			System->Transform->TransformActor(UndoActor.Actor, UndoActor.Transform);
		}
	}
}

void AMicroManageEquip::MulticastRefreshMaterials_Implementation(const TArray<AActor*>& Actors)
{
	System->Selection->RefreshMaterials(Actors);
}
