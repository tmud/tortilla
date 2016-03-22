#include "stdafx.h"
#include "debugHelpers.h"

#ifdef _DEBUG
const int debug_view = 2;
lua_State *L = NULL;
void init_debug_helpers(lua_State *_l) {
    L = _l;
} 
void debug_print(const tstring& text) {
    if (!L) return;
    base::vprint(L, debug_view, text.c_str());
}
#endif
