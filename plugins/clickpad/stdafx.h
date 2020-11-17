#pragma once

#define WINVER 0x0601
#define _WIN32_WINNT 0x0601
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <shellapi.h>

#include "api/api.h"

#include <atlbase.h>
#include <atlapp.h>
#include <atldlgs.h>
#include <atlctrls.h>
#include <atlctrlx.h>
#include <atlmisc.h>

#include "ToolTipDialog.h"

#include <map>
#include <vector>
#include <string>
#include <algorithm>

const int MAX_ROWS = 12;
const int MAX_COLUMNS = 12;

#include <assert.h>
#include "api/api.h"
