#pragma once

#include "CoreMinimal.h"
#include "UMG.h"
#include "UserWidget.h"
#include "UI/FGInteractWidget.h"
#include "MicroManageSystem.h"
#include "MicroManageToolWidget.generated.h"

UCLASS()
class MICROMANAGE_API UButtonProxy : public UUserWidget
{
	GENERATED_BODY()

public:
	static UButtonProxy* Create(EActionNameIdx AAction)
	{
		UButtonProxy* BtnProxy = NewObject<UButtonProxy>();
		BtnProxy->ToolAction = AAction;
		return BtnProxy;
	}

	EActionNameIdx ToolAction;

	UFUNCTION()
	void ClickEvent();

public:
	FORCEINLINE ~UButtonProxy() = default;
};

UCLASS(config = MicroManage)
class MICROMANAGE_API UMicroManageToolWidget : public UFGInteractWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY()
	TArray<UButtonProxy*> ButtonProxyArray;

	void HookWidget(EActionNameIdx ToolAction, UButton* Button, FString ToolTip);

	virtual void NativeConstruct() override;

public:
	UPROPERTY(meta = (BindWidget))
	UButton* btnAlignLeft;

	UPROPERTY(meta = (BindWidget))
	UButton* btnAlignLRCenter;

	UPROPERTY(meta = (BindWidget))
	UButton* btnAlignRight;

	UPROPERTY(meta = (BindWidget))
	UButton* btnSpaceLR;

	UPROPERTY(meta = (BindWidget))
	UButton* btnStackLR;

	UPROPERTY(meta = (BindWidget))
	UButton* btnLockScaleLR;

	UPROPERTY(meta = (BindWidget))
	UButton* btnAlignTop;

	UPROPERTY(meta = (BindWidget))
	UButton* btnAlignTBCenter;

	UPROPERTY(meta = (BindWidget))
	UButton* btnAlignBottom;

	UPROPERTY(meta = (BindWidget))
	UButton* btnSpaceTB;

	UPROPERTY(meta = (BindWidget))
	UButton* btnStackTB;

	UPROPERTY(meta = (BindWidget))
	UButton* btnLockScaleTB;

	UPROPERTY(meta = (BindWidget))
	UButton* btnAlignFront;

	UPROPERTY(meta = (BindWidget))
	UButton* btnAlignFBCenter;

	UPROPERTY(meta = (BindWidget))
	UButton* btnAlignBack;

	UPROPERTY(meta = (BindWidget))
	UButton* btnSpaceFB;

	UPROPERTY(meta = (BindWidget))
	UButton* btnStackFB;

	UPROPERTY(meta = (BindWidget))
	UButton* btnLockScaleFB;

	UPROPERTY(meta = (BindWidget))
	UButton* btnIsGrouped;

	UPROPERTY(meta = (BindWidget))
	UButton* btnIsViewBased;

	UPROPERTY(meta = (BindWidget))
	UButton* btnNextHologram;
	
	UPROPERTY(meta = (BindWidget))
	UButton* btnSettings;

	UPROPERTY(meta = (BindWidget))
	UButton* btnSameRotation;

	UPROPERTY(meta = (BindWidget))
	UButton* btnSameScale;

	UPROPERTY(meta = (BindWidget))
	UButton* btnSamePaint;

	UPROPERTY(meta = (BindWidget))
	UButton* btnConnect;

	UPROPERTY(meta = (BindWidget))
	UButton* btnDisconnect;

	UPROPERTY(meta = (BindWidget))
	UButton* btnSelectBoxSides;

	UPROPERTY(meta = (BindWidget))
	UButton* btnSelectBoxPivot;

	UPROPERTY(meta = (BindWidget))
	UButton* btnMoveSelection;

	UPROPERTY(meta = (BindWidget))
	UButton* btnCopySelection;

	UPROPERTY(meta = (BindWidget))
	UButton* btnNewSelection;

	UPROPERTY(meta = (BindWidget))
	UButton* btnDeleteSelection;

	UPROPERTY(meta = (BindWidget))
	UButton* btnSaveSelection;

	UPROPERTY(meta = (BindWidget))
	UButton* btnLoadSelection;

	UPROPERTY(meta = (BindWidget))
	UButton* btnClearUndo;

public:
	FORCEINLINE ~UMicroManageToolWidget() = default;
};
