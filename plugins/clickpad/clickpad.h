#pragma once

void setFocusToMudClient();
void processGameCommand(const std::wstring& cmd, bool template_cmd);
HWND getFloatWnd();
HWND getMudclientWnd();
void exitEditMode();
lua_State* getLuaState();
bool onlyNumbers(const std::wstring& str);
bool s2i(const std::wstring& number, int *value);
void getImagesDir(std::wstring* dir);
