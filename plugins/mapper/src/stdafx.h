#pragma once
#define _CRT_NON_CONFORMING_SWPRINTFS

#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <vector>

#include "api/api.h"

typedef std::wstring tstring;
typedef unsigned int uint;

#include <assert.h>
#include "common/crc32.h"
#include "common/autodel.h"
#include "common/dataQueue.h"

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlw.h>
#include <atlmisc.h>
#include <atlctrlx.h>

#include "resource.h"

#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <list>
