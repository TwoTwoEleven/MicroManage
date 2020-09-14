#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FMicroManageModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;
	virtual bool IsGameModule() const override { return true; }
public:
	FORCEINLINE ~FMicroManageModule() = default;
};
