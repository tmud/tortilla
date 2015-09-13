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
    lua_pushstring(L, "path");
    lua_pushstring(L, "");
    lua_settable(L, -3);
    lua_pushstring(L, "cpath");
    lua_pushstring(L, W2U(path));
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
        WideToAnsi w2a(files[j].c_str());
        if (luaL_dofile(L, w2a))
        {
            tstring error(L"Ошибка при загрузке модуля: ");
            error.append(files[j]);
            pluginLog(error);
        }
    }
    return true;
}

void unloadModules()
{
    lua_getglobal(L, "munloadf");
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        return;
    }
    lua_pushnil(L);                     // first key
    while (lua_next(L, -2) != 0)        // key index = -2, value index = -1
    {
        if (lua_isfunction(L, -1))
            lua_pcall(L, 0, 0, 0);
        else
            lua_pop(L, 1);
    }
}
