#include "stdafx.h"
#include "logger.h"

lua_State *logL = NULL;
void init_clientlog(lua_State *_l) {
	logL = _l;
}

void clientlog(const tchar* text)
{
    if (!logL) return;
	base::log(logL, text);
}

void clientlog(const tstring& text) 
{
	clientlog(text.c_str());
}
