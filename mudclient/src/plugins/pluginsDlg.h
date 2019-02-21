#pragma once
#include "plugin.h"

class PluginsDlg :  public CDialogImpl<PluginsDlg>, PropertyListCtrlHandler
{
     PluginsList* plugins_list;
     std::vector<int> state;
     PropertyListCtrl m_list;
     CButton m_onoff;
     CButton m_prior_up, m_prior_down;
     CEdit m_description;

public:
     enum { IDD = IDD_PLUGINS};
     PluginsDlg(PluginsList* plugins) : plugins_list(plugins)
     {
         int size = plugins->size();
         state.resize(size);
         for (int i = 0; i < size; ++i)
         {
             Plugin *p = plugins->at(i);
             state[i] = p->state() ? 1 : 0;
         }
     }

     bool getNewState(int plugin) const
     {
         return state[plugin] ? true : false;
     }

private:
    BEGIN_MSG_MAP(PluginsDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
        COMMAND_ID_HANDLER(IDC_BUTTON_ONOFF, OnOnOff)
        COMMAND_ID_HANDLER(IDC_BUTTON_PLUGINDOWN, OnPluginDown)
        COMMAND_ID_HANDLER(IDC_BUTTON_PLUGINUP, OnPluginUp)
        NOTIFY_HANDLER(IDC_LIST, LVN_ITEMCHANGED, OnListItemChanged)
        //NOTIFY_HANDLER(IDC_LIST, NM_SETFOCUS, OnListItemChanged)
        NOTIFY_HANDLER(IDC_LIST, NM_DBLCLK, OnItemDblClick)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    bool drawPropertyListItem(PropertyListItemData *pData)
    {
        if (pData->subitem == 3)
        {
            Plugin *p = plugins_list->at(pData->item);
            pData->bkg = state[pData->item] ? RGB(0, 255, 0) : RGB(255, 0, 0);
            pData->text = RGB(0,0,0);
            return true;
        }
        return false;
    }

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
        m_onoff.Attach(GetDlgItem(IDC_BUTTON_ONOFF));
        m_prior_down.Attach(GetDlgItem(IDC_BUTTON_PLUGINDOWN));
        m_prior_up.Attach(GetDlgItem(IDC_BUTTON_PLUGINUP));
        m_description.Attach(GetDlgItem(IDC_EDIT_PLUGIN_DESC));

        m_list.Attach(GetDlgItem(IDC_LIST));
        m_list.setHandler(this);
        m_list.addColumn(L"Название", 50);
        m_list.addColumn(L"Версия", 15);
        m_list.addColumn(L"Файл", 20);
        m_list.addColumn(L"Состояние", 15);
        m_list.SetExtendedListViewStyle( m_list.GetExtendedListViewStyle()|LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
        m_list.supportDoubleClick();

        for (int i=0,e=plugins_list->size(); i<e; ++i)
        {
            Plugin *p = plugins_list->at(i);
            int pos = m_list.GetItemCount();
            m_list.addItem(pos, 0, p->get(Plugin::NAME));
            m_list.addItem(pos, 1, p->get(Plugin::VERSION));
            m_list.addItem(pos, 2, p->get(Plugin::FILE));
            m_list.addItem(pos, 3, state[i] ? L"Вкл" : L"Выкл");
        }

        CenterWindow(GetParent());

        m_onoff.EnableWindow(FALSE);
        m_prior_down.EnableWindow(FALSE);
        m_prior_up.EnableWindow(FALSE);

        m_list.SetFocus();
        return 0;
    }

    LRESULT OnListItemChanged(int , LPNMHDR , BOOL&)
    {
        int item_selected = m_list.GetSelectedIndex();
        if (item_selected == -1)
        {
            m_onoff.EnableWindow(FALSE);
            m_prior_down.EnableWindow(FALSE);
            m_prior_up.EnableWindow(FALSE);
            m_description.SetWindowText(L"");
        }
        else
        {
            int last = m_list.GetItemCount() - 1;
            m_onoff.EnableWindow(TRUE);
            if (item_selected != 0)
                m_prior_up.EnableWindow(TRUE);
            if (item_selected != last)
                m_prior_down.EnableWindow(TRUE);
            Plugin *p = plugins_list->at(item_selected);
            m_description.SetWindowText(p->get(Plugin::DESCRIPTION));
        }
        return 0;
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
            int newstate = state[item_selected] ? 0 : 1;
            state[item_selected] = newstate;
            m_list.setItem(item_selected, 3, newstate ? L"Вкл" : L"Выкл");
        }
    }

    LRESULT OnPluginDown(WORD, WORD wID, HWND, BOOL&)
    {
        int item_selected = m_list.GetSelectedIndex();
        int last = m_list.GetItemCount() - 1;
        if (item_selected != last) {
            m_list.SelectItem(-1);
            swapItems(item_selected, item_selected+1);
            m_list.SelectItem(item_selected+1);
            SetFocus();
        }
        return 0;
    }

    LRESULT OnPluginUp(WORD, WORD wID, HWND, BOOL&)
    {
        int item_selected = m_list.GetSelectedIndex();
        if (item_selected != 0) {
            m_list.SelectItem(-1);
            swapItems(item_selected, item_selected-1);
            m_list.SelectItem(item_selected-1);
            SetFocus();
        }
        return 0;
    }

    LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
	{
		EndDialog(wID);
		return 0;
	}

    void swapItems(int index1, int index2)
    {
        Plugin* p1 = plugins_list->at(index1);
        Plugin* p2 = plugins_list->at(index2);
        m_list.setItem(index1, 0, p2->get(Plugin::NAME));
        m_list.setItem(index1, 1, p2->get(Plugin::VERSION));
        m_list.setItem(index1, 2, p2->get(Plugin::FILE));
        m_list.setItem(index1, 3, state[index2] ? L"Вкл" : L"Выкл");
        m_list.setItem(index2, 0, p1->get(Plugin::NAME));
        m_list.setItem(index2, 1, p1->get(Plugin::VERSION));
        m_list.setItem(index2, 2, p1->get(Plugin::FILE));
        m_list.setItem(index2, 3, state[index1] ? L"Вкл" : L"Выкл");
        plugins_list->at(index1) = p2;
        plugins_list->at(index2) = p1;
        int t = state[index1]; state[index1] = state[index2]; state[index2] = t;
    }
};
