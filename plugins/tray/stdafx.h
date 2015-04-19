#pragma once

#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <shellapi.h>

#include "api/api.h"

#include <atlbase.h>
#include <atlwin.h>
#include <atlapp.h>
#include <atldlgs.h>
#include <atlctrls.h>
#include <atlmisc.h>


#include <vector>
#include <string>
#include <assert.h>

typedef std::wstring tstring;

#define MAX_TIMEOUT 3600
#define MAX_INTERVAL 360
