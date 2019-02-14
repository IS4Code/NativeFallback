#ifndef MAIN_H_INCLUDED
#define MAIN_H_INCLUDED

#include "sdk/amx/amx.h"
#include <vector>
#include <unordered_map>

typedef void(*logprintf_t)(const char* format, ...);
extern logprintf_t logprintf;

extern std::unordered_map<std::string, AMX_NATIVE> registered;

extern std::unordered_map<AMX*, std::unordered_map<cell, AMX_NATIVE>> native_map;
extern std::unordered_map<AMX*, std::unordered_map<std::string, AMX_NATIVE>> reg_native_map;

#endif
