#include "hooks.h"
#include "main.h"
#include "natives.h"
#include "func_pool.h"

#include "sdk/amx/amx.h"
#include "sdk/plugincommon.h"
#include "subhook/subhook.h"

#include <cassert>
#include <cstring>
#include <vector>
#include <unordered_map>

extern void *pAMXFunctions;

template <class FType>
class amx_hook_func;

template <class Ret, class... Args>
class amx_hook_func<Ret(*)(Args...)>
{
public:
	typedef Ret hook_ftype(Ret(*)(Args...), Args...);

	typedef Ret AMXAPI handler_ftype(Args...);

	template <subhook_t &Hook, hook_ftype *Handler>
	static Ret AMXAPI handler(Args... args)
	{
		return Handler(reinterpret_cast<Ret(*)(Args...)>(subhook_get_trampoline(Hook)), args...);
	}
};

template <int Index>
class amx_hook;

template <int Index>
class amx_hook_var
{
	friend class amx_hook<Index>;

	static subhook_t hook;
};

template <int Index>
subhook_t amx_hook_var<Index>::hook;

template <int Index>
class amx_hook
{
	typedef amx_hook_var<Index> var;

public:
	template <class FType, typename amx_hook_func<FType>::hook_ftype *Func>
	struct ctl
	{
		static void load()
		{
			typename amx_hook_func<FType>::handler_ftype *hookfn = &amx_hook_func<FType>::template handler<var::hook, Func>;

			var::hook = subhook_new(reinterpret_cast<void*>(((FType*)pAMXFunctions)[Index]), reinterpret_cast<void*>(hookfn), {});
			subhook_install(var::hook);
		}

		static void unload()
		{
			subhook_remove(var::hook);
			subhook_free(var::hook);
		}

		static void install()
		{
			subhook_install(var::hook);
		}

		static void uninstall()
		{
			subhook_remove(var::hook);
		}

		static FType orig()
		{
			if(subhook_is_installed(var::hook))
			{
				return reinterpret_cast<FType>(subhook_get_trampoline(var::hook));
			}else{
				return ((FType*)pAMXFunctions)[Index];
			}
		}
	};
};

#define AMX_HOOK_FUNC(Func, ...) AMXAPI Func(decltype(&::Func) _base_func, __VA_ARGS__) noexcept
#define base_func _base_func
#define amx_Hook(Func) amx_hook<PLUGIN_AMX_EXPORT_##Func>::ctl<decltype(&::amx_##Func), &hooks::amx_##Func>

namespace hooks
{
	constexpr size_t fallback_pool_size = 256;

	static std::vector<std::string> functions;

	static subhook_t getproperty_hook = nullptr;

	static cell fallback_handler(size_t id, AMX *amx, cell *params)
	{
		logprintf("[NativeFallback] Implementation for '%s' was not provided.", functions[id].c_str());
		amx_RaiseError(amx, AMX_ERR_NOTFOUND);
		return 0;
	}

	static cell default_fallback_handler(AMX *amx, cell *params)
	{
		logprintf("[NativeFallback] Implementation for native function was not provided.");
		amx_RaiseError(amx, AMX_ERR_NOTFOUND);
		return 0;
	}

	using pool = aux::func<cell AMX_NATIVE_CALL(AMX *amx, cell *params)>::pool<fallback_pool_size, fallback_handler>;

	int AMX_HOOK_FUNC(amx_Exec, AMX *amx, cell *retval, int index)
	{
		if(amx && (amx->flags & AMX_FLAG_BROWSE) == 0 && (amx->flags & AMX_FLAG_NTVREG) == 0)
		{
			int num;
			amx_NumNatives(amx, &num);

			std::vector<AMX_NATIVE_INFO> fallback;
			auto hdr = reinterpret_cast<AMX_HEADER*>(amx->base);
			for(int i = 0; i < num; i++)
			{
				auto native = reinterpret_cast<cell*>(amx->base + hdr->natives + i * hdr->defsize);
				if(*native == 0)
				{
					const char *name;
					if(hdr->defsize == sizeof(AMX_FUNCSTUBNT))
					{
						name = reinterpret_cast<char*>(amx->base + reinterpret_cast<AMX_FUNCSTUBNT*>(native)->nameofs);
					}else{
						name = reinterpret_cast<AMX_FUNCSTUB*>(native)->name;
					}

					logprintf("[NativeFallback] Native function '%s' was not registered.", name);

					auto it = registered.find(name);
					if(it != registered.end())
					{
						fallback.emplace_back(AMX_NATIVE_INFO{name, it->second});
					}else{
						auto next = pool::add();
						if(next.first == -1)
						{
							logprintf("[NativeFallback] Fallback function pool is full! Increase fallback_pool_size and recompile (currently %d).", fallback_pool_size);
							next.first = functions.size();
							next.second = default_fallback_handler;
						}

						assert(next.first == functions.size());
						functions.emplace_back(name);

						fallback.emplace_back(AMX_NATIVE_INFO{name, next.second});
						registered[name] = next.second;
					}
				}
			}

			amx_Register(amx, fallback.data(), fallback.size());
		}
		return base_func(amx, retval, index);
	}

	int AMX_HOOK_FUNC(amx_Callback, AMX *amx, cell index, cell *result, cell *params)
	{
		AMX_HEADER *hdr;
		if(index >= 0 || !amx || !(hdr = reinterpret_cast<AMX_HEADER*>(amx->base)))
		{
			return base_func(amx, index, result, params);
		}
		auto &map = native_map[amx];
		auto it = map.find(index);
		if(it == map.end())
		{
			return base_func(amx, index, result, params);
		}
		auto f = it->second;

		if(amx->sysreq_d != 0)
		{
			auto code = reinterpret_cast<cell*>(amx->base + hdr->cod + amx->cip - sizeof(cell));
			if(code[-1] == 123 && code[0] == index)
			{
				code[-1] = amx->sysreq_d;
				code[0] = reinterpret_cast<cell>(f);
			}
		}

		amx->error = AMX_ERR_NONE;
		*result = f(amx, params);
		return amx->error;
	}

	static cell AMX_NATIVE_CALL hook_getproperty(AMX *amx, cell *params)
	{
		auto trampoline = reinterpret_cast<AMX_NATIVE>(subhook_get_trampoline(getproperty_hook));
		if(!amx || !params || static_cast<ucell>(params[0]) < 3 * sizeof(cell))
		{
			return trampoline(amx, params);
		}
		switch(params[1])
		{
			case 0x4E464D4E:
				return natives::MapNative(amx, params[3], params[2]);
			case 0x4E464E45:
				return natives::NativeExists(amx, params[2]);
			default:
				return trampoline(amx, params);
		}
	}

	int AMX_HOOK_FUNC(amx_Register, AMX *amx, const AMX_NATIVE_INFO *nativelist, int number)
	{
		int ret = base_func(amx, nativelist, number);
		auto &map = reg_native_map[amx];
		for(int i = 0; nativelist[i].name != nullptr && (i < number || number == -1); i++)
		{
			if(!getproperty_hook)
			{
				if(!std::strcmp(nativelist[i].name, "getproperty"))
				{
					getproperty_hook = subhook_new(reinterpret_cast<void*>(nativelist[i].func), reinterpret_cast<void*>(hook_getproperty), {});
					subhook_install(getproperty_hook);
				}
			}
			map[nativelist[i].name] = nativelist[i].func;
		}
		return ret;
	}
}

void hooks::load()
{
	amx_Hook(Exec)::load();
	amx_Hook(Callback)::load();
	amx_Hook(Register)::load();
}

void hooks::unload()
{
	amx_Hook(Exec)::unload();
	amx_Hook(Callback)::unload();
	amx_Hook(Register)::unload();
	if(getproperty_hook)
	{
		subhook_remove(getproperty_hook);
		subhook_free(getproperty_hook);
	}
}
