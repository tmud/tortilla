#pragma once

class SoundPlayer
{
public:
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
            }
            else if (count == 2)
            {
            }
            else {
                incorrect_params = true;
            }
        }
                

    
    }

};