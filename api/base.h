#pragma once

#pragma warning(disable: 4996)
#pragma execution_character_set("utf-8") // works only in Visual Studio!

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
#pragma comment(lib, "lua.lib")

#include "modules.h"
