#include "MicroManageModule.h"
#include "FGGameMode.h"
#include "FGPlayerController.h"
#include "MicroManageRCO.h"
#include "SML/mod/hooking.h"

void FMicroManageModule::StartupModule()
{
	SUBSCRIBE_VIRTUAL_FUNCTION(AFGGameMode, AFGGameMode::PostLogin, [](auto& scope, AFGGameMode* gm, APlayerController* pc)
	{
		if (gm->HasAuthority() && !gm->IsMainMenuGameMode()) {
			gm->RegisterRemoteCallObjectClass(UMicroManageRCO::StaticClass());
		}
	});

	SUBSCRIBE_VIRTUAL_FUNCTION(AFGPlayerController, AFGPlayerController::PostInitializeComponents, [](auto& scope, AFGPlayerController* pc)
	{
		if (pc->IsLocalController()) { // force MicroManageSystem reset if we're starting up
			UMicroManageSystem::MicroManageSystemSingleton = nullptr;
		}
	});
}
 
IMPLEMENT_GAME_MODULE(FMicroManageModule, MicroManage);
