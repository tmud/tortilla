#pragma once

float volume_ToFloat(int volume)
{
    float v = static_cast<float>(volume);
    v = v / 100.0f;
    if (v < 0) v = 0;
    if (v > 1) v = 1;
    return v;
}

int volume_toInt(float volume)
{
    int v = static_cast<int>(volume*100);
    if (v < 0) v = 0;
    if (v > 100) v = 100;
    return v;
}

int error(lua_State* L, const utf8* errmsg)
{
    u8string msg("BASS error: ");
    msg.append(errmsg);    
    lua_pushnil(L);
    lua_pushstring(L, msg.c_str());
    return 2;
}

int error_invargs(lua_State* L, const utf8* func)
{
     std::string msg("Invalid arguments in function: 'bass.");
     msg.append(func);
     msg.append("'");
     return error(L, msg.c_str());
}

int error(lua_State* L, const wchar_t* errmsg)
{
    return error(L, W2S(errmsg));
}
