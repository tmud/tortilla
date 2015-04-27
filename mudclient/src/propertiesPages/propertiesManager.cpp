#include "stdafx.h"
#include "propertiesManager.h"
#include "profiles/profilesPath.h"
#include "profiles/profileHelpers.h"

PropertiesManager::PropertiesManager() : m_first_startup(false)
{
}

PropertiesManager::~PropertiesManager()
{
}

bool PropertiesManager::init()
{
    ProfilesGroupList groups;
    if (!groups.init())
        return false;
    m_first_startup = groups.isFirstStartUp();
    m_default_profile = groups.isEmptyGroupList();
    int last = groups.getLast();
    groups.getName(last, &m_configName);
    return true;
}

bool PropertiesManager::loadProfile()
{
    if (loadSettings() && loadProfileData())
    {
        loadHistory();
        return true;
    }
    return false;
}

bool PropertiesManager::loadSettings()
{
    xml::node sd;
    if (!loadFromFile(sd, L"settings.xml"))
        return false;

    std::string profile;
    xml::request r(sd, "profile");
    if (r.size() != 0)
        r[0].gettext(&profile);
    if (profile.empty())
        return false;

    m_profileName.assign( Utf8ToWide(profile.c_str()) );
    sd.deletenode();
    return true;
}

bool PropertiesManager::loadHistory()
{
    m_propData.cmd_history.clear();
    xml::node hd;
    if (!loadFromFile(hd, L"history.xml"))
        return false;

    Utf8ToWide u2w;
    xml::request r(hd, "cmd");
    for (int i=0,e=r.size(); i<e; ++i)
    {
        std::string cmd;
        r[i].gettext(&cmd);
        if (!cmd.empty())
        {
            u2w.convert(cmd.c_str(), cmd.length());
            m_propData.cmd_history.push_back(tstring(u2w));
        }
        if (i>MAX_CMD_HISTORY_SIZE)
            break;
    }
    hd.deletenode();
    return true;
}

bool PropertiesManager::loadProfileData()
{
    m_propData.initAllDefault();
    tstring profile(L"profiles\\");
    profile.append(m_profileName);
    profile.append(L".xml");

    xml::node sd;
    if (!loadFromFile(sd, profile))
        return false;

    loadValue(sd, "viewsize", MIN_VIEW_HISTORY_SIZE, MAX_VIEW_HISTORY_SIZE, &m_propData.view_history_size);
    loadValue(sd, "cmdsize", MIN_CMD_HISTORY_SIZE, MAX_CMD_HISTORY_SIZE, &m_propData.cmd_history_size);
    loadValue(sd, "systemcmds", 0, 1, &m_propData.show_system_commands);
    loadValue(sd, "clearbar", 0, 1, &m_propData.clear_bar);
    loadValue(sd, "disableya", 0, 1, &m_propData.disable_ya);
    loadValue(sd, "disableosc", 0, 1, &m_propData.disable_osc);
    loadValue(sd, "historytab", 0, 1, &m_propData.history_tab);
    loadValue(sd, "timersf", 0, 1, &m_propData.timers_on);
    loadValue(sd, "plogs", 0, 1, &m_propData.plugins_logs);
    loadValue(sd, "plogswnd", 0, OUTPUT_WINDOWS, &m_propData.plugins_logs_window);
    tstring cp;
    loadString(sd, "codepage", &cp);
    if (cp != L"win" && cp != L"utf8")
        cp = L"win";
    m_propData.codepage = cp;
    loadValue(sd, "prompt", 0, 1, &m_propData.recognize_prompt);
    loadString(sd, "ptemplate", &m_propData.recognize_prompt_template);
    if (m_propData.recognize_prompt_template.empty())
        m_propData.recognize_prompt = 0;

    xml::request colors(sd, "colors/color");
    for (int i=0,e=colors.size(); i<e; ++i)
    {
        std::string name; COLORREF clr;
        if (loadRgbColor(colors[i], &name, &clr))
        {
            if (name == "background")
                m_propData.bkgnd = clr;
            else  
            {
                int id = 0;
                if (a2int(name, &id) && id >= 0 && id <= 15)
                    m_propData.colors[id] = clr;
            }
        }
    }

    xml::request font(sd, "font");
    if (font.size())
    {
        xml::node f = font[0];
        f.get("italic", &m_propData.font_italic);
        f.get("bold", &m_propData.font_bold);
        f.get("heigth", &m_propData.font_heigth);
        std::string font_name;
        f.get("name", &font_name);
        if (!font_name.empty())
            m_propData.font_name = Utf8ToWide(font_name.c_str());
    }

    loadArray(sd, "groups/group", true, false, &m_propData.groups);
    m_propData.addDefaultGroup();
    loadArray(sd, "actions/action", true, true, &m_propData.actions);
    loadArray(sd, "aliases/alias", true, true, &m_propData.aliases);
    loadArray(sd, "hotkeys/hotkey", true, true, &m_propData.hotkeys);
    loadArray(sd, "subs/sub", true, true, &m_propData.subs);
    loadArray(sd, "asubs/asub", false, true, &m_propData.antisubs);
    loadArray(sd, "gags/gag", false, true, &m_propData.gags);
    loadArray(sd, "highlights/highlight", true, true, &m_propData.highlights);
    loadArray(sd, "timers/timer", true, true, &m_propData.timers);
    loadList(sd, "tabwords/tabword", &m_propData.tabwords);
    loadArray(sd, "variables/var", true, false, &m_propData.variables);
    m_propData.plugins.clear();

    bool default_window = false;
    xml::request mw(sd, "mainwindow");
    if (mw.size())
    {
        xml::node w = mw[0];
        w.get("width", &m_propData.display_width);
        w.get("height", &m_propData.display_height);
        w.get("fullscreen", &m_propData.main_window_fullscreen);
        if (!loadRECT(w, &m_propData.main_window) ||
            m_propData.display_width != GetSystemMetrics(SM_CXVIRTUALSCREEN) || 
            m_propData.display_height != GetSystemMetrics(SM_CYVIRTUALSCREEN))
        {
            m_propData.initMainWindow();
            default_window = true;
        }
    }

    xml::request cw(sd, "windows/window");
    int e = cw.size();
    for (int i=0; i<OUTPUT_WINDOWS; ++i) 
    {
        OutputWindow w;
        if (!default_window && i<e)
        {
            xml::node xn = cw[i];
            if (loadWindow(cw[i], &w))
                m_propData.windows[i] = w;
        }
    }

    // load plugins and windows
    xml::request pp(sd, "plugins/plugin");
    for (int i = 0, e = pp.size(); i < e; ++i)
    {
        std::string name; int value = 0;
        xml::node pn = pp[i];
        pn.get("key", &name);
        pn.get("value", &value);
        if (!name.empty())
        {
            PluginData pd; pd.name = U2W(name);
            pd.state = (value == 1) ? 1 : 0;
            if (!default_window)
            {
                OutputWindow w;
                xml::request wp(pn, "windows/window");
                for (int j = 0, je = wp.size(); j < je; ++j)
                {
                    if (loadWindow(wp[j], &w))
                        pd.windows.push_back(w);
                }
            }
            m_propData.plugins.push_back(pd);
        }
    }

    xml::request msrq(sd, "messages");
    if (msrq.size() == 1)
    {
        xml::node ms(msrq[0]);
        PropertiesData::message_data& d = m_propData.messages;
        loadValue(ms, "actions", 0, 1, &d.actions);
        loadValue(ms, "aliases", 0, 1, &d.aliases);
        loadValue(ms, "subs", 0, 1, &d.subs);
        loadValue(ms, "antisubs", 0, 1, &d.antisubs);
        loadValue(ms, "hotkeys", 0, 1, &d.hotkeys);
        loadValue(ms, "highlights", 0, 1, &d.highlights);
        loadValue(ms, "gags", 0, 1, &d.gags);
        loadValue(ms, "groups", 0, 1, &d.groups);
        loadValue(ms, "vars", 0, 1, &d.variables);
        loadValue(ms, "timers", 0, 1, &d.timers);
        loadValue(ms, "tabwords", 0, 1, &d.tabwords);
    }

    sd.deletenode();
    m_propData.checkGroups();
    return true;
}

bool PropertiesManager::saveProfile()
{
    saveHistory();
    if (!saveProfileData())
        return false;
    if (!saveSettings())
        return false;
    return true;
}

bool PropertiesManager::saveProfileData()
{
    xml::node sd("profile");

    saveValue(sd, "viewsize", m_propData.view_history_size);
    saveValue(sd, "cmdsize", m_propData.cmd_history_size);
    saveValue(sd, "systemcmds", m_propData.show_system_commands);
    saveValue(sd, "clearbar", m_propData.clear_bar);
    saveValue(sd, "disableya", m_propData.disable_ya);
    saveValue(sd, "disableosc", m_propData.disable_osc);
    saveValue(sd, "historytab", m_propData.history_tab);
    saveValue(sd, "timersf", m_propData.timers_on);
    saveValue(sd, "plogs", m_propData.plugins_logs);
    saveValue(sd, "plogswnd", m_propData.plugins_logs_window);
    saveString(sd, "codepage", m_propData.codepage);
    saveValue(sd, "prompt", m_propData.recognize_prompt);
    saveString(sd, "ptemplate", m_propData.recognize_prompt_template);

    xml::node c = sd.createsubnode("colors");
    saveRgbColor(c, "background", m_propData.bkgnd);
    for (int i=0; i<16; ++i) {
         char buffer[4];
         itoa(i, buffer, 10);
         saveRgbColor(c, buffer, m_propData.colors[i]);
    }

    xml::node f = sd.createsubnode("font");
    f.set("italic", m_propData.font_italic);
    f.set("bold", m_propData.font_bold);
    f.set("heigth", m_propData.font_heigth);
    WideToUtf8 font_name(m_propData.font_name.c_str());
    f.set("name", font_name);

    xml::node actions = sd.createsubnode("actions");
    saveArray(actions, "action", m_propData.actions);
    xml::node aliases = sd.createsubnode("aliases");
    saveArray(aliases, "alias", m_propData.aliases);
    xml::node hotkeys = sd.createsubnode("hotkeys");
    saveArray(hotkeys, "hotkey", m_propData.hotkeys);
    xml::node subs = sd.createsubnode("subs");
    saveArray(subs, "sub", m_propData.subs);
    xml::node asubs = sd.createsubnode("asubs");
    saveArray(asubs, "asub", m_propData.antisubs);
    xml::node gags = sd.createsubnode("gags");
    saveArray(gags, "gag", m_propData.gags);
    xml::node hl = sd.createsubnode("highlights");
    saveArray(hl, "highlight", m_propData.highlights);
    xml::node timers = sd.createsubnode("timers");
    saveArray(timers, "timer", m_propData.timers);
    xml::node groups = sd.createsubnode("groups");
    saveArray(groups, "group", m_propData.groups);
    xml::node tabwords = sd.createsubnode("tabwords");
    saveList(tabwords, "tabword", m_propData.tabwords);
    xml::node vars = sd.createsubnode("variables");
    saveArray(vars, "var", m_propData.variables);

    xml::node windows = sd.createsubnode("windows");
    for (int i=0,e=m_propData.windows.size(); i<e; ++i)
        saveWindow(windows, m_propData.windows[i]);

    xml::node mw = sd.createsubnode("mainwindow");
    saveRECT(mw, m_propData.main_window);
    mw.set("width", m_propData.display_width);
    mw.set("height", m_propData.display_height);
    mw.set("fullscreen", m_propData.main_window_fullscreen);

    WideToUtf8 w2u;
    xml::node plugins = sd.createsubnode("plugins");
    for (int i = 0, e = m_propData.plugins.size(); i < e; ++i)
    {
        const PluginData &pd = m_propData.plugins[i];
        xml::node pn = plugins.createsubnode("plugin");
        w2u.convert(pd.name.c_str(), pd.name.length());
        pn.set("key", w2u);
        pn.set("value", pd.state == 0 ? 0 : 1);
        if (!pd.windows.empty())
        {
            xml::node pw = pn.createsubnode("windows");
            for (int j = 0, je = pd.windows.size(); j < je; ++j)
                saveWindow(pw, pd.windows[j]);
        }
    }

    xml::node ms = sd.createsubnode("messages");
    PropertiesData::message_data& d = m_propData.messages;
    saveValue(ms, "actions", d.actions);
    saveValue(ms, "aliases", d.aliases);
    saveValue(ms, "subs", d.subs);
    saveValue(ms, "antisubs", d.antisubs);
    saveValue(ms, "hotkeys", d.hotkeys);
    saveValue(ms, "highlights", d.highlights);
    saveValue(ms, "gags", d.gags);
    saveValue(ms, "groups", d.groups);
    saveValue(ms, "vars", d.variables);
    saveValue(ms, "timers", d.timers);
    saveValue(ms, "tabwords", d.tabwords);

    tstring config(L"profiles\\");
    config.append(m_profileName);
    config.append(L".xml");

    bool result = saveToFile(sd, config);
    sd.deletenode();
    return result;
}

bool PropertiesManager::saveHistory()
{
    std::vector<tstring> &h = m_propData.cmd_history;
    if (h.empty())
        return true;

    xml::node hd("history");
    WideToUtf8 w2u;
    for (int i=0, e=h.size(); i<e; ++i) 
    {
        xml::node n = hd.createsubnode("cmd");
        w2u.convert(h[i].c_str(), h[i].length());
        n.settext(w2u);
    }
    bool result = saveToFile(hd, L"history.xml");
    hd.deletenode();
    return result;
}

bool PropertiesManager::saveSettings()
{
    xml::node sd("settings");
    xml::node n = sd.createsubnode("profile");
    WideToUtf8 pname(m_profileName.c_str());
    n.settext(pname);
    bool result = saveToFile(sd, L"settings.xml");
    sd.deletenode();
    return result;
}
//----------------------------------------------------------------------------
bool PropertiesManager::loadWindow(xml::node parent, OutputWindow* w)
{
    if (!parent.get("side", &w->side))
       w->side = DOCK_FLOAT;
    if (!parent.get("lastside", &w->lastside))
       w->lastside = DOCK_FLOAT;
    std::string name;
    if (loadRECT(parent, &w->pos) && parent.get("name", &name))
    {
        w->name = U2W(name.c_str());
        int width = 0; int height = 0;
        if (!parent.get("width", &width))
            width = w->pos.right-w->pos.left;
        if (!parent.get("height", &height))
            height = w->pos.bottom-w->pos.top;
        w->size.cx = width; w->size.cy = height;
        return true;
    }
    return false;
}

void PropertiesManager::saveWindow(xml::node parent, const OutputWindow& w)
{
    WideToUtf8 w2u;
    w2u.convert(w.name.c_str(), w.name.length());
    xml::node xw = parent.createsubnode("window");
    xw.set("name", w2u );
    saveRECT(xw, w.pos);
    xw.set("side", w.side);
    xw.set("lastside", w.lastside);
    xw.set("width", w.size.cx);
    xw.set("height", w.size.cy);
}
//----------------------------------------------------------------------------
void PropertiesManager::loadArray(xml::node parent, const std::string& name, bool values_req, bool groups_req, PropertiesValues* values)
{
    xml::request r(parent, name.c_str());
    Utf8ToWide u2w;
    for (int i=0,e=r.size(); i<e; ++i)
    {
        std::string key, val, grp;
        if (r[i].get("key", &key) && !key.empty())
        {
            u2w.convert(key.c_str(), key.length());
            tstring _key(u2w);
            if (values->exist(_key)) 
                continue;
            if (values_req)
            {
                bool value_exists = r[i].get("value", &val);
                if (!value_exists || val.empty())
                    continue;
            }
            if (groups_req)
            {
                bool group_exists = r[i].get("group", &grp);
                if (!group_exists || grp.empty())
                    continue;
            }

            u2w.convert(val.c_str(), val.length());
            tstring value(u2w);
            u2w.convert(grp.c_str(), grp.length());
            tstring group(u2w);
            values->add(-1, _key, value, group);
        }
    }
}

void PropertiesManager::saveArray(xml::node parent, const std::string& name, const PropertiesValues& values)
{
    WideToUtf8 w2u;
    for (int i=0,e=values.size(); i<e; ++i)
    {
        const property_value &data = values.get(i);
        xml::node data_node = parent.createsubnode(name.c_str());
        w2u.convert(data.key.c_str(), data.key.length());
        data_node.set("key", w2u);
        if (!data.value.empty())
        {
            w2u.convert(data.value.c_str(), data.value.length());
            data_node.set("value", w2u);
        }
        if (!data.group.empty())
        {
            w2u.convert(data.group.c_str(), data.group.length());
            data_node.set("group", w2u);
        }
    }
}
//----------------------------------------------------------------------------
void PropertiesManager::loadList(xml::node parent, const std::string& name, PropertiesList* values)
{
    xml::request r(parent, name.c_str());
    Utf8ToWide u2w;
    for (int i=0,e=r.size(); i<e; ++i)
    {
        std::string val;
        if (r[i].get("value", &val) && !val.empty())
        {
            u2w.convert(val.c_str(), val.length());
            tstring _val(u2w);
            if (values->exist(_val)) 
                continue;
            values->add(-1, _val);
        }
    }
}

void PropertiesManager::saveList(xml::node parent, const std::string& name, const PropertiesList& values)
{
    WideToUtf8 w2u;
    for (int i=0,e=values.size(); i<e; ++i)
    {
        const tstring &val = values.get(i);
        xml::node data_node = parent.createsubnode(name.c_str());
        w2u.convert(val.c_str(), val.length());
        data_node.set("value", w2u);
    }
}
//----------------------------------------------------------------------------
bool PropertiesManager::loadValue(xml::node parent, const std::string& name, int min, int max, int *value)
{
    xml::request r(parent, name.c_str());
    if (r.size() == 0)
        return false;
    int v = 0;
    if (!r[0].get("value", &v))
        return false;

    if (v < min)
        v = min;
    if (v > max)
        v = max;
    *value = v;
    return true;
}

void PropertiesManager::saveValue(xml::node parent, const std::string& name, int value)
{
    xml::node n = parent.createsubnode(name.c_str());
    n.set("value", value);
}

bool PropertiesManager::loadString(xml::node parent, const std::string& name, tstring* value)
{
     xml::request r(parent, name.c_str());
     if (r.size() == 0)
         return false;
     std::string v;
     if (!r[0].get("value", &v))
         return false;

     U2W u2w(v);
     value->assign(u2w);
     return true;
}

void PropertiesManager::saveString(xml::node parent, const std::string& name, const tstring& value)
{
    W2U w2u(value);
    xml::node n = parent.createsubnode(name.c_str());
    n.set("value", w2u);
}

bool PropertiesManager::loadRgbColor(xml::node n, std::string* name, COLORREF* color)
{
    std::string cn;
    if (!n.get("id", &cn))
        return false;
    int r=0; int g=0; int b=0;
    if (!n.get("r", &r) ||
        !n.get("g", &g) ||
        !n.get("b", &b)
       ) return false;
    name->assign(cn);
    *color = RGB(r,g,b);
    return true;
}

void PropertiesManager::saveRgbColor(xml::node parent, const std::string& name, COLORREF color)
{
    xml::node n = parent.createsubnode("color");
    n.set("id", name.c_str());
    n.set("r", GetRValue(color));
    n.set("g", GetGValue(color));
    n.set("b", GetBValue(color));    
}

bool PropertiesManager::loadFromFile(xml::node& node, const tstring& file)
{
    WideToUtf8 config( ProfilePath(m_configName, file) );
    return (node.load(config)) ? true : false;        
}

bool PropertiesManager::saveToFile(xml::node node, const tstring& file)
{
    WideToUtf8 config(ProfilePath(m_configName, file));
    return (node.save(config)) ? true : false;
}

bool PropertiesManager::loadRECT(xml::node n, RECT *rc)
{
    int left = 0; int right = 0; int top = 0; int bottom = 0;
    if (!n.get("left", &left) ||
        !n.get("right", &right) ||
        !n.get("top", &top) ||
        !n.get("bottom", &bottom))
        return false;

    RECT pos = { left, top, right, bottom };
    *rc = pos;
    return true;
}

void PropertiesManager::saveRECT(xml::node n, const RECT &rc)
{
    n.set("left", rc.left);
    n.set("right", rc.right);
    n.set("top", rc.top);
    n.set("bottom", rc.bottom);
}
//----------------------------------------------------------------------------
bool PropertiesManager::createNewProfile(const tstring& name)
{
    m_propData.initAllDefault();
    m_profileName = name;
    m_propData.addDefaultGroup();
    bool result = saveProfileData();
    saveSettings();
    return result;
}

bool PropertiesManager::createCopyProfile(const tstring& from, const tstring& name)
{
    m_profileName = from;
    if (!loadProfileData())
        { m_profileName = name; return false; }

    m_profileName = name;
    m_propData.messages.initDefault();
    bool result = saveProfileData();
    saveSettings();
    return result;
}

bool PropertiesManager::loadNewProfile(const tstring& group, const tstring& name)
{
    m_configName = group;
    m_profileName = name;
    if (loadProfileData())
    {
        loadHistory();
        saveSettings();
        return true;
    }
    return false;
}

bool PropertiesManager::createNewProfile(const tstring& group, const tstring& name)
{
    m_propData.initAllDefault();
    m_propData.addDefaultGroup();
    m_configName = group;
    m_profileName = name;
    ProfileDirHelper dh;
    if (!dh.makeDir(m_configName, L"profiles"))
        return false;
    bool result = saveProfileData();
    saveSettings();
    return result;
}

bool PropertiesManager::renameProfile(const tstring& group, const tstring& name)
{
    m_configName = group;
    m_profileName = name;
    m_propData.initPlugins();
    ProfileDirHelper dh;
    if (!dh.makeDir(m_configName, L"profiles"))
        return false;
    m_propData.messages.initDefault();
    bool result = saveProfileData();
    saveSettings();
    return result;
}
