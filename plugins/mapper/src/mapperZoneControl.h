#pragma once
#include "editListBox.h"

class MappeZoneControl : public CDialogImpl<MappeZoneControl>
{
    CEditListBox m_list;
    RECT rc_list;    
    HWND m_parent;
    UINT m_msg;
    struct zonedata {
        int id;
        tstring name;
    };
    std::vector<zonedata> zones;
    typedef std::vector<zonedata>::iterator zones_iterator;
    typedef std::vector<zonedata>::const_iterator zones_const_iterator;
public:
    enum { IDD = IDD_MAPPER_ZONES };
    MappeZoneControl() : m_parent(NULL), m_msg(0)
    {
    }    
    void setNotifications(HWND wnd, UINT msg)
    {
        m_parent = wnd;
        m_msg = msg;
    }

    void updateList(const Rooms3dCubeList& zoneslist)
    {
        tstring currentZoneName;
        getItemText(m_list.GetCurSel(), &currentZoneName);

        Rooms3dCubeList z(zoneslist.begin(), zoneslist.end());
        std::sort(z.begin(), z.end(), [](Rooms3dCube *z1, Rooms3dCube* z2) { return z1->name() <  z2->name(); } );
        zones.clear();
        for (Rooms3dCube* zone : z) {
            zonedata zd; zd.id = zone->id(); zd.name = zone->name();
            zones.push_back(zd);
        }

        int elements = m_list.GetItemCount();
        int zonescount = zones.size();

        int count = min(elements, zonescount);
        for (int i=0; i<count; ++i) {
            const tstring &item = zones[i].name;
            tstring current;
            getItemText(i, &current);            
            if (item.compare(current)) {
                m_list.SetItemText(i, item.c_str());
            }
        }
        int total = max(elements, zonescount) - count;
        if (total > 0) {
            if (elements < zonescount) {
                for (int i=0; i< total; ++i) {
                    int index = i + count;
                    const tstring &item = zones[index].name;
                    m_list.AddItem(item.c_str());
                }
            } else {
                for (int i=count; i< total; ++i) {
                    m_list.DeleteItem(count);
                }
           }
       }
       if (currentZoneName.empty())
           return;
       for (int i=0,e=zones.size();i<e;++i) {
            if (zones[i].name == currentZoneName) {
                m_list.SelectItem(i);
                break;
            }
       }
    }

    void setCurrentZone(const Rooms3dCube* zone)
    {
        if (!zone) {
           m_list.SelectItem(-1);
           return;
        }
        int index = findZone(zone->id());
        assert(index != -1);
        m_list.SelectItem(index);
    }

    int getCurrentZone() const {
       int pos = m_list.GetCurSel();
       return (pos >= 0) ? zones[pos].id : -1;
    }

    const tstring& getZoneName(int id) const
    {
        static tstring empty;
        int index = findZone(id);
        return (index != -1) ? zones[index].name : empty;
    }

    void deleteAllZones() {
        m_list.DeleteAllItems();
        zones.clear();
    }
private:
    int findZone(int id) const {
        zones_const_iterator it = std::find_if(zones.begin(), zones.end(), [&](const zonedata& zd) { return (zd.id == id); });
        return (it != zones.end()) ? it-zones.begin() : -1;
    }

private:
    BEGIN_MSG_MAP(MappeZoneControl)
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
            tstring name = zones[item].name;
            assert(!name.empty());
            m_list.SetItemText(item, name.c_str());
        }
        else
        {
            zones[item].name = text;
        }
        return 0;
    }

    void getItemText(int item, tstring* text)
    {
        if (item == -1) {
            text->clear();
            return;
        }
        int len = m_list.GetItemTextLen(item);
        if (len > 31)
        {
            int size = (len + 1) * sizeof(tchar);
            MemoryBuffer buffer(size);
            tchar *textbuffer = (tchar*)buffer.getData();
            m_list.GetItemText(item, textbuffer, len);
            text->assign(textbuffer);
            return;
        }
        tchar buffer[32];
        m_list.GetItemText(item, buffer, len);
        text->assign(buffer);
    }
};
