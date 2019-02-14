/** NativeFallback v1.1 by IllidanS4 **/
//github.com/IllidanS4/NativeFallback

#if defined _inc_NativeFallback
#undef _inc_NativeFallback
#endif

#if defined _NativeFallback_included
#endinput
#endif
#define _NativeFallback_included

#include <core>

/*
native bool:MapNative(index, const name[]);
native bool:NativeExists(const name[]);
*/
#define MapNative(%0,%1) (bool:getproperty(.id=0x4E464D4E,.value=(%0),.name=(%1)))
#define NativeExists(%0) (bool:getproperty(.id=0x4E464E45,.name=(%0)))
