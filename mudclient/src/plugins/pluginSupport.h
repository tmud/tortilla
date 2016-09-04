#pragma  once
#include "api/api.h"
#include "plugin.h"

class PluginsIdTableControl
{
public:
    PluginsIdTableControl(int firstid, int lastid);
    UINT registerPlugin(Plugin*p, int code, bool button);
    UINT unregisterByCode(Plugin*p, int code, bool button);
    void unregisterById(Plugin*p, UINT id);
    UINT findId(Plugin*p, int code, bool button);
    void runPluginCmd(UINT id);   

private:
    int  getIndex(Plugin*p, int code, bool button);
    struct idplugin { int code; Plugin *plugin; bool button;  };
    std::vector<idplugin> plugins_id_table;
    int first_id;
    int last_id;
};

class PluginColorSerialize
{
public:
     void serialize(const MudViewStringBlock &b, tstring* color);
     int  deserialize(const tchar* color, MudViewStringBlock *out);
};
