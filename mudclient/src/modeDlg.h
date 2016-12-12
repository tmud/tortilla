#pragma once

class ModeDlg : public CDialogImpl<ModeDlg>, PropertyListCtrlHandler
{
    PropertyListCtrl m_list;
    PropertiesData *m_pdata;
    std::vector<int> m_modes;
public:
    enum { IDD = IDD_MODE };
    ModeDlg(PropertiesData *pd) : m_pdata(pd) {}
private:
    BEGIN_MSG_MAP(ModeDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
      COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
      COMMAND_ID_HANDLER(IDC_BUTTON_ONOFF, OnOnOff)    
      NOTIFY_HANDLER(IDC_LIST_COMPONENTS, NM_DBLCLK, OnItemDblClick)
      REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    void add(const tstring& name, int mode) {
       int  pos = m_list.GetItemCount();
       m_list.addItem(pos, 0, name);
       m_list.addItem(pos, 1, mode ? L"Вкл" : L"Выкл");
       m_modes.push_back(mode);
    }

    void save(int item, int* out)
    {
        int count = m_modes.size();
        if (item >=0 && item < count) {
            *out = (m_modes[item] == 0) ? 0 : 1;
        } else {
            assert(false);
        }
    }

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
       m_list.Attach(GetDlgItem(IDC_LIST_COMPONENTS));
       m_list.setHandler(this);
       m_list.addColumn(L"Компонент", 85);
       m_list.addColumn(L"Режим", 15);
       m_list.SetExtendedListViewStyle( m_list.GetExtendedListViewStyle()|LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
       m_list.supportDoubleClick();

       PropertiesData::working_mode &m = m_pdata->mode;
       add(L"Триггеры (actions)", m.actions);
       add(L"Макросы (aliases)", m.aliases);
       add(L"Замены (subs)", m.subs);
       add(L"Горячие клавишы (hotkeys)", m.hotkeys);
       add(L"Подсветки (highlights)", m.highlights);
       add(L"Антизамены (antisubs)", m.antisubs);
       add(L"Фильтры (gags)", m.gags);
       add(L"Плагины (plugins), триггеры, препост обработка", m.plugins);

       CenterWindow(GetParent());
       return 0;
    }

    LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
        if (wID == IDOK) {
            int i = 0;
            PropertiesData::working_mode &m = m_pdata->mode;
            save(i++, &m.actions);
            save(i++, &m.aliases);
            save(i++, &m.subs);
            save(i++, &m.hotkeys);
            save(i++, &m.highlights);
            save(i++, &m.antisubs);
            save(i++, &m.gags);
            save(i++, &m.plugins);
        }
		EndDialog(wID);
		return 0;
	}
    
    bool drawPropertyListItem(PropertyListItemData *pData)
    {
        if (pData->subitem == 1)
        {
            int item = pData->item;
            int count = m_modes.size();
            if (item >=0 && item < count) {
              pData->bkg = m_modes[item] ? RGB(0, 255, 0) : RGB(255, 0, 0);
              pData->text = RGB(0,0,0);            
              return true;
            }
        }
        return false;
    }

    LRESULT OnItemDblClick(int, LPNMHDR, BOOL&)
    {
        switchItemOnOff();
        return 0;
    }

    LRESULT OnOnOff(WORD, WORD wID, HWND, BOOL&)
    {
        switchItemOnOff();
        return 0;
    }

    void switchItemOnOff()
    {
        int item_selected = m_list.GetSelectedIndex();
        if (item_selected != -1) {
            int count = m_modes.size();
            if (item_selected >=0 && item_selected < count) {
            int newstate = m_modes[item_selected] ? 0 : 1;
            m_modes[item_selected] = newstate;
            m_list.setItem(item_selected, 1, newstate ? L"Вкл" : L"Выкл");
        }}
    }
};
