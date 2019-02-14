#ifndef HOOKS_H_INCLUDED
#define HOOKS_H_INCLUDED

#include "sdk/amx/amx.h"
#include <string>

namespace hooks
{
	void load();
	void unload();

	AMX_NATIVE get_fallback(const std::string &name);
}

#endif
