#pragma once

void setFocusToMudClient();
void processGameCommand(const tstring& cmd, bool template_cmd);
HWND getFloatWnd();
HWND getMudclientWnd();
void exitEditMode();
lua_State* getLuaState();