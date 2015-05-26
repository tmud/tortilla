#include "stdafx.h"
#include "MainFrm.h"
#include "network/network_init.h"
#include "profiles/profilesPath.h"

#ifdef _DEBUG
#pragma comment(lib, "zlibd.lib")
#else
#pragma comment(lib, "zlib.lib")
#endif

CAppModule _Module;
CMainFrame _wndMain;

int Run(LPTSTR /*lpstrCmdLine*/, int nCmdShow = SW_SHOWDEFAULT)
{
    NetworkInit n;
    if (!n.init())
    {
        msgBox(NULL, IDS_ERROR_INIT_NETWORK, MB_OK|MB_ICONERROR);
        return 0;
    }

#ifndef _DEBUG  // для запуска новых копий клиента из панели задач
    {
        DWORD a = GetFileAttributes(L"gamedata");
        if (a == INVALID_FILE_ATTRIBUTES || !(a&FILE_ATTRIBUTE_DIRECTORY))
        {
            WCHAR path[MAX_PATH+1];
            GetModuleFileName(NULL, path, MAX_PATH);
            WCHAR *p = wcsrchr(path, L'\\');
            int len = p-path+1;
            std::wstring new_cdir(path, len);
            SetCurrentDirectory(new_cdir.c_str());
        }
    }
#endif

	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);

	if(_wndMain.CreateEx() == NULL)
		return 0;

	int nRet = theLoop.Run();
	_Module.RemoveMessageLoop();
	return nRet;
}

#ifdef _DEBUG
//#include "C:/Program Files (x86)\Visual Leak Detector/include/vld.h"
#endif
int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int nCmdShow)
{
#ifdef _DEBUG
    _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

    HRESULT hRes = ::CoInitialize(NULL);
    ATLASSERT(SUCCEEDED(hRes));
	
    ::DefWindowProc(NULL, 0, 0, 0L);                            // this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
    AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls

    hRes = _Module.Init(NULL, hInstance);
    ATLASSERT(SUCCEEDED(hRes));

    Run(lpstrCmdLine, nCmdShow);

    _Module.Term();
    ::CoUninitialize();
    return 0;
}
