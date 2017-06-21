#pragma once
#include "editListBox.h"

class MappeZoneControl : public CDialogImpl<MappeZoneControl>
{
    CEditListBox m_list;
    RECT rc_list;    
    HWND m_parent;
    UINT m_msg;
    std::map<int, tstring> zones;
    typedef std::map<int, tstring>::iterator zones_iterator;
    typedef std::map<int, tstring>::const_iterator zones_const_iterator;
    int m_current_zone;
public:
    enum { IDD = IDD_MAPPER_ZONES };
    MappeZoneControl() : m_parent(NULL), m_msg(0), m_current_zone(-1)
    {
    }    
    void setNotifications(HWND wnd, UINT msg)
    {
        m_parent = wnd;
        m_msg = msg;
    }
    
    void addNewZone(const Rooms3dCube* zone)
    {
        if (!zone) {
           assert(false);
           return;
        }
        int id = zone->id();
        const tstring& name = zone->name();
        zones_iterator it = zones.find(id);
        if (it == zones.end())
        {
            zones[id] = name;
            m_list.AddItem(name.c_str());
        }
    }

    void setPosition(const Rooms3dCube* zone)
    {
        if (!zone) {
           assert(false);
           return;
        }
        addNewZone(zone);
        int id = zone->id();
        const tstring& name = zone->name();
        zones_iterator it = zones.find(id);
     
        int pos = m_list.FindItem(it->second.c_str());
        if (it->second != name)
        {                
            if (pos == -1)  {
               assert(false);
               return;
            }
            it->second = name;
            m_list.SetItemText(pos, name.c_str());
        }
        m_list.SelectItem(pos);
        m_current_zone = id;
    }

    int getCurrentZone() const {
        return m_current_zone;
    }

    //todo! name can be changed
    const tstring& getZoneName(int id) const {
        static tstring empty;
        zones_const_iterator it = zones.find(id);
        return (it != zones.end()) ? it->second : empty;
    }

    /*int selectZone(const tstring& zone, bool select)
    {
        MemoryBuffer b;
        int index = m_list.GetCurSel();
        if (index == -1)
        {
            for (int i=0,e=m_list.GetItemCount();i<e;++i)
            {
                int len = m_list.GetItemTextLen(i);
                b.alloc((len+1)*sizeof(tchar));
                tchar* buffer = (tchar*)b.getData();
                m_list.GetItemText(i, buffer, len);
            }
        }

        int selection = m_list.GetCurSel();
        int index = findZone(zone);
        if (index == -1)
        {
            ZoneParams zp;
            zone->getParams(&zp);
            index = m_list.GetItemCount();
            m_list.InsertItem(index, zp.name.c_str());
            zones.push_back(zone);
            index = m_list.GetItemCount()-1;
        }
        if (select) {
                m_list.SelectItem(index);
        } else {
            if (selection == -1)
                m_list.SelectItem(-1);
        }        
        return index;
    }*/

private:
    BEGIN_MSG_MAP(MapperToolbar)
		MESSAGE_HANDLER(WM_INITDIALOG, OnCreate)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        NOTIFY_CODE_HANDLER(EDLN_ITEMCHANGED, OnChanged)
        MESSAGE_HANDLER(WM_USER, OnSelectItem)
	END_MSG_MAP()

	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&bHandled)
	{
        m_list.SubclassWindow(GetDlgItem(IDC_LIST_ZONES));
        RECT rc;
        GetWindowRect(&rc);
        m_list.GetWindowRect(&rc_list);
        rc_list.top -= rc.top;
        m_list.SelectItem(-1);
        bHandled = FALSE;
        return 0;
	}

    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&bHandled)
	{
        RECT rc, rc2;
        GetClientRect(&rc);
        m_list.GetClientRect(&rc2);
        rc2.top = rc_list.top;
        rc2.right = rc.right;
        rc2.bottom = rc.bottom;
        m_list.MoveWindow(&rc2);
        bHandled = FALSE;
        return 0;
    }

    LRESULT OnSelectItem(UINT, WPARAM, LPARAM, BOOL&)
    {
        int item = m_list.GetCurSel();
        if (item != -1)
        {
            tstring name;
            getItemText(item, &name);
            item = -1;
            zones_iterator it = zones.begin(), it_end = zones.end();
            for(; it!=it_end;++it) {
                if (it->second == name) { item = it->first; break; }
            }
            m_current_zone = item;
            if (::IsWindow(m_parent))
                ::SendMessage(m_parent, m_msg, 0, 0);
        }
        return 0;
    }

    LRESULT OnChanged(int, LPNMHDR pnmh, BOOL&)
    {
        NMEDITLIST *list = (NMEDITLIST*)pnmh;
        int item = list->iIndex;
        tstring text;
        getItemText(item, &text);

        bool conflict = false;
        tstring text2;
        for (int i = 0, e = m_list.GetItemCount(); i < e; ++i)
        {
            if (i == item) continue;
            getItemText(i, &text2);
            if (!text.compare(text2)) { conflict = true; break; }
        }

        if (conflict)
        {
            MessageBox(L"Зона с таким именем уже существует!", L"Ошибка", MB_OK | MB_ICONERROR);
            tstring name = getZoneName(m_current_zone);
            assert(!name.empty());
            m_list.SetItemText(item, name.c_str());
        }
        else
        {
            zones[item] = text;
        }
        return 0;
    }

    void getItemText(int item, tstring* text)
    {
        int len = m_list.GetItemTextLen(item);
        int size = (len + 1) * sizeof(WCHAR);
        MemoryBuffer buffer(size);
        WCHAR *textbuffer = (WCHAR*)buffer.getData();
        m_list.GetItemText(item, textbuffer, len);
        text->assign(textbuffer);
    }
};
