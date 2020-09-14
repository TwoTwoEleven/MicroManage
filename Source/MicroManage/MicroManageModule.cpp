#include "MicroManageModule.h"
#include "FGGameMode.h"
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
}
 
IMPLEMENT_GAME_MODULE(FMicroManageModule, MicroManage);
