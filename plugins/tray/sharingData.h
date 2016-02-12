#pragma once

#include "memoryBuffer.h"
#include "dataQueue.h"
#include <algorithm>

class Serialize
{
    DataQueue &d;
public:
    Serialize(DataQueue &dq) : d(dq) {}
    void write(const std::wstring& s)
    {
       int len = s.length();
       d.write(&len, sizeof(int));
       if (len > 0)
            d.write(s.c_str(), len*sizeof(wchar_t));
    }
    bool read(std::wstring &s)
    {
        int string_len = 0;
        if (!d.read(&string_len, sizeof(int)) || string_len < 0)
           return false;
        if (string_len > 0)
        {
            MemoryBuffer mb;
            int toread_bytes = string_len * sizeof(wchar_t);
            mb.alloc(toread_bytes);
            if (!d.read(mb.getData(), toread_bytes))
                return false;
            s.assign((wchar_t*)mb.getData(), string_len);
        }
        return true;
    }
    void write(int v) { d.write(&v, sizeof(int));  }
    bool read(int &v) { return d.read(&v, sizeof(int));  }
    void write(COLORREF c) { d.write(&c, sizeof(COLORREF)); }
    bool read(COLORREF &c) { return d.read(&c, sizeof(COLORREF)); }
    void write(const RECT& r) { d.write(&r, sizeof(RECT)); }
    bool read(RECT &r) { return d.read(&r, sizeof(RECT)); }
    void write(const MemoryBuffer &m) {
        int size = m.getSize();
        write(size);
        if (size > 0)
            d.write(m.getData(), size);
    }
    bool read(MemoryBuffer &m)
    {
        int len = 0;
        if (!d.read(&len, sizeof(int)) || len < 0)
            return false;
        m.alloc(len);
        if (len > 0)
        {
            if (!d.read(m.getData(), len))
                return false;
        }
        return true;
    }
};

struct SharingDataMessage
{
    SharingDataMessage() : background(0), text(0), showtime(0), windowpos(RECT()) {}
    std::vector<std::wstring> textlines;
    COLORREF background;
    COLORREF text;
    int showtime;
    RECT windowpos;

    void serialize(DataQueue &d) const
    {
        Serialize s(d);
        int strings = textlines.size();
        s.write(strings);
        for (int i=0; i<strings; ++i)
            s.write(textlines[i]);
        s.write(background);
        s.write(text);
        s.write(showtime);
        s.write(windowpos);
    }

    bool deserialize(DataQueue &d)
    {
        Serialize s(d);
        int strings = 0;
        if (!s.read(strings) || strings <= 0)
            return false;
        for (int i=0; i<strings; ++i)
        {
            std::wstring t;
            if (!s.read(t)) 
                return false;
            textlines.push_back(t);
        }
        if (!s.read(background) || !s.read(text) || !s.read(showtime) || !s.read(windowpos))
            return false;
        return true;
    }
};

struct SharingDataApps
{
    SharingDataApps() {}
    std::wstring appid;

};


enum SharingCommands {
    SC_NONE = 0, SC_MESSAGE, SC_REGTRAY, SC_UNREGTRAY
};

struct SharingCommand
{
    SharingCommand() : command(SC_NONE) {}
    int command;
    MemoryBuffer command_data;
    
    void serialize(DataQueue &d) const
    {
        Serialize s(d);
        s.write(command);
        s.write(command_data);
    }

    bool deserialize(DataQueue &d)
    {
        Serialize s(d);
        if (!s.read(command) || !s.read(command_data))
            return false;
        return true;
    }
};

struct AddMessageCommand : public SharingCommand
{
    AddMessageCommand(const SharingDataMessage& msg)
    {
        command = SC_MESSAGE;
        DataQueue d;
        d.setBufferSize(256);
        msg.serialize(d);
        command_data.alloc(d.getSize());
        memcpy(command_data.getData(), d.getData(), d.getSize());
    }
};

struct RegTrayCommand : public SharingCommand
{
    RegTrayCommand(const std::wstring& trayid)
    {
        command = SC_REGTRAY;
        DataQueue d;
        Serialize s(d);
        s.write(trayid);
        command_data.alloc(d.getSize());
        memcpy(command_data.getData(), d.getData(), d.getSize());
    }
};

struct UnregTrayCommand : public SharingCommand
{
    UnregTrayCommand(const std::wstring& trayid)
    {
        command = SC_UNREGTRAY;
        DataQueue d;
        Serialize s(d);
        s.write(trayid);
        command_data.alloc(d.getSize());
        memcpy(command_data.getData(), d.getData(), d.getSize());
    }
};

struct SharingData
{
    std::wstring main_tray_id;
    std::wstring new_tray_id;
    std::vector<SharingDataMessage*> windows;
    std::vector<SharingCommand*> commands;

    SharingData() {}
    ~SharingData() { clear(); }

    void serialize(DataQueue &d)
    {
        Serialize s(d);
        s.write(main_tray_id);
        s.write(new_tray_id);
        int wc = windows.size();
        s.write(wc);
        for (int i=0; i<wc; ++i)
            windows[i]->serialize(d);
        int cc = commands.size();
        s.write(cc);
        for (int i=0; i<cc; ++i)
            commands[i]->serialize(d);
    }

    bool deserialize(DataQueue &d)
    {
        Serialize s(d);
        clear();
        if (!s.read(main_tray_id) || !s.read(new_tray_id)) 
            return false;
        int wc = 0;
        if (!s.read(wc) || wc < 0)
            return false;
        for (int i=0; i<wc; ++i)
        {
            SharingDataMessage* wnd = new SharingDataMessage;
            if (!wnd->deserialize(d))
                { delete wnd; return false; }
            windows.push_back(wnd);
        }
        int cc = 0;
        if (!s.read(cc) || wc < 0)
            return false;
        for (int i=0; i<cc; ++i)
        {
            SharingCommand* cmd = new SharingCommand;
            if (!cmd->deserialize(d))
                { delete cmd; return false; }
            commands.push_back(cmd);
        }
        return true;
    }

private:
    void clear()
    {
        main_tray_id.clear();
        new_tray_id.clear();
        std::for_each(windows.begin(), windows.end(), [](SharingDataMessage* obj) { delete obj; });
        std::for_each(commands.begin(), commands.end(), [](SharingCommand* obj) { delete obj; });
    }
};
