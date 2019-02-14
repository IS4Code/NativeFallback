#include "natives.h"
#include "main.h"

namespace natives
{
	cell MapNative(AMX *amx, cell index, cell native)
	{
		if(index >= 0)
		{
			logprintf("[NativeFallback] MapNative: Native index must be negative (was %d).", index);
			amx_RaiseError(amx, AMX_ERR_NATIVE);
			return 0;
		}
		auto &map = native_map[amx];
		auto &reg_map = reg_native_map[amx];

		const char *native_name;
		amx_StrParam(amx, native, native_name);
		if(!native_name)
		{
			return 0;
		}
		auto it = reg_map.find(native_name);
		if(it == reg_map.end())
		{
			return 0;
		}
		map[index] = it->second;
		return 1;
	}

	cell NativeExists(AMX *amx, cell native)
	{
		const char *native_name;
		amx_StrParam(amx, native, native_name);
		if(!native_name)
		{
			return 0;
		}

		std::string name(native_name);
		auto &map = reg_native_map[amx];
		return registered.find(name) == registered.end() && map.find(name) != map.end();
	}

	cell AMX_NATIVE_CALL n_MapNative(AMX *amx, cell *params)
	{
		if(static_cast<ucell>(params[0]) < 2 * sizeof(cell))
		{
			logprintf("[NativeFallback] MapNative: Not enough arguments.");
			amx_RaiseError(amx, AMX_ERR_NATIVE);
			return 0;
		}
		return MapNative(amx, params[1], params[2]);
	}

	cell AMX_NATIVE_CALL n_NativeExists(AMX *amx, cell *params)
	{
		if(static_cast<ucell>(params[0]) < sizeof(cell))
		{
			logprintf("[NativeFallback] NativeExists: Not enough arguments.");
			amx_RaiseError(amx, AMX_ERR_NATIVE);
			return 0;
		}
		return NativeExists(amx, params[1]);
	}

	static AMX_NATIVE_INFO native_list[] =
	{
		{"MapNative", n_MapNative},
		{"NativeExists", n_NativeExists},
	};

	void reg(AMX *amx)
	{
		amx_Register(amx, native_list, sizeof(native_list) / sizeof(*native_list));
	}
}
