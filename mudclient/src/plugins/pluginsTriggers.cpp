#include "stdafx.h"
#include "pluginsApi.h"
#include "pluginsTriggers.h"
#include "pluginsParseData.h"
#include "accessors.h"
extern Plugin* _cp;
//-----------------------------------------------------------------------
triggerParseData::triggerParseData(triggerKeyData *t) : tr(t), m_current_compare_pos(0)
{
    int count = t->getLen();
    m_strings.resize(count, NULL);
    for (int i=0;i<count;++i) { m_strings[i] = new triggerParseDataString;  }        
    resetindex();
}

triggerParseData::~triggerParseData()
{
    m_parseData.detach();
    std::for_each(m_strings.begin(), m_strings.end(), [](triggerParseDataString* tpd) { delete tpd;} );
}

void triggerParseData::reset()
{
    m_parseData.detach();
    std::for_each(m_strings.begin(), m_strings.end(), [](triggerParseDataString* tpd) { tpd->clear(); });
    resetindex();
    m_current_compare_pos = 0;
}

void triggerParseData::resetindex()
{
    int count = m_strings.size();
    m_indexes.resize(count);
    for (int i=0;i<count;++i) m_indexes[i] = i;
}

bool triggerParseData::correctindex(int string_index) const 
{
    int count = m_indexes.size();
    return (string_index >= 0 && string_index < count) ? true : false;
}

void triggerParseData::pushString(const CompareData& cd, const CompareObject &co, bool incompl_flag)
{
    m_parseData.strings.push_back(cd.string);
    m_parseData.last_finished = !incompl_flag;
    triggerParseDataString* tpd = m_strings[m_current_compare_pos];
    co.getParameters(&tpd->params);
    cd.string->getMd5(&tpd->crc);
}
//-----------------------------------------------------------------------
PluginsTrigger::PluginsTrigger() : L(NULL), p(NULL), m_enabled(false)
{
}

PluginsTrigger::~PluginsTrigger()
{
    m_trigger_func_ref.unref(L);    
}

bool PluginsTrigger::init(lua_State *pl, Plugin *pp)
{
    assert(pl && pp);
    L = pl; 
    p = pp;
    if (luaT_check(L, 2, LUA_TSTRING, LUA_TFUNCTION) ||
        luaT_check(L, 2, LUA_TTABLE, LUA_TFUNCTION))
    {
        m_trigger_func_ref.createRef(L);  // save function on the top of stack
        if (lua_isstring(L, 1))
        {
            m_compare_objects.resize(1);
            tstring key(luaT_towstring(L, 1));
            if (!m_compare_objects[0].init(key, true))
                return false;
        }
        else
        {
            std::vector<tstring> keys;
            lua_pushnil(L);                     // first key
            while (lua_next(L, -2) != 0)        // key index = -2, value index = -1
            {
                if (!lua_isstring(L, -1))
                    return false;
                tstring key(luaT_towstring(L, -1));
                keys.push_back(key);
                lua_pop(L, 1);
            }
            bool all_empty = true;
            for (int i=0,e=keys.size();i<e;++i) {
                if (!keys[i].empty()) { all_empty = false; break; }
            }
            if (all_empty) 
                return false;
            m_compare_objects.resize(keys.size());
            for (int i=0,e=keys.size();i<e;++i) 
            {
                tstring k = keys[i];
                if (k.empty()) { k.assign(L"%%"); }
                if (!m_compare_objects[i].init(k, true))
                    return false;
            }
        }
        m_enabled = true;
        return true;
    }
    return false;
}

void PluginsTrigger::enable(bool enable)
{
    if (enable != m_enabled) {
        // clear m_triggers_in_comparing state
        for (int i=0,e=m_triggers_in_comparing.size(); i<e; ++i) {
            triggerParseData *t = m_triggers_in_comparing[i];
            t->reset();
            m_empty_data.push_back(t);
        }
        m_triggers_in_comparing.clear();
    }
    m_enabled = enable;
}

bool PluginsTrigger::isEnabled() const
{
    return m_enabled;
}

int PluginsTrigger::getLen() const
{
    return m_compare_objects.size();
}

bool PluginsTrigger::getKey(int index, tstring* key)
{
    int count = m_compare_objects.size();
    if (index >= 0 && index < count) {
        key->assign( m_compare_objects[index].getKey() );
        return true;
    }
    return false;
}

struct triggeredData : TriggerActionHook {
    PluginsTrigger* tr;
    triggerParseVector data;
    void run() {
        assert(tr);
        if (tr) 
        {
            tr->run(&data);
            tr = NULL;
        }
    }
};

TriggerAction PluginsTrigger::compare(const CompareData& cd, bool incompl_flag)
{
    if (!m_enabled)
        return std::shared_ptr<TriggerActionHook>();

    triggerParseVector ok, next;
    for (int i=0,e=m_triggers_in_comparing.size(); i<e; ++i) {
        triggerParseData *t = m_triggers_in_comparing[i];
        CompareResult result = compareParseData(t, cd, incompl_flag);
        if (result == CR_FAIL) {
            m_empty_data.push_back(t);
        } else if (result == CR_NEXT) {
            next.push_back(t);
        } else {
            ok.push_back(t);
        }
    }
    if (!next.empty())
        m_triggers_in_comparing.moveFrom(next);





    if (ok.empty())
        return std::shared_ptr<TriggerActionHook>();
    triggeredData *d = new triggeredData();
    d->tr = this;
    d->data.moveFrom(ok);
    return std::shared_ptr<TriggerActionHook>(d);
}

PluginsTrigger::CompareResult PluginsTrigger::compareParseData(triggerParseData* tpd, const CompareData& cd, bool incompl_flag)
{
    int pos = tpd->getComparePos();
    CompareObject &co = m_compare_objects[pos];
    
    bool result = false;
    if (incompl_flag && co.isFullstrReq()) {
        // not compared / full string req.
    }
    else {
        result = co.compare(cd.fullstr);
    }
    if (result)
    {
        tpd->pushString(cd, co, incompl_flag);
        int last = m_compare_objects.size() - 1;
        if (pos == last)
            return CR_OK;
        tpd->incComparePos();
        return CR_NEXT;
    }   
    tpd->reset();   
    return CR_FAIL;
}

void PluginsTrigger::freeTriggerData(triggerParseData* tpd)
{
    tpd->reset();
    m_empty_data.push_back(tpd);
}

triggerParseData* PluginsTrigger::getFreeTriggerData()
{
    triggerParseData *data = m_empty_data.pop_back();
    if (!data)
        data = new triggerParseData(this);
    return data;
}

/*void PluginsTrigger::run()
{
    m_trigger_func_ref.pushValue(L);
    Plugin *oldcp = _cp;
    _cp = p;
    {   // вызов деструктора PluginsParseData - обратная перекодировка строк
        PluginsParseData ppd(&m_parseData, &m_triggerParseData);
        luaT_pushobject(L, &ppd, LUAT_VIEWDATA);
        if (lua_pcall(L, 1, 0, 0))
        {
            //error
            if (luaT_check(L, 1, LUA_TSTRING))
            {
                pluginOut(lua_toerror(L));
            }
            else
            {
                pluginLog(L"неизвестная ошибка");
            }
            lua_settop(L, 0);
        }
    }
    _cp = oldcp;
    reset();
}*/

int trigger_create(lua_State *L)
{
    if (luaT_check(L, 2, LUA_TSTRING, LUA_TFUNCTION) ||
        luaT_check(L, 2, LUA_TTABLE, LUA_TFUNCTION))
    {
        PluginsTrigger *t = new PluginsTrigger();
        if (t->init(L, _cp))
        {
            _cp->triggers.push_back(t);
            luaT_pushobject(L, t, LUAT_TRIGGER);
            return 1;
        }
        delete t;
        return pluginInvArgsValues(L, L"createTrigger");
    }
    return pluginInvArgs(L, L"createTrigger");
}

int trigger_enable(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_TRIGGER))
    {
        PluginsTrigger *t = (PluginsTrigger*)luaT_toobject(L, 1);
        t->enable(true);
        return 0;
    }
    return pluginInvArgs(L, L"trigger:enable");
}

int trigger_disable(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_TRIGGER))
    {
        PluginsTrigger *t = (PluginsTrigger*)luaT_toobject(L, 1);
        t->enable(false);
        return 0;
    }
    return pluginInvArgs(L, L"trigger:disable");
}

int trigger_isEnabled(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_TRIGGER))
    {
        PluginsTrigger *t = (PluginsTrigger*)luaT_toobject(L, 1);
        lua_pushboolean(L, t->isEnabled() ? 1 : 0);
        return 1;
    }
    return pluginInvArgs(L, L"trigger:isEnabled");
}

int trigger_towatch(lua_State *L)
{
    if (luaT_check(L, 1, LUAT_TRIGGER))
    {
        lua_newtable(L);
        PluginsTrigger *t = (PluginsTrigger*)luaT_toobject(L, 1);
        lua_pushstring(L, "mode");
        lua_pushstring(L, t->isEnabled() ? "on" : "off");
        lua_settable(L, -3);
        int count = t->getLen();
        for (int i=0; i<count; ++i)
        {
            tstring k;
            t->getKey(i, &k);
            lua_pushinteger(L, i+1);
            luaT_pushwstring(L, k.c_str());
            lua_settable(L, -3);
        }
        return 1;
    }
    return 0;
}

void reg_mt_trigger(lua_State *L)
{
    lua_register(L, "createTrigger", trigger_create);
    luaL_newmetatable(L, "trigger");
    regFunction(L, "enable", trigger_enable);
    regFunction(L, "disable", trigger_disable);
    regFunction(L, "isEnabled", trigger_isEnabled);
    regFunction(L, "__towatch", trigger_towatch);
    regIndexMt(L);
    lua_pop(L, 1);
}
