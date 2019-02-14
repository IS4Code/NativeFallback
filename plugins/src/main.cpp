#include "main.h"
#include "hooks.h"
#include "natives.h"

#include "sdk/amx/amx.h"
#include "sdk/plugincommon.h"

logprintf_t logprintf;
extern void *pAMXFunctions;

std::unordered_map<std::string, AMX_NATIVE> registered;

std::unordered_map<AMX*, std::unordered_map<cell, AMX_NATIVE>> native_map;
std::unordered_map<AMX*, std::unordered_map<std::string, AMX_NATIVE>> reg_native_map;

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports() 
{
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES;
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData)
{
	pAMXFunctions = ppData[PLUGIN_DATA_AMX_EXPORTS];
	logprintf = (logprintf_t)ppData[PLUGIN_DATA_LOGPRINTF];

	hooks::load();

	logprintf(" NativeFallback v1.1 loaded");
	logprintf(" Created by IllidanS4");
	return true;
}

PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
	hooks::unload();

	logprintf(" NativeFallback v1.1 unloaded");
}

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *amx) 
{
	natives::reg(amx);
	return AMX_ERR_NONE;
}

PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *amx) 
{
	native_map.erase(amx);
	reg_native_map.erase(amx);
	return AMX_ERR_NONE;
}
