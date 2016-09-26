#pragma once

#include "editListBox.h"

class MappeZoneControl : public CDialogImpl<MappeZoneControl>
{
    CEditListBox m_list;
    RECT rc_list;
    std::vector<Zone*> zones;
    HWND m_parent;
    UINT m_msg;

public:
    enum { IDD = IDD_MAPPER_ZONES };
    MappeZoneControl() : m_parent(NULL), m_msg(0)
    {
    }
    
  /*  void roomChanged(const ViewMapPosition& pos)
    {
        if (!pos.level)
        {
            m_list.SelectItem(-1);
            return;
        }

        Zone *newzone = pos.level->getZone();
        int current_zone = m_list.GetCurSel();
        if (current_zone == -1 || zones[current_zone] != newzone)
        {
            int index = -1;
            for (int i = 0, e = zones.size(); i < e; ++i)
                 { if (zones[i] == newzone) { index = i; break; }}
            m_list.SelectItem(index);
        }
    }
    
    int addNewZone(Zone *zone)
    {
        int selection = m_list.GetCurSel();
        int index = findZone(zone);
        if (index == -1)
        {
            ZoneParams zp;
            zone->getParams(&zp);
            index = m_list.GetItemCount();
            m_list.InsertItem(index, zp.name.c_str());
            zones.push_back(zone);
        }
        if (selection == -1)
            m_list.SelectItem(-1);
        return index;
    }

    Zone* getCurrentZone()
    {
        int id = m_list.GetCurSel();
        if (id == -1) return NULL;
        return zones[id];
    }

    void setNotifications(HWND wnd, UINT msg)
    {
        m_parent = wnd;
        m_msg = msg;
    }

private:
    int findZone(Zone *zone) 
    {
        ZoneParams zp;
        zone->getParams(&zp);
        for (int i = 0, e = m_list.GetItemCount(); i<e; ++i)
        {
            tstring text;
            getItemText(i, &text);
            if (!zp.name.compare(text))
                return i;
        }
        return -1;
    }

    void getItemText(int item, tstring* text)
    {
        int len = m_list.GetItemTextLen(item);
        int size = (len + 1) * sizeof(WCHAR);
        MemoryBuffer buffer(size);
        WCHAR *textbuffer = (WCHAR*)buffer.getData();
        m_list.GetItemText(item, textbuffer, len);
        text->assign(textbuffer);
    }*/

private:
    BEGIN_MSG_MAP(MapperToolbar)
		/*MESSAGE_HANDLER(WM_INITDIALOG, OnCreate)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        NOTIFY_CODE_HANDLER(EDLN_ITEMCHANGED, OnChanged)
        MESSAGE_HANDLER(WM_USER, OnSelectItem)*/
	END_MSG_MAP()

	/*LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&bHandled)
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
        if (::IsWindow(m_parent))
            ::SendMessage(m_parent, m_msg, 0, 0);
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
            ZoneParams zp;
            zones[item]->getParams(&zp);
            m_list.SetItemText(item, zp.name.c_str());
        }
        else
        {
            zones[item]->setName(text);
        }
        return 0;
    }*/
};
