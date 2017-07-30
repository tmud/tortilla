#include "stdafx.h"
#include "logger.h"

lua_State *logL = NULL;
void init_log(lua_State *_l) {
	logL = _l;
}
void log(const tstring& text) 
{
	if (!logL) return;
	base::log(logL, text.c_str());
}