#pragma once

#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <tchar.h>

#include "api/api.h"

#include <string>
#include <vector>
#include <algorithm>
#include <map>
#include <functional>

typedef std::wstring tstring;
typedef wchar_t tchar;