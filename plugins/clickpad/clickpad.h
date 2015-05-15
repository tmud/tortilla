#pragma once

void setFocusToMudClient();
void runGameCommand(const tstring& cmd);
HWND getFloatWnd();
HWND getMudclientWnd();
void exitEditMode();
lua_State* getLuaState();