#pragma once

#ifdef _DEBUG
void init_debug_helpers(lua_State *_l);
void debug_print(const tstring& text);
void debug_print(const tstring& text, int val);
#define DEBUGINIT(L) init_debug_helpers(L)
#define DEBUGOUT(x) debug_print(x)
#define DEBUGOUT2(x,y) debug_print(x,y)
#else
#define DEBUGINIT(x)
#define DEBUGOUT(x)
#define DEBUGOUT2(x)
#endif
