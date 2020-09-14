#include "MicroManageInput.h"
#include "MicroManageEquip.h"
#include "FGInputLibrary.h"
#include "Components/InputComponent.h"

void UMicroManageInput::ClearAllKeyTimers()
{
	for (auto& KeyTimer : KeyTimerHandleMap) {
		System->Manager->GetWorldTimerManager().ClearTimer(KeyTimer.Value);
	}
	KeyTimerHandleMap.Empty();
}

void UMicroManageInput::SetupKeyBinding(FMicroManageKeyConfig KeyConfig, EInputEvent InputEvent)
{
	FInputChord Chord = FInputChord(KeyConfig.Key, KeyConfig.Shift, KeyConfig.Ctrl, KeyConfig.Alt, false);
	FInputKeyBinding KeyBinding = FInputKeyBinding(Chord, InputEvent);
	KeyBinding.bConsumeInput = true;
	KeyBinding.bExecuteWhenPaused = false;

	if ((KeyConfig.UseRepeats) && (InputEvent == EInputEvent::IE_Pressed) && ((KeyConfig.Key == EKeys::LeftMouseButton) ||
		(KeyConfig.Key == EKeys::MiddleMouseButton) || (KeyConfig.Key == EKeys::RightMouseButton))) {

		// use alternate delegate that handles simulated mouse button repeats by adding a timer/delegate when pressed
		KeyBinding.KeyDelegate.GetDelegateWithKeyForManualSet().BindLambda([=](const FKey& Key)
		{
			if (!KeyTimerHandleMap.Contains(KeyConfig.Key)) {
				FTimerDelegate TimerCallback;
				TimerCallback.BindLambda([=]
				{
					PerformIndexedAction(KeyConfig.Key, KeyConfig.ActionIndex, EInputEvent::IE_Repeat);
				});
				FTimerHandle TimerHandle;
				System->Manager->GetWorldTimerManager().SetTimer(TimerHandle, TimerCallback, 0.1, true, 0.5);
				KeyTimerHandleMap.Add(KeyConfig.Key, TimerHandle);
			}
			PerformIndexedAction(Key, KeyConfig.ActionIndex, InputEvent);
		});
	} else {
		KeyBinding.KeyDelegate.GetDelegateWithKeyForManualSet().BindLambda([=](const FKey& Key)
		{
			PerformIndexedAction(Key, KeyConfig.ActionIndex, InputEvent);
		});
	}
	System->Manager->InputComponent->KeyBindings.Add(KeyBinding);
}

void UMicroManageInput::SetupInputComponent()
{
	if (System->Manager->InputComponent) {
		return;
	}
	System->Manager->InputComponent = NewObject<UInputComponent>();

	for (const auto& ThisKey : System->Config->MMKeyConfigs.ActionKeys) {
		if (ThisKey.Key == EKeys::Invalid) {
			continue;
		}
		SetupKeyBinding(ThisKey, EInputEvent::IE_Pressed);
		if (ThisKey.UseRepeats) {
			if ((ThisKey.Key == EKeys::LeftMouseButton) || (ThisKey.Key == EKeys::MiddleMouseButton) ||
				(ThisKey.Key == EKeys::RightMouseButton)) {

				SetupKeyBinding(ThisKey, EInputEvent::IE_Released);
			} else {
				SetupKeyBinding(ThisKey, EInputEvent::IE_Repeat);
			}
		}
	}
}

void UMicroManageInput::PerformIndexedAction(FKey Key, EActionNameIdx ActionIndex, EInputEvent InputEvent)
{
	// remove timer if the key has been released
	if (InputEvent == EInputEvent::IE_Released) {
		if (KeyTimerHandleMap.Contains(Key)) {
			System->Manager->GetWorldTimerManager().ClearTimer(KeyTimerHandleMap[Key]);
			KeyTimerHandleMap.Remove(Key);
		}
		return;
	}

	// check to see if Key is actually pressed for an existing timer and remove it if it's not
	if (InputEvent == EInputEvent::IE_Repeat) { 
		if (KeyTimerHandleMap.Contains(Key) && !System->Controller->IsInputKeyDown(Key)) {
			System->Manager->GetWorldTimerManager().ClearTimer(KeyTimerHandleMap[Key]);
			KeyTimerHandleMap.Remove(Key);
			return;
		}
	}

	System->ExecuteAction(ActionIndex);
}

void UMicroManageInput::Attach()
{
	SetupInputComponent();
	System->Controller->PushInputComponent(System->Manager->InputComponent);
}

void UMicroManageInput::Detach()
{
	ClearAllKeyTimers();
	System->Controller->PopInputComponent(System->Manager->InputComponent);
}