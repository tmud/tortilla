#include "stdafx.h"
#include "debugHelpers.h"

#ifdef _DEBUG
const int debug_view = 3;
lua_State *L = NULL;
void init_debug_helpers(lua_State *_l) {
    L = _l;
} 
void debug_print(const tstring& text) {
    if (!L) return;
    lua_ref ref;
    ref.createRef(L);
    base::vprint(L, debug_view, text.c_str());
    ref.pushValue(L);
    ref.unref(L);
    tstring t(text); t.append(L"\r\n");
    OutputDebugString(t.c_str());
}
#endif
