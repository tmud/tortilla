#pragma once

#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <shellapi.h>

#include "api/api.h"

#include <atlbase.h>
#include <atlapp.h>
#include <atldlgs.h>
#include <atlctrls.h>
#include <atlctrlx.h>

#include <map>
#include <vector>
#include <string>
#include <algorithm>

typedef std::wstring tstring;
typedef wchar_t tchar;

const int MAX_ROWS = 5;
const int MAX_COLUMNS = 10;

#include <assert.h>
