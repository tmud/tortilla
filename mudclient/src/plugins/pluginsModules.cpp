#include "stdafx.h"
#include "pluginsApi.h"
extern luaT_State L;

class Module
{
public:
    static bool isModule(const wchar_t* fname)
    {
        const wchar_t *e = wcsrchr(fname, L'.');
        if (!e) return false;
        tstring ext(e + 1);
        return (ext == L"lua") ? true : false;
    }
};

int require_stub(lua_State *L)
{
    lua_pushnil(L);
    return 1;
}

bool loadModules()
{
    ChangeDir cd;
    if (!cd.changeDir(L"modules"))
    {
        lua_register(L, "require", require_stub);
        return false;
    }

    tstring path(cd.getCurrentDir());
    path.append(L"\\modules\\?.dll");
    luaopen_package(L);
    luaT_pushwstring(L, L"path");
    luaT_pushwstring(L, L"");
    lua_settable(L, -3);
    luaT_pushwstring(L, L"cpath");
    luaT_pushwstring(L, path.c_str());
    lua_settable(L, -3);
    lua_setglobal(L, "package");
    lua_pop(L, 1);

    std::vector<tstring> files;
    {
        WIN32_FIND_DATA fd;
        memset(&fd, 0, sizeof(WIN32_FIND_DATA));
        HANDLE file = FindFirstFile(L"*.*", &fd);
        if (file != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    if (Module::isModule(fd.cFileName))
                        files.push_back(fd.cFileName);
                }
            } while (::FindNextFile(file, &fd));
            ::FindClose(file);
        }
    }

    for (int j = 0, je = files.size(); j < je; ++j)
    {
        TW2A w2a(files[j].c_str());
        if (luaL_dofile(L, w2a))
        {
            tstring error(L"Ошибка при загрузке модуля: ");
            error.append(files[j]);
            pluginOut(error.c_str());
        }
    }
    return true;
}

std::vector<lua_ref> m_unload_functions;
void unloadModules()
{
    int last = m_unload_functions.size()-1;
    for (int i=last;i>=0;--i)
    {
        m_unload_functions[i].pushValue(L);
        lua_pcall(L, 0, 0, 0);
        m_unload_functions[i].unref(L);
    }
    m_unload_functions.clear();
}

int regUnloadFunction(lua_State *L)
{
    if (!lua_isfunction(L, -1))
    {
        lua_pushboolean(L, 0);
        return 1;
    }
    lua_ref f;
    f.createRef(L);
    m_unload_functions.push_back(f);
    lua_pushboolean(L, 1);
    return 1;
}
