#include "stdafx.h"
#include "accessors.h"
#include "network/network.h"
#include "plugins/pluginsApi.h"
#include "msdpNetwork.h"

extern luaT_State L;
extern Plugin* _cp;

MsdpNetwork::MsdpNetwork() : m_state(false), m_utf8_encoding(false)
{
    m_to_send.setBufferSize(1024);
}

MsdpNetwork::~MsdpNetwork()
{
    releaseReports();
}

void MsdpNetwork::translateReceived(DataQueue& msdp_data)
{
    if (msdp_data.getSize() > 0)
        translate(msdp_data);
}

DataQueue& MsdpNetwork::getSendData()
{
    return m_to_send;
}

void MsdpNetwork::translate(DataQueue& msdp)
{
    const tbyte* data = (const tbyte*)msdp.getData();
    int len = msdp.getSize();
    //OUTPUT_BYTES(data, len, len, "MSDP");

    while (len > 0)
    {
        if (len < 3 || data[0] != IAC || data[2] != MSDP)
        {
            assert(false);
            msdp.clear();
            return;
        }

        tbyte cmd = data[1];
        if (cmd == SB)
        {
            const tbyte *p = data;
            for (int i=3; i<len; ++i)
            {
                if (p[i]==SE && p[i-1]==IAC && p[i-2]!=IAC)
                {
                    run_plugins_msdp(p+3, i-4);
                    len = i+1;
                    break;
                }
            }
            msdp.truncate(len);
        }
        else if (cmd == DO)
        {
            m_state = true;
            msdp.truncate(3);
            send_varval("CLIENT_NAME", "TORTILLA");
            send_varval("CLIENT_VERSION", TW2A(TORTILLA_VERSION));
            tortilla::getPluginsManager()->processPluginsMethod("msdpon", 0);
        }
        else if (cmd == DONT)
        {
            m_state = false;
            msdp.truncate(3);
            tortilla::getPluginsManager()->processPluginsMethod("msdpoff", 0);
            releaseReports();
        }
        else
        {
            assert(false);
            msdp.clear();
            return;
        }
        data = (const tbyte*)msdp.getData();
        len = msdp.getSize();
    }
}

void MsdpNetwork::send_varval(const char* var, const char* val)
{
    send_begin();
    send_param(MSDP_VAR, var);
    send_param(MSDP_VAL, val);
    send_end();
}

void MsdpNetwork::send_varvals(const char* var, const std::vector<std::string>& vals)
{
    send_begin();
    send_param(MSDP_VAR, var);
    for (int i=0,e=vals.size(); i<e; ++i)
        send_param(MSDP_VAL, vals[i].c_str());
    send_end();
}

void MsdpNetwork::send_begin()
{
    tbyte begin[] = { IAC, SB, MSDP };
    m_to_send.write(begin, 3);
}

void MsdpNetwork::send_end()
{
    tbyte end[] = { IAC, SE };
    m_to_send.write(end, 2);
}

void MsdpNetwork::send_param(tbyte param, const char* param_text)
{
    m_to_send.write(&param, 1);
    m_to_send.write(param_text, strlen(param_text));
}

bool MsdpNetwork::run_plugins_msdp(const tbyte* data, int len)
{
    //OUTPUT_BYTES(data, len, len, "PLUGINS MSDP");
    // translate msdp data into lua table
    lua_newtable(L);
    cursor c; c.p = data; c.e = data + len;
    while (c.p != c.e)
    {
        if (!process_var(c))
            { lua_pop(L, 1); return false; }
        if (!process_val(c))
            { lua_pop(L, 1); return false; }
    }
    // run plugins
    tortilla::getPluginsManager()->processPluginsMethodCopyArgs("msdp", 1);
    return true;
}

bool MsdpNetwork::process_var(cursor& c)
{
     const tbyte* p = c.p;
     if (p == c.e || *p != MSDP_VAR)
         return false;
     const tbyte* b = p+1;
     while (b != c.e && *b != MSDP_VAL)
     {
         if (*b < ' ') return false;
         b++;
     }
     if (b == c.e)
         return false;
     std::string var_name((const char*)(p+1), b-p-1);
     lua_pushstring(L, var_name.c_str());
     c.p = b;
     return true;
}

bool MsdpNetwork::process_val(cursor& c)
{
     const tbyte* p = c.p;
     if (p == c.e || *p != MSDP_VAL)
         return false;
     const tbyte* b = p+1;
     if (b == c.e)
     {
         lua_pushstring(L, "");
         lua_settable(L, -3);
         c.p = b;
         return true;
     }
     if (*b >= ' ')
     {
         while (b != c.e && *b >= ' ') b++;
         if (b != c.e && *b != MSDP_VAR && *b != MSDP_VAL && *b != MSDP_ARRAY_CLOSE && *b != MSDP_TABLE_CLOSE)
             return false;
         std::string value((const char*)(p+1), b-p-1);
         if (m_utf8_encoding) 
         {
             lua_pushstring(L, value.c_str());
         }
         else
         {
             std::string newval;
             const char* b0 = value.c_str();
             const tbyte* b = (const tbyte*)b0;
             const tbyte* e = b + value.length();             
             while (b != e) 
             {
                 const tbyte *p = b;
                 while (p!=e && *p!=0xff) p++;
                 if (p==e) break;
                 p++; if (p==e) break;
                 newval.append((const char*)b, p-b);
                 if (*p == 0xff) { p++; }
                 b = p;
             }
             newval.append((const char*)b);             
             TA2W ws(newval.c_str());         
             luaT_pushwstring(L, ws);
         }
         
         lua_settable(L, -3);
         c.p = b;
         return true;
     }
     else if (*b == MSDP_ARRAY_OPEN)
     {
         c.p = b+1;
         if (c.p == c.e)
             return false;

         int index = 1;               // index for array's vars
         lua_newtable(L);             // table for array
         while(true)
         {
            lua_pushinteger(L, index++);
            if (!process_val(c))
                { lua_pop(L, 2); return false; }
            if (*c.p != MSDP_VAL && *c.p != MSDP_ARRAY_CLOSE)
                { lua_pop(L, 2); return false; }
            if (*c.p == MSDP_ARRAY_CLOSE)
            {
                lua_settable(L, -3);
                c.p++;
                return true;
            }
         }
     }
     else if (*b == MSDP_TABLE_OPEN)
     {
         c.p = b+1;
         if (c.p == c.e)
             return false;

          lua_newtable(L);             // table for table
          while (true)
          {
               if (!process_var(c))
                  { lua_pop(L, 1); return false; }
                if (!process_val(c))
                    { lua_pop(L, 2); return false; }
                if (*c.p != MSDP_VAR && *c.p != MSDP_TABLE_CLOSE)
                    { lua_pop(L, 2); return false; }
                if (*c.p == MSDP_TABLE_CLOSE)
                {
                    lua_settable(L, -3);
                    c.p++;
                    return true;
                }
          }
     }
     else if (*b == MSDP_VAL || *b == MSDP_VAR)
     {
         lua_pushstring(L, "");
         lua_settable(L, -3);
         c.p = b;
         return true;
     }
     return false;
}

void MsdpNetwork::report(Plugin* p, std::vector<std::string> *report)
{
    std::vector<std::string> new_report;
    for (int i=0,e=report->size(); i<e; ++i)
    {
        const std::string &cmd = report->at(i);
        PluginIterator it = m_plugins_reports.find(cmd);
        if (it != m_plugins_reports.end())
        {
            PluginReport *pr = it->second;
            if (std::find(pr->begin(), pr->end(), p) == pr->end())
                pr->push_back(p);
        }
        else
        {
            PluginReport *pr = new PluginReport;
            m_plugins_reports[cmd] = pr;
            pr->push_back(p);
            new_report.push_back(cmd);
        }
    }
    report->swap(new_report);
}

void MsdpNetwork::unreport(Plugin* p, std::vector<std::string> *report)
{
    std::vector<std::string> new_report;
    for (int i = 0, e = report->size(); i < e; ++i)
    {
        const std::string &cmd = report->at(i);
        PluginIterator it = m_plugins_reports.find(cmd);
        if (it == m_plugins_reports.end())
            continue;
        PluginReport *pr = it->second;
        PluginReportIterator pt = std::find(pr->begin(), pr->end(), p);
        if (pt == pr->end())
            continue;
        pr->erase(pt);
        if (pr->empty())
        {
            m_plugins_reports.erase(it);
            delete pr;
            new_report.push_back(cmd);
        }
    }
    report->swap(new_report);
}

void MsdpNetwork::loadPlugin(Plugin *p)
{
    if (!m_state)
        return;
    tortilla::getPluginsManager()->processPluginMethod(p, "msdpon", 0);
}

void MsdpNetwork::unloadPlugin(Plugin *p)
{
    if (!m_state)
        return;
    tortilla::getPluginsManager()->processPluginMethod(p, "msdpoff", 0);
    std::vector<std::string> unreport;
    PluginIterator it = m_plugins_reports.begin(), it_end = m_plugins_reports.end();
    for (;it!=it_end;++it)
    {
        PluginReport* pr = it->second;
        PluginReportIterator pt = std::find(pr->begin(), pr->end(), p);
        if (pt == pr->end())
            continue;
        pr->erase(pt);
        if (pr->empty())
        {
            unreport.push_back(it->first);
            m_plugins_reports.erase(it);
            delete pr;
        }
    }
    if (!unreport.empty())
        send_varvals("UNREPORT", unreport);
}

void MsdpNetwork::loadPlugins()
{
    if (!m_state)
        return;
    tortilla::getPluginsManager()->processPluginsMethod("msdpon", 0);
}

void MsdpNetwork::unloadPlugins()
{
    if (!m_state)
        return;
    tortilla::getPluginsManager()->processPluginsMethod("msdpoff", 0);
    std::vector<std::string> unreport;
    PluginIterator it = m_plugins_reports.begin(), it_end = m_plugins_reports.end();
    for (;it!=it_end;++it)
        unreport.push_back(it->first);
    releaseReports();
    if (!unreport.empty())
        send_varvals("UNREPORT", unreport);
    m_state = false;
}

void MsdpNetwork::reset()
{
    if (!m_state)
        return;
    tortilla::getPluginsManager()->processPluginsMethod("msdpoff", 0);
    releaseReports();
    m_state = false;
}

void MsdpNetwork::setUtf8Encoding(bool flag)
{
    m_utf8_encoding = flag;
}

void MsdpNetwork::releaseReports()
{
    PluginIterator it = m_plugins_reports.begin(), it_end = m_plugins_reports.end();
    for (;it!=it_end;++it)
    {
        PluginReport* pr = it->second;
        delete pr;
    }
    m_plugins_reports.clear();
}

MsdpNetwork* getMsdp()
{
    return tortilla::getPluginsManager()->getMsdp();
}

bool msdp_isoff()
{
   return (!getMsdp()->state()) ? true : false;
}

int msdpOffError(lua_State *L, const char* fname) 
{
    TA2W f(fname);
    return pluginMethodError(f, L"MSDP is off");
}

int msdp_list(lua_State *L)
{
    if (msdp_isoff())
        return msdpOffError(L, "msdp.list");

    if (luaT_check(L, 1, LUA_TSTRING))
    {
        getMsdp()->send_varval("LIST", lua_tostring(L, 1));
        return 0;
    }
    return pluginInvArgs(L, L"msdp.list");
}

int msdp_multi_command(lua_State *L, const char* cmd, const char* cmdname)
{
    if (msdp_isoff())
        return msdpOffError(L, cmdname);
    if (luaT_check(L, 1, LUA_TSTRING) || luaT_check(L, 1, LUA_TTABLE))
    {
        std::vector<std::string> vals;
        if (lua_isstring(L, 1))
            vals.push_back(lua_tostring(L, 1));
        else
        {
            lua_pushnil(L);                     // first key
            while (lua_next(L, -2) != 0)        // key index = -2, value index = -1
            {
                if (!lua_isstring(L, -1))
                    pluginInvArgs(L, TA2W(cmdname));
                vals.push_back(lua_tostring(L, -1));
                lua_pop(L, 1);
            }
        }
        if (!strcmp(cmd,"REPORT"))
            getMsdp()->report(_cp, &vals);
        else if (!strcmp(cmd,"UNREPORT"))
            getMsdp()->unreport(_cp, &vals);
        if (!vals.empty())
            getMsdp()->send_varvals(cmd, vals);
        return 0;
    }
    return pluginInvArgs(L, TA2W(cmdname));
}

int msdp_reset(lua_State *L)
{
    return msdp_multi_command(L, "RESET", "msdp.reset");
}

int msdp_send(lua_State *L)
{
    return msdp_multi_command(L, "SEND", "msdp.send");
}

int msdp_report(lua_State *L)
{
    return msdp_multi_command(L, "REPORT", "msdp.report");
}

int msdp_unreport(lua_State *L)
{
    return msdp_multi_command(L, "UNREPORT", "msdp.unreport");
}

void reg_msdp(lua_State *L)
{
    lua_newtable(L);
    regFunction(L, "list", msdp_list);
    regFunction(L, "reset", msdp_reset);
    regFunction(L, "send", msdp_send);
    regFunction(L, "report", msdp_report);
    regFunction(L, "unreport", msdp_unreport);
    lua_setglobal(L, "msdp");
}
