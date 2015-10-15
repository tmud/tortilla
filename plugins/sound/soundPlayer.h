#pragma once

class SoundPlayer
{
    lua_State *L;
public:
    SoundPlayer(lua_State* l) : L(l) {} 

    void runCommand(const std::vector<tstring>& params, tstring* error)
    {
        if (params.empty())
            { error->assign(L"Не заданы параметры"); return; }

        bool incorrect_params = false;

        int count = params.size();
        const tstring &cmd = params[0];
        if (cmd == L"play")
        {
        }
        else if (cmd == L"volume")
        {
            if (count == 1)
            {

                print(L"Громкость");
            }
            else if (count == 2)
            {
            }
            else {
                incorrect_params = true;
            }
        }
                

    
    }

private:
    void print(const wchar_t* message)
    {
        tstring text(L"[sound] ");
        text.append(message);
        base::print(L, TW2U(text.c_str()) );    
    }

    int getVolume() {
        //run(L"getVolume", )
    }

    void run(const wchar_t* function, wchar_t* params, ...)
    {
        //luaT_run(L, TW2U(function), TW2U(params),  )    
    
    }

};

/*

--local n = bass.load('d:\\Mud\\1.mp3')
--bass.play(n)
--local m = bass.load('d:\\Mud\\2.mp3')
--bass.play(m)
--local x = bass.loadSample('d:\\sample.wav')
--bass.play(x)

--local v = bass.getVolume()
--system.msgbox(false)
--bass.setVolume(50)

*/