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

/*void int2w(int value, tstring* str)
{
    wchar_t buffer[16];
    swprintf(buffer, L"%d", value);
    str->assign(buffer);
}*/

void debug_print(const tstring& text, const tstring& text2)
{
    tstring out;
    out.assign(text);
    out.append(text2);
    debug_print(out);
}
#endif
