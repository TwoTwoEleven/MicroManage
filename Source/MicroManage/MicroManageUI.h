#pragma once

#include "CoreMinimal.h"
#include "MicroManageSystem.h"
#include "MicroManageUI.generated.h"

#define PLACEHOLDER_WIDGET_CLASS "/Game/MicroManage/UI/MMPlaceholder.MMPlaceholder_C"
#define CLIPBOARD_WIDGET_CLASS "/Game/MicroManage/UI/MMInfoWidget.MMInfoWidget_C"
#define TOOLBAR_WIDGET_CLASS "/Game/MicroManage/UI/MMToolsUI.MMToolsUI_C"

#define TITLE_COMING_SOON "Coming Soon"
#define BODY_COMING_SOON "This functionality is not yet available."
#define TITLE_REQUIRES_TARGET "Error"
#define BODY_REQUIRES_TARGET "This function requires that a target is set first.\nShift-RMB"
#define TITLE_START_NEW_SELECTION "Start New Selection"
#define BODY_START_NEW_SELECTION "Clear the current selection and start a new one?"
#define TITLE_REQUIRES_ANCHOR_AND_TARGET "Error"
#define BODY_REQUIRES_ANCHOR_AND_TARGET	"This function requires that both anchor and target are set first.\nSet Anchor: Shift-LMB\nSet Target: Shift-RMB"
#define TITLE_CONNECTION_MADE "Connection Success"
#define BODY_CONNECTION_MADE "%s connection successful.\nAnchor:%s\nto\nTarget:%s"
#define TITLE_CONNECTION_NOT_FOUND "Connection Error"
#define BODY_CONNECTION_NOT_FOUND "No available connections found."
#define TITLE_CONNECTION_ERROR_SPLITTER "Connection Error"
#define BODY_CONNECTION_ERROR_SPLITTER "Splitter output can only be connected to converyor belts and lifts."
#define TITLE_DISCONNECTION_MADE "Disconnection Success"
#define BODY_DISCONNECTION_MADE "%d disconnection%s successful."

#define TITLE_WARNING_LARGE_MOVE "Warning"
#define BODY_WARNING_LARGE_MOVE "You are moving a large number of objects across a distance that may cross map tile boundries.\nThis may lead to the game freezing for a short bit after the move while the game recaculates.\n\nDo you wish to continue?"

UENUM()
enum EWidgetPanel
{
	Placeholder,
	InfoBoard
};

UCLASS(BlueprintType)
class MICROMANAGE_API UMicroManageUI : public UMicroManageComponent
{
	GENERATED_BODY()

private:
	EWidgetPanel CurrentWidgetType = EWidgetPanel::InfoBoard;
	UUserWidget* CurrentWidget;

public:
	UFUNCTION()
	void PopupClosed(bool ConfirmClicked);

	void ShowPopup(const FString& Title, const FString& Message);

	void ShowConfirm(const FString& Title, const FString& Message, UObject* Object, const FName& FuncName);

	void ShowToolsUI();

	void HideMMWidget();
	void ShowMMWidget();
	void NextMMWidget();

	void ComingSoon();

public:
	FORCEINLINE ~UMicroManageUI() = default;
};
