#pragma once

#ifdef _WINDBG

#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
LPTOP_LEVEL_EXCEPTION_FILTER hOldFilter = NULL;

LONG WINAPI CustomUnhandledExceptionFilter( PEXCEPTION_POINTERS pExInfo )
{
    HANDLE hFile = CreateFile(L"crash.dmp", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (INVALID_HANDLE_VALUE == hFile)
        return EXCEPTION_EXECUTE_HANDLER;
    MINIDUMP_EXCEPTION_INFORMATION eInfo;
    eInfo.ThreadId = GetCurrentThreadId();
    eInfo.ExceptionPointers = pExInfo;
    eInfo.ClientPointers = FALSE;
    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &eInfo, NULL, NULL);
    CloseHandle(hFile);
    return EXCEPTION_EXECUTE_HANDLER;
}

void initWinDbg()
{
    hOldFilter = SetUnhandledExceptionFilter( CustomUnhandledExceptionFilter );
}

void releaseWinDbg()
{
    SetUnhandledExceptionFilter(hOldFilter);
}

#else
void initWinDbg() {}
void releaseWinDbg() {}
#endif
