#pragma once

#define TORTILLA_VERSION L"0.98"
#define TORTILLA_VERSION_MAJOR 0
#define TORTILLA_VERSION_MINOR 98

#ifndef _UNICODE
#error("Support only unicode version!")
#endif

#define _CRT_SECURE_NO_DEPRECATE         // Disable the CRT deprecation warnings 4996
#define _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NON_CONFORMING_SWPRINTFS
#define WINVER		0x0501
#define _WIN32_WINNT	0x0501
#define _WIN32_IE	0x0501
#define _RICHEDIT_VER	0x0200

// only for debug (debuging text formatting)
#ifdef _DEBUG
//#define MARKERS_IN_VIEW
//#define _WINDBG
#endif

// Visual Studio Leak Detector
//#define VLD

#ifdef _DEBUG
#ifndef VLD
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#else
#include "vld.h"
#endif
#endif

#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlmisc.h>
//#include <atlcrack.h>

#include "resource.h"

#include <assert.h>

#include <list>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <set>
#include <deque>
#include <functional>
#include <memory>

typedef std::wstring tstring;
typedef WCHAR tchar;
typedef unsigned char tbyte;
typedef unsigned int uint;

#include "../common/memoryBuffer.h"
#include "../common/dataQueue.h"
#include "../common/pcreHelper.h"
#include "api/api.h"

#include "common/common.h"
#include "common/wideToAnsi.h"
#include "common/wideToUtf8.h"
#include "common/bevelLine.h"
#include "common/splitterEx.h"
#include "common/changeDir.h"
#include "common/paramsHelper.h"
#include "common/atldock.h"
#include "common/toolbarEx.h"
#include "common/criticalSection.h"
#include "common/luaSupport.h"
#include "common/tokenizer.h"

#define OUTPUT_WINDOWS 6
#define TIMERS_COUNT 10
#define MAINWND_CLASS_NAME L"TortillaMudClient"

extern CAppModule _Module;
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define default_profile_folder L"default"
#define default_profile_name L"player"
