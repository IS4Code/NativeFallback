#ifndef NATIVES_H_INCLUDED
#define NATIVES_H_INCLUDED

#include "sdk/amx/amx.h"

namespace natives
{
	cell AMX_NATIVE_CALL MapNative(AMX *amx, cell index, cell native);
	cell AMX_NATIVE_CALL NativeExists(AMX *amx, cell native);

	void reg(AMX *amx);
}

#endif
