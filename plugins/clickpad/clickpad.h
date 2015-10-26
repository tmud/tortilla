#pragma once

void setFocusToMudClient();
void processGameCommand(const tstring& cmd, bool template_cmd);
HWND getFloatWnd();
HWND getMudclientWnd();
void exitEditMode();
lua_State* getLuaState();
bool onlyNumbers(const tstring& str);
bool s2i(const tstring& number, int *value);
void getImagesDir(tstring* dir);
