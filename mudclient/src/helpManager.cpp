#include "stdafx.h"
#include "helpManager.h"

class RegHelper
{  HKEY key;
public:
    RegHelper() : key(NULL) {}
    ~RegHelper() { if (key) RegCloseKey(key); }
    bool openKey(HKEY bkey, const tstring& path)
    {
        if (RegOpenKeyEx(bkey, path.c_str(), 0, KEY_QUERY_VALUE, &key)== ERROR_SUCCESS)
            return true;
        return false;
    }
    bool getValue(const tstring& name, tstring* value)
    {
        WCHAR path[MAX_PATH+1];
        DWORD pathlen = MAX_PATH;
        DWORD type = REG_SZ;
        DWORD x = (RegQueryValueEx(key, name.c_str(), NULL, &type, (LPBYTE)&path, &pathlen));
        if (x == ERROR_SUCCESS)
        {
            value->assign(path);
            return true;
        }
        return false;
    }
};

bool check_exe(const tstring& path)
{
    if (path.find(L".exe") == -1)
        return false;
    if (path.find(L"%1") == -1)
        return false;
    return true;
}

bool openURL(const tstring& url)
{
    tstring browser_exe;
    {
        RegHelper rh;
        if (rh.openKey(HKEY_CURRENT_USER, L"Software\\Classes\\http\\shell\\open\\command"))
            rh.getValue(L"", &browser_exe);
    }

    if (!check_exe(browser_exe))
    {
        browser_exe.clear();
        tstring progid;
        {
            RegHelper rh;
            if (rh.openKey(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\http\\UserChoice"))
                rh.getValue(L"Progid", &progid);
        }
        if (!progid.empty())
        {
            tstring newkey(L"Software\\Classes\\");
            newkey.append(progid);
            newkey.append(L"\\shell\\open\\command");
            RegHelper rh;
            if (rh.openKey(HKEY_CURRENT_USER, newkey))
                rh.getValue(L"", &browser_exe);
            
            if (!check_exe(browser_exe))
            {
                browser_exe.clear();
                newkey.assign(progid);
                newkey.append(L"\\shell\\open\\command");
                RegHelper rh2;
                if (rh2.openKey(HKEY_CLASSES_ROOT, newkey))
                    rh2.getValue(L"", &browser_exe);
                if (!check_exe(browser_exe))
                    browser_exe.clear();
            }
        }    
    }

    if (browser_exe.empty())
    {
        RegHelper rh;
        if (rh.openKey(HKEY_CLASSES_ROOT, L"htmlfile\\shell\\open\\command") && rh.getValue(L"", &browser_exe))
        {
            if (browser_exe.find(L".exe") == -1) 
                browser_exe.clear();
            else
            {
                if (browser_exe.find(L"%1") == -1)
                    browser_exe.append(L" \"%1\"");
            }
        }
    }
    
    tstring_replace(&browser_exe, L"%1", url);
    
    int len = browser_exe.length();
    WCHAR *tmppath = new (std::nothrow) WCHAR[len+1];
    if (!tmppath)
        return false;
    wcscpy(tmppath, browser_exe.c_str());
  
    STARTUPINFO cif;
	ZeroMemory(&cif,sizeof(STARTUPINFO));
	PROCESS_INFORMATION pi;
    BOOL result = CreateProcess(NULL, tmppath, NULL, NULL, FALSE, 0, NULL, NULL, &cif, &pi);
    delete []tmppath;

    return result ? true : false;
}

void openHelp(HWND parent, const tstring& helplabel)
{
    tstring helpfile(L"help\\tortilla.htm");
    DWORD attr = GetFileAttributes(helpfile.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES || attr&FILE_ATTRIBUTE_DIRECTORY)
    {
        WCHAR buffer[MAX_PATH];
        LoadString(_Module.GetResourceInstance(), IDR_MAINFRAME, buffer, MAX_PATH-1);
        tstring message(L"Невозможно открыть справку! Отсутствует файл: ");
        message.append(helpfile);
        MessageBox(parent, message.c_str(), buffer, MB_OK|MB_ICONSTOP);
        return;
    }

    tstring path, path0;
    {
        WCHAR cdir[MAX_PATH];
        int len = GetCurrentDirectory(MAX_PATH, cdir);
        path.assign(cdir);
        int last = path.size()-1;
        if (path.at(last) != L'\\')
            path.append(L"\\");
        path.append(helpfile);
        path0 = path;
        for (int i=0,e=path.length(); i<e; ++i)
        {
            if (path.at(i) == L'\\')
                path.at(i) = L'/';
        }
    }

    tstring url;
    url.append(L"file:///");
    url.append(path);    
    if (!helplabel.empty()) { url.append(L"#"); url.append(helplabel); }
    if (!openURL(url))
    {
        WCHAR buffer[MAX_PATH];
        LoadString(_Module.GetResourceInstance(), IDR_MAINFRAME, buffer, MAX_PATH-1);
        tstring message(L"Невозможно открыть справку! Не получилось определить браузер по умолчанию. Справка находится тут: ");
        message.append(path0);
        MessageBox(parent, message.c_str(), buffer, MB_OK|MB_ICONSTOP);
        return;
    }
}
