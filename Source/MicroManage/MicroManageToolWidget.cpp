#include "MicroManageToolWidget.h"

void UButtonProxy::ClickEvent()
{
	UMicroManageSystem::Get()->ExecuteAction(ToolAction);
}

void UMicroManageToolWidget::HookWidget(EActionNameIdx ToolAction, UButton* Button, FString ToolTip)
{
	UButtonProxy* ButtonProxy = UButtonProxy::Create(ToolAction);
	ButtonProxyArray.Add(ButtonProxy); // for persistence
	Button->OnClicked.AddDynamic(ButtonProxy, &UButtonProxy::ClickEvent);
	Button->SetToolTipText(FText::FromString(ToolTip));
}

void UMicroManageToolWidget::NativeConstruct()
{
	Super::NativeConstruct();
	mUseKeyboard = true;
	//mUseMouse = true;
	//mCaptureInput = true;

	HookWidget(EActionNameIdx::AlignLeft, btnAlignLeft, "(Coming Soon) Align Left Edges To Anchor");
	HookWidget(EActionNameIdx::AlignLRCenter, btnAlignLRCenter, "(Coming Soon) Align Left-Right Centers To Anchor");
	HookWidget(EActionNameIdx::AlignRight, btnAlignRight, "(Coming Soon) Align Right Edges To Anchor");
	HookWidget(EActionNameIdx::SpaceLR, btnSpaceLR, "(Coming Soon) Space Objects Equally Left To Right");
	HookWidget(EActionNameIdx::StackLR, btnStackLR, "(Coming Soon) Stack Objects Left To Right");
	HookWidget(EActionNameIdx::LockScaleLR, btnLockScaleLR, "(Work In Progress) Lock Scaling Left To Right");

	HookWidget(EActionNameIdx::AlignTop, btnAlignTop, "(Coming Soon) Align Tops To Anchor");
	HookWidget(EActionNameIdx::AlignTBCenter, btnAlignTBCenter, "(Coming Soon) Align Top-Bottom Centers To Anchor");
	HookWidget(EActionNameIdx::AlignBottom, btnAlignBottom, "(Coming Soon) Align Bottoms To Anchor");
	HookWidget(EActionNameIdx::SpaceTB, btnSpaceTB, "(Coming Soon) Space Objects Equally Top To Bottom");
	HookWidget(EActionNameIdx::StackTB, btnStackTB, "(Coming Soon) Stack Objects Top To Bottom");
	HookWidget(EActionNameIdx::LockScaleTB, btnLockScaleTB, "(Work In Progress) Lock Scaling Top To Bottom");

	HookWidget(EActionNameIdx::AlignFront, btnAlignFront, "(Coming Soon) Align Front Edges To Anchor");
	HookWidget(EActionNameIdx::AlignFBCenter, btnAlignFBCenter, "(Coming Soon) Align Front-Back Centers To Anchor");
	HookWidget(EActionNameIdx::AlignBack, btnAlignBack, "(Coming Soon) Align Back Edges To Anchor");
	HookWidget(EActionNameIdx::SpaceFB, btnSpaceFB, "(Coming Soon) Space Objects Equally Front To Back");
	HookWidget(EActionNameIdx::StackFB, btnStackFB, "(Coming Soon) Stack Objects Front To Back");
	HookWidget(EActionNameIdx::LockScaleFB, btnLockScaleFB, "(Work In Progress) Lock Scaling Front To Back");

	HookWidget(EActionNameIdx::SameRotation, btnSameRotation, "Set Anchor (with Selection) to Target Rotation");
	HookWidget(EActionNameIdx::SameScale, btnSameScale, "Set Selection to Target Scale");
	HookWidget(EActionNameIdx::SamePaint, btnSamePaint, "Set Selection to Target Paint Color");

	HookWidget(EActionNameIdx::Connect, btnConnect, "Connect Anchor Output to Target Input");
	HookWidget(EActionNameIdx::Disconnect, btnDisconnect, "Disconnect Anchor Outputs from Target Inputs");

	HookWidget(EActionNameIdx::SelectBoxSides, btnSelectBoxSides, "Select items between Anchor and Target Sides");
	HookWidget(EActionNameIdx::SelectBoxPivot, btnSelectBoxPivot, "Select items between Anchor and Target Centers");
	HookWidget(EActionNameIdx::MoveSelection, btnMoveSelection, "Move Selection from Anchor to Target");
	HookWidget(EActionNameIdx::CopySelection, btnCopySelection, "(Coming Soon) Copy Selection from Anchor to Target");
	HookWidget(EActionNameIdx::NewSelection, btnNewSelection, "Start New Selection");
	HookWidget(EActionNameIdx::DeleteSelection, btnDeleteSelection, "(Coming Soon) Delete Selection");
	HookWidget(EActionNameIdx::SaveSelection, btnSaveSelection, "Save Selection");
	HookWidget(EActionNameIdx::LoadSelection, btnLoadSelection, "Load Selection");

	HookWidget(EActionNameIdx::IsGrouped, btnIsGrouped, "Grouped or Ungrouped Selection");
	HookWidget(EActionNameIdx::IsViewBased, btnIsViewBased, "View or Object Relative Actions");
	HookWidget(EActionNameIdx::NextHologram, btnNextHologram, "Switch To Next Hologram Style");
	HookWidget(EActionNameIdx::Settings, btnSettings, "(Coming Soon) Settings");
	HookWidget(EActionNameIdx::ClearUndo, btnClearUndo, "Clear Saved Undo Information");
}
