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
    groups.init();

    for (int i=0,e=groups.getCount(); i<e; ++i)
    {
        tstring group;
        groups.getName(i, &group);
        ProfilePath pp(group, L"profiles\\player.txml");
        DeleteFile(pp);
    }

    m_first_startup = false;
    int last = groups.getLast();
    if (last == -1)
    {
        NewProfileHelper h;
        if (!h.createFromResources(groups))
            return false;
        m_first_startup = h.isFirstStartUp();
        m_profile =  h.getProfile();
        return true;
    }
    groups.getName(last, &m_profile.group);
    return true;
}

bool PropertiesManager::loadProfile(const tstring& force_profile, tstring *error)
{
    if (force_profile.empty())
    {
        if (!loadSettings(error))
            return false;
    }
    else 
    {
        Tokenizer t(force_profile.c_str(), L":");
        t.trimempty();
        if (t.size() != 2)
            return false;
        m_profile.name.assign( t[1] );
        m_profile.group.assign( t[0] );
    }
    if (loadProfileData(error))
    {
        tstring dummy;
        loadHistory(&dummy);
        return true;
    }
    return false;
}

bool PropertiesManager::loadSettings(tstring *error)
{
    xml::node sd;
    if (!loadFromFile(sd, L"settings.xml", error))
        return false;

    tstring profile;
    xml::request r(sd, L"profile");
    if (r.size() != 0)
        r[0].gettext(&profile);
    if (profile.empty())
        return false;

    m_profile.name.assign( profile );
    sd.deletenode();
    return true;
}

bool PropertiesManager::loadHistory(tstring* error)
{
    m_propData.cmd_history.clear();
    xml::node hd;
    if (!loadFromFile(hd, L"history.xml", error))
        return false;

    xml::request r(hd, L"cmd");
    for (int i=0,e=r.size(); i<e; ++i)
    {
        tstring cmd;
        r[i].gettext(&cmd);
        if (!cmd.empty())
           m_propData.cmd_history.push_back(cmd);
        if (i>MAX_CMD_HISTORY_SIZE)
            break;
    }
    hd.deletenode();
    return true;
}

bool PropertiesManager::loadProfileData(tstring *error)
{
    m_propData.initAllDefault();
    tstring profile(L"profiles\\");
    profile.append(m_profile.name);
    profile.append(L".xml");

    xml::node sd;
    if (!loadFromFile(sd, profile, error))
        return false;

    loadValue(sd, L"viewsize", MIN_VIEW_HISTORY_SIZE, MAX_VIEW_HISTORY_SIZE, &m_propData.view_history_size);
    loadValue(sd, L"cmdsize", MIN_CMD_HISTORY_SIZE, MAX_CMD_HISTORY_SIZE, &m_propData.cmd_history_size);
    loadValue(sd, L"systemcmds", 0, 1, &m_propData.show_system_commands);
    loadValue(sd, L"newlinecmds", 0, 1, &m_propData.newline_commands);
    loadValue(sd, L"clearbar", 0, 1, &m_propData.clear_bar);
    loadValue(sd, L"disableya", 0, 1, &m_propData.disable_ya);
    loadValue(sd, L"disableosc", 0, 1, &m_propData.disable_osc);
    loadValue(sd, L"historytab", 0, 1, &m_propData.history_tab);
    loadValue(sd, L"plogs", 0, 1, &m_propData.plugins_logs);
    loadValue(sd, L"plogswnd", 0, OUTPUT_WINDOWS, &m_propData.plugins_logs_window);
    loadValue(sd, L"softscroll", 0, 1, &m_propData.soft_scroll);
    loadValue(sd, L"unknowncmd", 0, 1, &m_propData.unknown_cmd);
    loadValue(sd, L"anyfont", 0, 1, &m_propData.any_font);
    loadValue(sd, L"disablealt", 0, 1, &m_propData.disable_alt);
    loadValue(sd, L"tray", 0, 1, &m_propData.move_totray);
    loadValue(sd, L"disablemccp", 0, 1, &m_propData.disable_mccp);
    tstring cp;
    loadString(sd, L"codepage", &cp);
    if (cp != L"win" && cp != L"utf8")
        cp = L"win";
    m_propData.codepage = cp;
    tstring logformat;
    loadString(sd, L"logformat", &logformat);
    if (logformat != L"html" && logformat != L"txt")
        logformat = L"html";
    m_propData.logformat = logformat;

    loadValue(sd, L"prompt", 0, 1, &m_propData.recognize_prompt);
    loadString(sd, L"ptemplate", &m_propData.recognize_prompt_template);
    if (m_propData.recognize_prompt_template.empty())
        m_propData.recognize_prompt = 0;

    m_propData.rebar.clear();
    loadString(sd, L"rebar", &m_propData.rebar);

    xml::request colors(sd, L"colors/color");
    for (int i=0,e=colors.size(); i<e; ++i)
    {
        tstring name; COLORREF clr;
        if (loadRgbColor(colors[i], &name, &clr))
        {
            if (name == L"background")
                m_propData.bkgnd = clr;
            else  
            {
                int id = 0;
                if (w2int(name, &id) && id >= 0 && id <= 15)
                    m_propData.colors[id] = clr;
            }
        }
    }

    xml::request font(sd, L"font");
    if (font.size())
    {
        xml::node f = font[0];
        f.get(L"italic", &m_propData.font_italic);
        f.get(L"bold", &m_propData.font_bold);
        f.get(L"height", &m_propData.font_heigth);
        tstring font_name;
        f.get(L"name", &font_name);
        if (!font_name.empty())
            m_propData.font_name = font_name;
    }

    loadArray(sd, L"groups/group", true, false, &m_propData.groups);
    m_propData.addDefaultGroup();
    loadArray(sd, L"actions/action", true, true, &m_propData.actions);
    loadArray(sd, L"aliases/alias", true, true, &m_propData.aliases);
    loadArray(sd, L"hotkeys/hotkey", true, true, &m_propData.hotkeys);
    loadArray(sd, L"subs/sub", false, true, &m_propData.subs);
    loadArray(sd, L"asubs/asub", false, true, &m_propData.antisubs);
    loadArray(sd, L"gags/gag", false, true, &m_propData.gags);
    loadArray(sd, L"highlights/highlight", true, true, &m_propData.highlights);
    PropertiesValues timers;
    loadArray(sd, L"timers/timer", true, true, &timers);
    PropertiesValues& t = m_propData.timers;
    for (int i=0,e=timers.size();i<e;++i)
    {
        const property_value& v = timers.get(i);
        int id = 0;
        if (w2int(v.key, &id) && id >= 1 && id <= TIMERS_COUNT)
            t.add(-1, v.key, v.value, v.group);
    }

    loadList(sd, L"tabwords/tabword", &m_propData.tabwords);
    loadArray(sd, L"variables/var", true, false, &m_propData.variables);

    xml::request msrq(sd, L"messages");
    if (msrq.size() == 1)
    {
        xml::node ms(msrq[0]);
        PropertiesData::message_data& d = m_propData.messages;
        loadValue(ms, L"actions", 0, 1, &d.actions);
        loadValue(ms, L"aliases", 0, 1, &d.aliases);
        loadValue(ms, L"subs", 0, 1, &d.subs);
        loadValue(ms, L"antisubs", 0, 1, &d.antisubs);
        loadValue(ms, L"hotkeys", 0, 1, &d.hotkeys);
        loadValue(ms, L"highlights", 0, 1, &d.highlights);
        loadValue(ms, L"gags", 0, 1, &d.gags);
        loadValue(ms, L"groups", 0, 1, &d.groups);
        loadValue(ms, L"vars", 0, 1, &d.variables);
        loadValue(ms, L"timers", 0, 1, &d.timers);
        loadValue(ms, L"tabwords", 0, 1, &d.tabwords);
    }

    m_propData.displays.load(sd);

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
    xml::node sd(L"profile");

    saveValue(sd, L"viewsize", m_propData.view_history_size);
    saveValue(sd, L"cmdsize", m_propData.cmd_history_size);
    saveValue(sd, L"systemcmds", m_propData.show_system_commands);
    saveValue(sd, L"newlinecmds", m_propData.newline_commands);
    saveValue(sd, L"clearbar", m_propData.clear_bar);
    saveValue(sd, L"disableya", m_propData.disable_ya);
    saveValue(sd, L"disableosc", m_propData.disable_osc);
    saveValue(sd, L"historytab", m_propData.history_tab);
    saveValue(sd, L"plogs", m_propData.plugins_logs);
    saveValue(sd, L"plogswnd", m_propData.plugins_logs_window);
    saveValue(sd, L"softscroll", m_propData.soft_scroll);
    saveValue(sd, L"unknowncmd", m_propData.unknown_cmd);
    saveValue(sd, L"anyfont", m_propData.any_font);
    saveValue(sd, L"disablealt", m_propData.disable_alt);
    saveValue(sd, L"tray", m_propData.move_totray);
    saveValue(sd, L"disablemccp", m_propData.disable_mccp);
    saveString(sd, L"codepage", m_propData.codepage);
    saveString(sd, L"logformat", m_propData.logformat);
    saveValue(sd, L"prompt", m_propData.recognize_prompt);
    saveString(sd, L"ptemplate", m_propData.recognize_prompt_template);
    saveString(sd, L"rebar", m_propData.rebar);

    xml::node c = sd.createsubnode(L"colors");
    saveRgbColor(c, L"background", m_propData.bkgnd);
    for (int i=0; i<16; ++i) {
         tstring buffer;
         int2w(i, &buffer);
         saveRgbColor(c, buffer, m_propData.colors[i]);
    }

    xml::node f = sd.createsubnode(L"font");
    f.set(L"italic", m_propData.font_italic);
    f.set(L"bold", m_propData.font_bold);
    f.set(L"height", m_propData.font_heigth);
    f.set(L"name", m_propData.font_name);

    xml::node actions = sd.createsubnode(L"actions");
    saveArray(actions, L"action", m_propData.actions);
    xml::node aliases = sd.createsubnode(L"aliases");
    saveArray(aliases, L"alias", m_propData.aliases);
    xml::node hotkeys = sd.createsubnode(L"hotkeys");
    saveArray(hotkeys, L"hotkey", m_propData.hotkeys);
    xml::node subs = sd.createsubnode(L"subs");
    saveArray(subs, L"sub", m_propData.subs);
    xml::node asubs = sd.createsubnode(L"asubs");
    saveArray(asubs, L"asub", m_propData.antisubs);
    xml::node gags = sd.createsubnode(L"gags");
    saveArray(gags, L"gag", m_propData.gags);
    xml::node hl = sd.createsubnode(L"highlights");
    saveArray(hl, L"highlight", m_propData.highlights);
    xml::node timers = sd.createsubnode(L"timers");
    saveArray(timers, L"timer", m_propData.timers);
    xml::node groups = sd.createsubnode(L"groups");
    saveArray(groups, L"group", m_propData.groups);
    xml::node tabwords = sd.createsubnode(L"tabwords");
    saveList(tabwords, L"tabword", m_propData.tabwords);
    xml::node vars = sd.createsubnode(L"variables");
    saveArray(vars, L"var", m_propData.variables);

    xml::node ms = sd.createsubnode(L"messages");
    PropertiesData::message_data& d = m_propData.messages;
    saveValue(ms, L"actions", d.actions);
    saveValue(ms, L"aliases", d.aliases);
    saveValue(ms, L"subs", d.subs);
    saveValue(ms, L"antisubs", d.antisubs);
    saveValue(ms, L"hotkeys", d.hotkeys);
    saveValue(ms, L"highlights", d.highlights);
    saveValue(ms, L"gags", d.gags);
    saveValue(ms, L"groups", d.groups);
    saveValue(ms, L"vars", d.variables);
    saveValue(ms, L"timers", d.timers);
    saveValue(ms, L"tabwords", d.tabwords);

    m_propData.displays.save(sd);

    tstring config(L"profiles\\");
    config.append(m_profile.name);
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

    xml::node hd(L"history");
    for (int i=0, e=h.size(); i<e; ++i) 
    {
        xml::node n = hd.createsubnode(L"cmd");
        n.settext(h[i].c_str());
    }
    bool result = saveToFile(hd, L"history.xml");
    hd.deletenode();
    return result;
}

bool PropertiesManager::saveSettings()
{
    xml::node sd(L"settings");
    xml::node n = sd.createsubnode(L"profile");
    n.settext(m_profile.name.c_str());
    bool result = saveToFile(sd, L"settings.xml");
    sd.deletenode();
    return result;
}
//----------------------------------------------------------------------------
bool PropertiesManager::loadWindow(xml::node parent, OutputWindow* w)
{
    if (!parent.get(L"side", &w->side))
       w->side = DOCK_FLOAT;
    if (!parent.get(L"lastside", &w->lastside))
       w->lastside = DOCK_FLOAT;
    tstring name;
    if (loadRECT(parent, &w->pos) && parent.get(L"name", &name))
    {
        w->name = name;
        int width = 0; int height = 0;
        if (!parent.get(L"width", &width))
            width = w->pos.right-w->pos.left;
        if (!parent.get(L"height", &height))
            height = w->pos.bottom-w->pos.top;
        w->size.cx = width; w->size.cy = height;
        return true;
    }
    return false;
}

void PropertiesManager::saveWindow(xml::node parent, const OutputWindow& w)
{
    xml::node xw = parent.createsubnode(L"window");
    xw.set(L"name", w.name.c_str() );
    saveRECT(xw, w.pos);
    xw.set(L"side", w.side);
    xw.set(L"lastside", w.lastside);
    xw.set(L"width", w.size.cx);
    xw.set(L"height", w.size.cy);
}
//----------------------------------------------------------------------------
void PropertiesManager::loadArray(xml::node parent, const tstring& name, bool values_req, bool groups_req, PropertiesValues* values)
{
    xml::request r(parent, name.c_str());
    for (int i=0,e=r.size(); i<e; ++i)
    {
        tstring key, val, grp;
        if (r[i].get(L"key", &key) && !key.empty())
        {
            if (values_req)
            {
                bool value_exists = r[i].get(L"value", &val);
                if (!value_exists || val.empty())
                    continue;
            }
            if (groups_req)
            {
                bool group_exists = r[i].get(L"group", &grp);
                if (!group_exists || grp.empty())
                    continue;
            }
            if (values->exist(key)) 
            {
                if (!groups_req)
                    continue;
                int exist = values->find(key);
                const property_value&data = values->get(exist);
                if (data.group == grp)
                    continue;
            }
            values->add(-1, key, val, grp);
        }
    }
}

void PropertiesManager::saveArray(xml::node parent, const tstring& name, const PropertiesValues& values)
{
    for (int i=0,e=values.size(); i<e; ++i)
    {
        const property_value &data = values.get(i);
        xml::node data_node = parent.createsubnode(name.c_str());
        data_node.set(L"key", data.key.c_str());
        if (!data.value.empty())
            data_node.set(L"value", data.value.c_str());
        if (!data.group.empty())
            data_node.set(L"group", data.group.c_str());
    }
}
//----------------------------------------------------------------------------
void PropertiesManager::loadList(xml::node parent, const tstring& name, PropertiesList* values)
{
    xml::request r(parent, name.c_str());
    for (int i=0,e=r.size(); i<e; ++i)
    {
        tstring val;
        if (r[i].get(L"value", &val) && !val.empty())
        {
            if (values->exist(val)) 
                continue;
            values->add(-1, val);
        }
    }
}

void PropertiesManager::saveList(xml::node parent, const tstring& name, const PropertiesList& values)
{
    for (int i=0,e=values.size(); i<e; ++i)
    {
        const tstring &val = values.get(i);
        xml::node data_node = parent.createsubnode(name.c_str());
        data_node.set(L"value", val.c_str());
    }
}
//----------------------------------------------------------------------------
bool PropertiesManager::loadValue(xml::node parent, const tstring& name, int min, int max, int *value)
{
    xml::request r(parent, name.c_str());
    if (r.size() == 0)
        return false;
    int v = 0;
    if (!r[0].get(L"value", &v))
        return false;

    if (v < min)
        v = min;
    if (v > max)
        v = max;
    *value = v;
    return true;
}

void PropertiesManager::saveValue(xml::node parent, const tstring& name, int value)
{
    xml::node n = parent.createsubnode(name.c_str());
    n.set(L"value", value);
}

bool PropertiesManager::loadString(xml::node parent, const tstring& name, tstring* value)
{
     xml::request r(parent, name.c_str());
     if (r.size() == 0)
         return false;
     tstring v;
     if (!r[0].get(L"value", &v))
         return false;
     value->assign(v);
     return true;
}

void PropertiesManager::saveString(xml::node parent, const tstring& name, const tstring& value)
{
    xml::node n = parent.createsubnode(name.c_str());
    n.set(L"value", value);
}

bool PropertiesManager::loadRgbColor(xml::node n, tstring* name, COLORREF* color)
{
    tstring cn;
    if (!n.get(L"id", &cn))
        return false;
    int r=0; int g=0; int b=0;
    if (!n.get(L"r", &r) ||
        !n.get(L"g", &g) ||
        !n.get(L"b", &b)
       ) return false;
    name->assign(cn);
    *color = RGB(r,g,b);
    return true;
}

void PropertiesManager::saveRgbColor(xml::node parent, const tstring& name, COLORREF color)
{
    xml::node n = parent.createsubnode(L"color");
    n.set(L"id", name.c_str());
    n.set(L"r", GetRValue(color));
    n.set(L"g", GetGValue(color));
    n.set(L"b", GetBValue(color));    
}

bool PropertiesManager::loadFromFile(xml::node& node, const tstring& file, tstring* error)
{
    ProfilePath config(m_profile.group, file);
    bool result = (node.load(config, error)) ? true : false;
    return result;
}

bool PropertiesManager::saveToFile(xml::node node, const tstring& file)
{
    ProfilePath config(m_profile.group, file);
    return (node.save(config)) ? true : false;
}

bool PropertiesManager::loadRECT(xml::node n, RECT *rc)
{
    int left = 0; int right = 0; int top = 0; int bottom = 0;
    if (!n.get(L"left", &left) ||
        !n.get(L"right", &right) ||
        !n.get(L"top", &top) ||
        !n.get(L"bottom", &bottom))
        return false;

    rc->left = left;
    rc->right = right;
    rc->top = top;
    rc->bottom = bottom;
    return true;
}

void PropertiesManager::saveRECT(xml::node n, const RECT &rc)
{
    n.set(L"left", rc.left);
    n.set(L"right", rc.right);
    n.set(L"top", rc.top);
    n.set(L"bottom", rc.bottom);
}
//----------------------------------------------------------------------------
bool PropertiesManager::createEmptyProfile(const Profile& profile)
{
    m_propData.initAllDefault();
    m_propData.addDefaultGroup();
    m_profile = profile;
    ProfileDirHelper dh;
    if (!dh.makeDir(m_profile.group, L"profiles"))
        return false;
    bool result = saveProfileData();
    saveSettings();
    return result;
}

bool PropertiesManager::copyProfile(const Profile& src, const Profile& dst, tstring* error)
{
    NewProfileHelper h;
    if (!h.copy(src, dst)) {
        if (error)
            error->assign(L"Can't copy profile");
        return false;
    }
    return loadProfile(dst, error);
}

bool PropertiesManager::loadProfile(const Profile& profile, tstring* error)
{
    m_profile = profile;
    if (loadProfileData(error))
    {
        tstring dummy;
        loadHistory(&dummy);
        saveSettings();
        return true;
    }
    return false;
}

bool PropertiesManager::checkProfile(const Profile& profile)
{
    tstring path(L"profiles\\");
    path.append(profile.name);
    path.append(L".xml");
    xml::node sd;
    ProfilePath config(profile.group, path);
    bool result = sd.load(config, NULL);
    sd.deletenode();
    return result;
}