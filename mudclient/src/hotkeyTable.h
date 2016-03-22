#pragma once

class HotkeyTable 
{
   struct keydata
   {
      WCHAR* name;
      unsigned char scancode;
      unsigned char extflag;      
   };  
   keydata *keys;
   int keydata_len;

public:
    HotkeyTable() 
    {
        keydata k[] = {
            {L"INS", 82 , 1},
            {L"HOME", 71 , 1},
            {L"PGUP", 73 ,1},
            {L"DEL", 83 ,1},
            {L"END", 79 ,1},
            {L"PGDN", 81 ,1},
            {L"UP", 72 ,1},
            {L"LEFT",  75 ,1},
            {L"DOWN", 80 ,1}, 
            {L"RIGHT", 77 , 1},
            {L"DIV", 53 ,1},
            {L"MUL", 55 , 0},
            {L"MIN", 74 ,0},
            {L"NUM7", 71 ,0},
            {L"NUM8", 72 ,0},
            {L"NUM9", 73 ,0},
            {L"NUM4",75 ,0},
            {L"NUM5", 76 ,0},
            {L"NUM6",77 ,0},
            {L"NUM1",79 ,0},
            {L"NUM2", 80 ,0},
            {L"NUM3", 81 ,0},
            {L"NUM0",82 ,0},
            {L"NUMDEL", 83 ,0},
            {L"ADD", 78 ,0},
            {L"RETURN", 28 , 1},
            {L"BACK", 14 ,0},
            {L"TAB", 15 , 0},
            {L"ENTER", 28 , 0},
            {L"ESC",1 ,0},
            {L"SPACE",57 ,0},
            {L":", 39 ,0},
            {L"=",13 , 0},
            {L",", 51 , 0},
            {L"-", 12 ,0},
            {L".", 52 ,0},
            {L"/",53 , 0},
            {L"`", 41 , 0},
            {L"[",26 ,0},
            {L"\\", 43 ,0},
            {L"\\", 86 ,0},
            {L"]", 27 ,0},
            {L"'", 40 , 0},
            {L"", 0 ,0}
        };
        
        int i=0;
        for (; k[i].scancode != 0; ++i) {}
        keydata_len = i;
        keys = new keydata[keydata_len];
        i=0;
        for (; k[i].scancode != 0; ++i) { keys[i] = k[i]; }

    }
    ~HotkeyTable() 
    {
        delete []keys;
    }

    void recognize(WPARAM wparam, LPARAM lparam, tstring* key) const
    {
        WORD flags = HIWORD(lparam);
        int scancode = LOBYTE(flags);
        int ext = HIBYTE(flags) & 0x1;
        int vk = wparam;
        bool ctrl = (GetKeyState(VK_CONTROL) < 0) ? true : false;
        bool alt = (GetKeyState(VK_MENU) < 0) ? true : false;
        bool shift = (GetKeyState(VK_SHIFT) < 0) ? true : false;

        key->clear();
        if (ctrl) key->append(L"Ctrl+");
        if (alt) key->append(L"Alt+");
        if (shift) key->append(L"Shift+");

        for (int i=0; i<keydata_len; ++i)
        {
            if (keys[i].scancode == scancode && keys[i].extflag == ext)
            {
                key->append(keys[i].name);
                return;
            }
        }

        if (ext) { key->clear(); return; }  // key not recognized
        
        if(vk >= '0' && vk <= '9' || vk >= 'A' && vk <= 'Z')
        {
            // block simple label key without ctrl etc!!! (block typing of some letters)
            if (ctrl || alt || shift)
            {
                WCHAR x[2] = { vk, 0 };
                key->append(x); 
            }
            else
            {
                key->clear();
            }
        }
        else if(vk >= VK_F1 && vk <= VK_F24) 
        {
            key->append(L"F");
            WCHAR buffer[8];
            _itot(vk-VK_F1+1, buffer, 10);
            key->append(buffer);
        }
        else
        {
            key->clear();
        }
    }

    bool isKey(const tstring& key, tstring *normalized) const
    {
        // check ctrl+, alt+, shift+
        tstring tmp(key);
        tstring_toupper(&tmp);
        int pos1 = tmp.find(L"CTRL+");
        int pos2 = tmp.find(L"ALT+");
        int pos3 = tmp.find(L"SHIFT+");
        int last = max(pos1, pos2);
        last = max(last, pos3);
        if (last >= 0)
        {
            last = tmp.find(L"+", last);
            tmp = tmp.substr(last+1);
        }

        if (tmp.empty())
            return false;

        if (normalized)
        {
            if (pos1 >= 0)
                normalized->append(L"Ctrl+");
            if (pos2 >= 0)
                normalized->append(L"Alt+");
            if (pos3 >= 0)
                normalized->append(L"Shift+");
        }

        // check table
        for (int i=0; i<keydata_len; ++i)
        {
            if (keys[i].name == tmp)
            {
                if (normalized)
                    normalized->append(tmp);
                return true;
            }
        }

        // check numbers and letters
        if (tmp.size() == 1)
        {
            WCHAR vk = tmp.at(0);
            if(vk >= L'0' && vk <= L'9' || vk >= L'A' && vk <= L'Z')
            {
                if (last >= 0)
                {
                    if (normalized)
                        normalized->append(tmp);
                    return true;
                }
                return false;
            }
        }

        // check Fxx
        if (tmp.at(0) == L'F')
        {
            tstring f(tmp.substr(1));
            if (wcsspn(f.c_str(), L"0123456789") == f.length())
            {
                int id = _wtoi(f.c_str());
                if (id >= 1 && id <= 24)
                {
                    if (normalized)
                        normalized->append(tmp);                    
                    return true;
                }
            }
        }
        return false;
    }
};
