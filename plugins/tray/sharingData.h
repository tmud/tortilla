#pragma once

#include "memoryBuffer.h"
#include "dataQueue.h"

class Serialize
{
    DataQueue &d;
public:
    Serialize(DataQueue &dq) : d(dq) {}
    void write(const tstring& s)
    {
       int len = s.length();
       d.write(&len, sizeof(int));
       if (len > 0)
            d.write(s.c_str(), len*sizeof(tchar));
    }
    bool read(tstring &s)
    {
        int string_len = 0;
        if (!d.read(&string_len, sizeof(int)) || string_len < 0)
           return false;
        if (string_len > 0)
        {
            MemoryBuffer mb;
            int toread_bytes = string_len * sizeof(tchar);
            mb.alloc(toread_bytes);
            if (!d.read(mb.getData(), toread_bytes))
                return false;
            s.assign((tchar*)mb.getData(), string_len);
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
    std::vector<tstring> textlines;
    COLORREF background;
    COLORREF text;
    int showtime;
    RECT windowpos;

    void serialize(Serialize &s) const
    {
        int strings = textlines.size();
        s.write(strings);
        for (int i=0; i<strings; ++i)
            s.write(textlines[i]);
        s.write(background);
        s.write(text);
        s.write(showtime);
        s.write(windowpos);
    }

    bool deserialize(Serialize &s)
    {
        int strings = 0;
        if (!s.read(strings) || strings <= 0)
            return false;
        for (int i=0; i<strings; ++i)
        {
            tstring t;
            if (!s.read(t)) 
                return false;
            textlines.push_back(t);
        }
        if (!s.read(background) || !s.read(text) || !s.read(showtime) || !s.read(windowpos))
            return false;
        return true;
    }
};

enum SharingCommands {
    SC_NONE = 0, SC_ADDMESSAGE, SC_REGTRAY, SC_UNREGTRAY
};

struct SharingCommand
{
    SharingCommand() : command(SC_NONE) {}
    int command;
    MemoryBuffer command_data;
    
    void serialize(Serialize &s) const
    {
        s.write(command);
        s.write(command_data);
    }

    bool deserialize(Serialize &s)
    {
        if (!s.read(command) || !s.read(command_data))
            return false;
        return true;
    }
};

struct AddMessageCommand : public SharingCommand
{
    AddMessageCommand(const SharingDataMessage& msg)
    {
        command = SC_ADDMESSAGE;
        DataQueue d;
        d.setBufferSize(256);
        Serialize s(d);
        msg.serialize(s);
        command_data.alloc(d.getSize());
        memcpy(command_data.getData(), d.getData(), d.getSize());
    }
};

struct RegTrayCommand : public SharingCommand
{
    RegTrayCommand(const tstring& trayid)
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
    UnregTrayCommand(const tstring& trayid)
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
    tstring main_tray_id;
    tstring new_tray_id;
    std::vector<SharingDataMessage*> windows;
    std::vector<SharingCommand*> commands;

    SharingData() {}
    ~SharingData() { clear(); }

    void serialize(Serialize &s)
    {
        s.write(main_tray_id);
        s.write(new_tray_id);
        int wc = windows.size();
        s.write(wc);
        for (int i=0; i<wc; ++i)
            windows[i]->serialize(s);
        int cc = commands.size();
        s.write(cc);
        for (int i=0; i<cc; ++i)
            commands[i]->serialize(s);
    }

    bool deserialize(Serialize &s)
    {
        clear();
        if (!s.read(main_tray_id) || !s.read(new_tray_id)) 
            return false;
        int wc = 0;
        if (!s.read(wc) || wc < 0)
            return false;
        for (int i=0; i<wc; ++i)
        {
            SharingDataMessage* wnd = new SharingDataMessage;
            if (!wnd->deserialize(s))
                { delete wnd; return false; }
            windows.push_back(wnd);
        }
        int cc = 0;
        if (!s.read(cc) || wc < 0)
            return false;
        for (int i=0; i<cc; ++i)
        {
            SharingCommand* cmd = new SharingCommand;
            if (!cmd->deserialize(s))
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
