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
	System = UMicroManageSystem::Get();
}

void AMicroManageEquip::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Equip/UnEquip functions ------------------------------------------------------------------------

void AMicroManageEquip::MicroManageEquip()
{
	ManagerEquipped = true;
	if (!System) {
		System = UMicroManageSystem::Get(); // Required to prevent crash if manager equipped on game load
	}
	System->Controller = ActiveController;
	System->MMRCO = Cast<UMicroManageRCO>(ActiveController->GetRemoteCallObjectOfClass(UMicroManageRCO::StaticClass()));
	System->Character = ActiveCharacter;
	System->Manager = this;

	System->UI->ShowMMWidget();
	System->Input->Attach();
}

void AMicroManageEquip::MicroManageUnEquip()
{
	if (ManagerEquipped) {
		System->Input->Detach();
		System->UI->HideMMWidget();
		System->Selection->SelectClear();
		ManagerEquipped = false;
	}
	System->Manager = nullptr;
}

void AMicroManageEquip::MulticastEquip_Implementation(AFGCharacterPlayer* Character, AMicroManageEquip* Equip)
{
	if ((Equip != nullptr) && (Equip->IsValidLowLevel()) && (Character != nullptr) && (Character->IsValidLowLevel()))
	{
//		Equip->ActiveCharacter = Character;
//		Equip->ActiveController = Cast<AFGPlayerController>(Character->Controller);

		AFGPlayerController* Controller = Cast<AFGPlayerController>(Character->Controller);
		if ((Controller != nullptr) && (Controller->IsValidLowLevel()) && (Controller->IsLocalController())) {

			Equip->ActiveCharacter = Character;
			Equip->ActiveController = Controller;
			Equip->MicroManageEquip();
		}
	}
}

void AMicroManageEquip::MulticastUnEquip_Implementation(AMicroManageEquip* Equip)
{
	if ((Equip != nullptr) && (Equip->IsValidLowLevel()) && 
		(Equip->ActiveController != nullptr) && (Equip->ActiveController->IsValidLowLevel()) &&
		(Equip->ActiveController->IsLocalController()))	{

		Equip->MicroManageUnEquip();
	}
}

void AMicroManageEquip::Equip(AFGCharacterPlayer* Character)
{
	// use the server's copy of this equipment for RCO compatibility/registration
	if (Role == ENetRole::ROLE_Authority) {
		MulticastEquip(Character, this);
	}
}

void AMicroManageEquip::UnEquip()
{
	// UnEquip and ShouldSaveState are sent only to server.  multicast to allow clients to locally process their own unequip.
	if (Role == ENetRole::ROLE_Authority) {
		MulticastUnEquip(this);
	}
}

// Multicast action functions ---------------------------------------------------------------------

void AMicroManageEquip::MulticastShowPopup_Implementation(AMicroManageEquip* Equip, const FString& Title, const FString& Body)
{
	if (Equip->IsValidLowLevel() && Equip->ActiveController->IsValidLowLevel() && Equip->ActiveController->IsLocalController()) {
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

// ------------------------

FVector AMicroManageEquip::GetCameraViewVector()
{
	FVector CameraVector = System->Controller->PlayerCameraManager->GetCameraRotation().Vector();
	return FVector(CameraVector.X, CameraVector.Y, 0.f).GetSafeNormal(); // flatten and normalize
}
