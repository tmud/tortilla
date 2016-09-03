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
#include <set>
#include <unordered_map>
#include <functional>
#include <deque>

typedef std::wstring tstring;
typedef wchar_t tchar;

#include "common.h"
#include "../common/memoryBuffer.h"
#include "../common/dataQueue.h"
