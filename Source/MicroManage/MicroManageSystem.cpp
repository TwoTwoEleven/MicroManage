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
		// force persistence of UMicroManageSystem object
		((UGameEngine*)GEngine)->GameInstance->RegisterReferencedObject(MicroManageSystemSingleton);
		MicroManageSystemSingleton->ID = FMath::Rand();

		// Init all components
		MicroManageSystemSingleton->Transform = InitComponent<UMicroManageTransform>();
		MicroManageSystemSingleton->Selection = InitComponent<UMicroManageSelection>();
		MicroManageSystemSingleton->Undo = InitComponent<UMicroManageUndo>();
		MicroManageSystemSingleton->Config = InitComponent<UMicroManageConfiguration>();
		MicroManageSystemSingleton->UI = InitComponent<UMicroManageUI>();
		MicroManageSystemSingleton->Action = InitComponent<UMicroManageAction>();
		MicroManageSystemSingleton->Input = InitComponent<UMicroManageInput>();
	}
	return MicroManageSystemSingleton;
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
			MMRCO->ServerHandleConnect(Manager, true, Selection->AnchorActor, Selection->TargetActor);
			break;
		case EActionNameIdx::Disconnect:
			MMRCO->ServerHandleConnect(Manager, false, Selection->AnchorActor, Selection->TargetActor);
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
