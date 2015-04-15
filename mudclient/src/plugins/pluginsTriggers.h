#pragma once

class PluginsTriggerString
{
public:
    

};

class PluginsTrigger
{
public:
    PluginsTrigger();
    ~PluginsTrigger();
    bool init(lua_State *pL);

private:
    bool reg_trigger();

private:
    lua_State *L;
    u8string m_trigger;
    int m_tigger_func_index;
};
