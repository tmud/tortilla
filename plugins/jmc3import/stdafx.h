#pragma once

#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <assert.h>

#include <vector>
#include <string>
#include <map>

#include <CommDlg.h>
#include "api/api.h"

#include "dataQueue.h"
#include "memoryBuffer.h"

#include <atlbase.h>
#include <atlapp.h>
#include <atlctrls.h>