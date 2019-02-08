Native Fallback v1.0
==========

This plugin provides a fallback implementation for all native functions that are imported by any script, allowing it to be executed without missing functions.

The fallback implementation raises AMX_ERR_NOTFOUND and prints its name, when the function is called. You can use [PawnPlus](https://github.com/IllidanS4/PawnPlus/) to modify a particular fallback function. By default, only 256 unique fallback functions are provided (modify `fallback_pool_size` and recompile to increase); if more fallbacks need to be provided, a generic one is used (which doesn't print the function name).

## Installation
Download the latest [release](//github.com/IllidanS4/NativeFallback/releases/latest) for your platform to the "plugins" directory and add "NativeFallback" (or "NativeFallback.so" on Linux) to the `plugins` line in server.cfg.

## Building
Use Visual Studio to build the project on Windows, or `make` on Linux.
