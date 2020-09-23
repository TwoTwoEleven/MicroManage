#include "MicroManageSystem.h"
#include "MicroManageUndo.h"
#include "MicroManageConfig.h"
#include "MicroManageSelection.h"
#include "MicroManageTransform.h"
#include "MicroManageUI.h"
#include "MicroManageAction.h"
#include "MicroManageInput.h"
#include "MicroManageRCO.h"

UMicroManageSystem* UMicroManageSystem::Get()
{
	if (!MicroManageSystemSingleton) {
		MicroManageSystemSingleton = NewObject<UMicroManageSystem>();
		MicroManageSystemSingleton->SystemId = FGuid::NewGuid();
		MicroManageSystemSingleton->Initialize(((UGameEngine*)GEngine)->GameInstance);
	}
	return MicroManageSystemSingleton;
}

void UMicroManageSystem::Initialize(UGameInstance* GameInstance)
{
	// force persistence of UMicroManageSystem object
	GameInstance->RegisterReferencedObject(this);
	CurrentWorld = GameInstance->GetWorld();

	// Init all components
	Transform = InitComponent<UMicroManageTransform>();
	Selection = InitComponent<UMicroManageSelection>();
	Undo = InitComponent<UMicroManageUndo>();
	Config = InitComponent<UMicroManageConfiguration>();
	UI = InitComponent<UMicroManageUI>();
	Action = InitComponent<UMicroManageAction>();
	Input = InitComponent<UMicroManageInput>();
}

UMicroManageRCO* UMicroManageSystem::GetMMRCO()
{
	if (!MMRCO) {
		MMRCO = Cast<UMicroManageRCO>(GetLocalController()->GetRemoteCallObjectOfClass(UMicroManageRCO::StaticClass()));
	}
	return MMRCO;
}

AMicroManageEquip* UMicroManageSystem::GetEquip()
{
	for (AMicroManageEquip* Equip : ActiveEquipment) {
		if (Equip->IsLocal) {
			return Equip;
		}
	}
	return ActiveEquipment[0];
}

void UMicroManageSystem::AddActiveEquipment(AMicroManageEquip* Equip)
{
	if (!ActiveEquipment.Contains(Equip)) {
		ActiveEquipment.Emplace(Equip);
	}
}

void UMicroManageSystem::RemoveActiveEquipment(AMicroManageEquip* Equip)
{
	if (ActiveEquipment.Contains(Equip)) {
		ActiveEquipment.Remove(Equip);
	}
}

UWorld* UMicroManageSystem::GetWorld() const
{
	return CurrentWorld;
}

AFGPlayerController* UMicroManageSystem::GetLocalController()
{
	if (!LocalController) {
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator) {
			if (Iterator->Get()->IsLocalPlayerController()) {
				LocalController = Cast<AFGPlayerController>(Iterator->Get());
				break;
			}
		}
	}
	return LocalController;
}

bool UMicroManageSystem::IsIdMatch(const FGuid& CheckId)
{
	return ((CheckId.A == SystemId.A) && (CheckId.B == SystemId.B) && (CheckId.C == SystemId.C) && (CheckId.D == SystemId.D));
}

FVector UMicroManageSystem::GetCameraViewVector()
{
	FVector CameraVector = GetLocalController()->PlayerCameraManager->GetCameraRotation().Vector();
	return FVector(CameraVector.X, CameraVector.Y, 0.f).GetSafeNormal(); // flatten and normalize
}

void UMicroManageSystem::BasicTransform(EActionNameIdx ActionIndex)
{
	FVector Location = FVector::ZeroVector;
	FRotator Rotation = FRotator::ZeroRotator;
	FVector Scale = FVector::OneVector;
	FMicroManageIncrement IncSetting = Config->MMConfig.IncrementSettings[Config->CurrentIncrementSize()];
	switch (ActionIndex) {
		case EActionNameIdx::SpinLeft:
			Rotation.Yaw = -IncSetting.DegreesToRotate; // -Rotate to Z
			break;
		case EActionNameIdx::SpinRight:
			Rotation.Yaw = IncSetting.DegreesToRotate; // Rotate to Z
			break;
		case EActionNameIdx::MoveUp:
			Location.Z = IncSetting.CentimetersToMove; // Move to Z
			break;
		case EActionNameIdx::MoveDown:
			Location.Z = -IncSetting.CentimetersToMove; // -Move to Z
			break;

		case EActionNameIdx::MoveLeft:
			Location.Y = -IncSetting.CentimetersToMove; // -Move to Y
			break;
		case EActionNameIdx::MoveRight:
			Location.Y = IncSetting.CentimetersToMove; // Move to Y
			break;
		case EActionNameIdx::MoveAway:
			Location.X = IncSetting.CentimetersToMove; // Move to X
			break;
		case EActionNameIdx::MoveToward:
			Location.X = -IncSetting.CentimetersToMove; // -Move to X
			break;

		case EActionNameIdx::RollLeft:
			Rotation.Roll = IncSetting.DegreesToRotate; // Rotate to X
			break;
		case EActionNameIdx::RollRight:
			Rotation.Roll = -IncSetting.DegreesToRotate; // -Rotate to X
			break;
		case EActionNameIdx::PitchAway:
			Rotation.Pitch = IncSetting.DegreesToRotate; // Rotate to Y
			break;
		case EActionNameIdx::PitchToward:
			Rotation.Pitch = -IncSetting.DegreesToRotate; // -Rotate to Y
			break;

		case EActionNameIdx::Shrink:
			Scale = FVector(1.f / ((IncSetting.PercentToGrow / 100.f) + 1.f));
			break;
		case EActionNameIdx::Grow:
			Scale = FVector(((IncSetting.PercentToGrow / 100.f) + 1.f));
			break;
	}
	Action->PrepareTransform(Location, Rotation, Scale);
}

void UMicroManageSystem::ExecuteAction(EActionNameIdx ActionIndex)
{
	if ((ActionIndex >= EActionNameIdx::SpinLeft) && (ActionIndex <= EActionNameIdx::Grow)) {
		BasicTransform(ActionIndex);
		return;
	}

	switch (ActionIndex) {
		case EActionNameIdx::NoAction:
			break;
		case EActionNameIdx::SelectTarget:
		case EActionNameIdx::DeselectTarget:
			Action->SelectActor(Selection->LineTraceFromPlayer(), ActionIndex == EActionNameIdx::SelectTarget);
			break;
		case EActionNameIdx::Undo:
			Action->PerformUndo();
			break;
		case EActionNameIdx::ChangeIncSize:
			Config->NextIncrementSize();
			Config->SaveMicroManageConfig();
			break;
		case EActionNameIdx::KnowNotes:
			UI->NextMMWidget();
			break;
		case EActionNameIdx::ShowTools:
			UI->ShowToolsUI();
			break;
		case EActionNameIdx::SetAnchor:
			if (Selection->SetAnchor(Selection->LineTraceFromPlayer())) {
				Action->MakeActorMovable(Selection->AnchorActor);
			}
			break;
		case EActionNameIdx::SetTarget:
			if (Selection->SetTarget(Selection->LineTraceFromPlayer())) {
				Action->MakeActorMovable(Selection->TargetActor);
			}
			break;

		case EActionNameIdx::AlignLeft:
			UI->ComingSoon();
			break;
		case EActionNameIdx::AlignLRCenter:
			UI->ComingSoon();
			break;
		case EActionNameIdx::AlignRight:
			UI->ComingSoon();
			break;
		case EActionNameIdx::SpaceLR:
			UI->ComingSoon();
			break;
		case EActionNameIdx::StackLR:
			UI->ComingSoon();
			break;
		case EActionNameIdx::LockScaleLR:
			Config->MMConfig.IsScaleLockedLR = !Config->MMConfig.IsScaleLockedLR;
			Config->SaveMicroManageConfig();
			break;

		case EActionNameIdx::AlignTop:
			UI->ComingSoon();
			break;
		case EActionNameIdx::AlignTBCenter:
			UI->ComingSoon();
			break;
		case EActionNameIdx::AlignBottom:
			UI->ComingSoon();
			break;
		case EActionNameIdx::SpaceTB:
			UI->ComingSoon();
			break;
		case EActionNameIdx::StackTB:
			UI->ComingSoon();
			break;
		case EActionNameIdx::LockScaleTB:
			Config->MMConfig.IsScaleLockedTB = !Config->MMConfig.IsScaleLockedTB;
			Config->SaveMicroManageConfig();
			break;

		case EActionNameIdx::AlignFront:
			UI->ComingSoon();
			break;
		case EActionNameIdx::AlignFBCenter:
			UI->ComingSoon();
			break;
		case EActionNameIdx::AlignBack:
			UI->ComingSoon();
			break;
		case EActionNameIdx::SpaceFB:
			UI->ComingSoon();
			break;
		case EActionNameIdx::StackFB:
			UI->ComingSoon();
			break;
		case EActionNameIdx::LockScaleFB:
			Config->MMConfig.IsScaleLockedFB = !Config->MMConfig.IsScaleLockedFB;
			Config->SaveMicroManageConfig();
			break;

		case EActionNameIdx::SameRotation:
			Action->MoveSelectionToTarget(true);
			break;
		case EActionNameIdx::SameScale:
			Action->SetSameScale();
			break;
		case EActionNameIdx::SamePaint:
			Action->SetSamePaint();
			break;

		case EActionNameIdx::Connect:
			GetMMRCO()->ServerHandleConnect(SystemId, true, Selection->AnchorActor, Selection->TargetActor);
			break;
		case EActionNameIdx::Disconnect:
			GetMMRCO()->ServerHandleConnect(SystemId, false, Selection->AnchorActor, Selection->TargetActor);
			break;

		case EActionNameIdx::SelectBoxSides:
		case EActionNameIdx::SelectBoxPivot:
			Selection->AddAnchorTargetBoxToSelection(ActionIndex == EActionNameIdx::SelectBoxSides);
			break;
		case EActionNameIdx::MoveSelection:
			Action->PrepareMove();
			break;
		case EActionNameIdx::CopySelection:
			UI->ComingSoon();
			break;
		case EActionNameIdx::NewSelection:
			UI->ShowConfirm(TITLE_START_NEW_SELECTION, BODY_START_NEW_SELECTION, Selection, "SelectClear");
			break;
		case EActionNameIdx::DeleteSelection:
			UI->ComingSoon();
			break;
		case EActionNameIdx::SaveSelection:
			Selection->SaveSelection();
			break;
		case EActionNameIdx::LoadSelection:
			Selection->LoadSelection();
			break;

		case EActionNameIdx::ClearUndo:
			Undo->ClearUndoStack();
			break;
		case EActionNameIdx::IsGrouped:
			Config->MMConfig.IsGrouped = !Config->MMConfig.IsGrouped;
			Config->SaveMicroManageConfig();
			break;
		case EActionNameIdx::IsViewBased:
			Config->MMConfig.IsViewBased = !Config->MMConfig.IsViewBased;
			Config->SaveMicroManageConfig();
			break;
		case EActionNameIdx::NextHologram:
			Selection->SelectNextMaterial();
			Config->SaveMicroManageConfig();
			break;
		case EActionNameIdx::Settings:
			UI->ComingSoon();
			break;
	}
}
