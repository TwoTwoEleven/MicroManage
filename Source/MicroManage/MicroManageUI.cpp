#include "MicroManageUI.h"
#include "UI\FGGameUI.h"
#include "UI\Message\FGMessageBase.h"
#include "FGBlueprintFunctionLibrary.h"

void UMicroManageUI::NextMMWidget()
{
	CurrentWidgetType = static_cast<EWidgetPanel>((CurrentWidgetType + 1) % 2);
	ShowMMWidget();
}

void UMicroManageUI::HideMMWidget()
{
	if ((CurrentWidget != nullptr) && (CurrentWidget->IsValidLowLevel())) {
		CurrentWidget->RemoveFromParent();
		CurrentWidget = nullptr;
	}
}

void UMicroManageUI::ShowMMWidget()
{
	HideMMWidget();

	FStringClassReference WidgetClassReference;

	switch (CurrentWidgetType) {
	case EWidgetPanel::Placeholder:
		WidgetClassReference = FStringClassReference(TEXT(PLACEHOLDER_WIDGET_CLASS));
		break;
	case EWidgetPanel::InfoBoard:
		WidgetClassReference = FStringClassReference(TEXT(CLIPBOARD_WIDGET_CLASS));
		break;
	}
	if (UClass* locWidgetClass = WidgetClassReference.TryLoadClass<UUserWidget>()) {
		CurrentWidget = CreateWidget<UUserWidget>(System->GetLocalController()->GetGameInstance(), locWidgetClass);
		if (CurrentWidget) {
			CurrentWidget->AddToViewport(-1);
		}
	}
}

// ShowMessage - temp popup to show info at bottom of UI without requiring user interaction (display length in seconds)
// FGGameUI.h - AddPendingMessage?   FGMessageBase.h for message to add

void UMicroManageUI::ShowMessage(const FString& Message)
{
//	AFGHUD* HUD = Cast<AFGHUD>(System->Controller->MyHUD);
//	UFGGameUI* GameUI = HUD->GetGameUI();
//	UFGMessageBase* Msg = NewObject<UFGMessageBase>();
//	Msg->mTitle = FText::FromString("Title");
//	GameUI->AddPendingMessage(Msg);
}

void UMicroManageUI::PopupClosed(bool ConfirmClicked)
{

}

void UMicroManageUI::ShowPopup(const FString& Title, const FString& Message)
{
	FPopupClosed ClosedDelagate;
	ClosedDelagate.BindUFunction(this, "PopupClosed");
	UFGBlueprintFunctionLibrary::AddPopupWithCloseDelegate(System->GetLocalController(), 
		FText::FromString(Title), FText::FromString(Message), ClosedDelagate, EPopupId::PID_OK);
}

void UMicroManageUI::ShowConfirm(const FString& Title, const FString& Message, UObject* Object, const FName& FuncName)
{
	FPopupClosed ClosedDelagate;
	ClosedDelagate.BindUFunction(Object, FuncName);
	UFGBlueprintFunctionLibrary::AddPopupWithCloseDelegate(System->GetLocalController(), 
		FText::FromString(Title), FText::FromString(Message), ClosedDelagate, EPopupId::PID_OK_CANCEL);
}

void UMicroManageUI::ComingSoon()
{
	ShowPopup(TITLE_COMING_SOON, BODY_COMING_SOON);
}

void UMicroManageUI::ShowToolsUI()
{
	FStringClassReference WidgetClassReference = FStringClassReference(TEXT(TOOLBAR_WIDGET_CLASS));
	if (UClass* WidgetClass = WidgetClassReference.TryLoadClass<UUserWidget>()) {
		AFGHUD* HUD = Cast<AFGHUD>(System->GetLocalController()->MyHUD);
		if (HUD->IsValidLowLevel()) {
			HUD->OpenInteractUI(WidgetClass, System);
		}
	}
}
