#include "stdafx.h"
#include "base.h"

int module_test(lua_State *L)
{
    lua_pushnumber(L, 5);
    return 1;
}

// создает таблицу, с методом t.test(), который возвращает значение 5
// где t - таблица, которая находится на стеке при выходе из метода
// В коде, где загружается модуль, должен быть код вида:
// [local] module = require("module")
// тогда таблица t будет записана в перемнную module.

int luaopen_module(lua_State *L)
{
    lua_newtable(L);
    lua_pushstring(L, "test");
    lua_pushcfunction(L, module_test);
    lua_settable(L, -3);    
    return 1;
}
