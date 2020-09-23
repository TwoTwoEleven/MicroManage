#pragma once

#include "CoreMinimal.h"
#include "MicroManageSystem.h"
#include "MicroManageConfig.h"
#include "MicroManageInput.generated.h"

UCLASS(BlueprintType)
class MICROMANAGE_API UMicroManageInput : public UMicroManageComponent
{
	GENERATED_BODY()

private:
	TMap<FKey, FTimerHandle> KeyTimerHandleMap;

	AMicroManageEquip* Equip;

	void SetupKeyBinding(FMicroManageKeyConfig KeyConfig, EInputEvent InputEvent);

	void SetupInputComponent();

	void ClearAllKeyTimers();

	void PerformIndexedAction(FKey Key, EActionNameIdx ActionIndex, EInputEvent InputEvent);

public:
	void Attach(AMicroManageEquip* Equipment);

	void Detach();

public:
	FORCEINLINE ~UMicroManageInput() = default;
};
