#pragma once
#include "plugin.h"

struct PluginStateToDlg
{
    Plugin *p;
    PluginData::State state;
};
typedef std::vector<PluginStateToDlg> PluginDlgParametes;

class PluginsDlg :  public CDialogImpl<PluginsDlg>, PropertyListCtrlHandler
{
     PluginDlgParametes* plugins_list;
     PropertyListCtrl m_list;
     CButton m_onoff, m_hide, m_prior_up, m_prior_down;
     CEdit m_description;
     CButton m_show_hidden;
     std::vector<int> m_hidden_indexes;

public:
     enum { IDD = IDD_PLUGINS};
     PluginsDlg(PluginDlgParametes* plugins) : plugins_list(plugins)
     {
     }

private:
    BEGIN_MSG_MAP(PluginsDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
        COMMAND_ID_HANDLER(IDC_BUTTON_ONOFF, OnOnOff)
        COMMAND_ID_HANDLER(IDC_BUTTON_HIDE_PLUGIN, OnHidePlugin)
        COMMAND_ID_HANDLER(IDC_BUTTON_PLUGINDOWN, OnPluginDown)
        COMMAND_ID_HANDLER(IDC_BUTTON_PLUGINUP, OnPluginUp)
        NOTIFY_HANDLER(IDC_LIST, LVN_ITEMCHANGED, OnListItemChanged)
        //NOTIFY_HANDLER(IDC_LIST, NM_SETFOCUS, OnListItemChanged)
        NOTIFY_HANDLER(IDC_LIST, NM_DBLCLK, OnItemDblClick)
        COMMAND_ID_HANDLER(IDC_CHECK_SHOW_HIDDEN, OnShowHidden)
        REFLECT_NOTIFICATIONS()
    END_MSG_MAP()

    bool drawPropertyListItem(PropertyListItemData *pData)
    {
        if (pData->subitem == 1)
        {
            PluginStateToDlg &pd = getRef(pData->item);
            pData->bkg = getStateColor(pd.state);
            pData->text = RGB(0,0,0);
            return true;
        }
        return false;
    }

    COLORREF getStateColor(PluginData::State state)
    {
        if (state == PluginData::PDS_ON)
            return RGB(0, 240, 0);
        if (state == PluginData::PDS_OFF)
            return RGB(240, 0, 0);
        if (state == PluginData::PDS_HIDDEN)
            return RGB(128, 128, 128);
        return RGB(255, 0, 255);
    }

    const tchar* getStateText(PluginData::State state)
    {
        if (state == PluginData::PDS_ON)
            return L"Вкл";
        if (state == PluginData::PDS_OFF)
            return L"Выкл";
        if (state == PluginData::PDS_HIDDEN)
            return L"-";
        return L"?";
    }

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
	{
        m_onoff.Attach(GetDlgItem(IDC_BUTTON_ONOFF));
        m_hide.Attach(GetDlgItem(IDC_BUTTON_HIDE_PLUGIN));
        m_prior_down.Attach(GetDlgItem(IDC_BUTTON_PLUGINDOWN));
        m_prior_up.Attach(GetDlgItem(IDC_BUTTON_PLUGINUP));
        m_description.Attach(GetDlgItem(IDC_EDIT_PLUGIN_DESC));
        m_show_hidden.Attach(GetDlgItem(IDC_CHECK_SHOW_HIDDEN));

        m_list.Attach(GetDlgItem(IDC_LIST));
        m_list.setHandler(this);
        m_list.addColumn(L"Название", 85);
        m_list.addColumn(L"", 15);
        m_list.SetExtendedListViewStyle( m_list.GetExtendedListViewStyle()|LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT);
        m_list.supportDoubleClick();

        UpdateList();
        
        CenterWindow(GetParent());
        m_onoff.EnableWindow(FALSE);
        m_prior_down.EnableWindow(FALSE);
        m_prior_up.EnableWindow(FALSE);
        m_list.SetFocus();
        return 0;
    }

    LRESULT OnShowHidden(WORD, WORD, HWND, BOOL&)
    {
        UpdateList();
        return 0;
    }

    void UpdateList()
    {
        bool show_hidden = (m_show_hidden.GetCheck() == BST_CHECKED);
        m_hidden_indexes.clear();
        m_list.DeleteAllItems();
        for (int i = 0, e = plugins_list->size(); i < e; ++i)
        {
            PluginStateToDlg &pd = plugins_list->at(i);
            if (!show_hidden && pd.state == PluginData::PDS_HIDDEN)
              continue;
            m_hidden_indexes.push_back(i);
            Plugin *p = pd.p;
            int pos = m_list.GetItemCount();
            m_list.addItem(pos, 0, p->get(Plugin::NAME));
            m_list.addItem(pos, 1, getStateText(pd.state));
        }
    }

    LRESULT OnListItemChanged(int , LPNMHDR , BOOL&)
    {
        int item_selected = m_list.GetSelectedIndex();
        if (item_selected == -1)
        {
            m_onoff.EnableWindow(FALSE);
            m_hide.EnableWindow(FALSE);
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
            PluginStateToDlg &pd = getRef(item_selected);
            if (pd.state == PluginData::PDS_HIDDEN)
                m_hide.EnableWindow(FALSE);
            else
                m_hide.EnableWindow(TRUE);
            Plugin *p = pd.p;
            tstring text(L"Название: "); text.append(p->get(Plugin::NAME));
            text.append(L"\r\nФайл: "); text.append(p->get(Plugin::FILE));
            text.append(L"\r\nВерсия: "); text.append(p->get(Plugin::VERSION));
            text.append(L"\r\n\r\n"); text.append(p->get(Plugin::DESCRIPTION));
            m_description.SetWindowText(text.c_str());
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

    LRESULT OnHidePlugin(WORD, WORD wID, HWND, BOOL&)
    {
        int item_selected = m_list.GetSelectedIndex();
        if (item_selected != -1)
        {
            PluginStateToDlg &pd = getRef(item_selected);
            pd.state = PluginData::PDS_HIDDEN;
            m_list.setItem(item_selected, 1, getStateText(pd.state));
            m_hide.EnableWindow(FALSE);
        }
        return 0;
    }

    void switchItemOnOff()
    {
        int item_selected = m_list.GetSelectedIndex();
        if (item_selected != -1) 
        {
            PluginStateToDlg &pd = getRef(item_selected);
            PluginData::State newstate = (pd.state==PluginData::PDS_ON) ? PluginData::PDS_OFF : PluginData::PDS_ON;
            pd.state = newstate;
            m_list.setItem(item_selected, 1, getStateText(pd.state));
            m_hide.EnableWindow(TRUE);
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
        PluginStateToDlg pd1 = getState(index1);
        PluginStateToDlg pd2 = getState(index2);
        Plugin* p1 = pd1.p;
        Plugin* p2 = pd2.p;
        m_list.setItem(index1, 0, p2->get(Plugin::NAME));
        m_list.setItem(index1, 1, getStateText(pd2.state));
        m_list.setItem(index2, 0, p1->get(Plugin::NAME));
        m_list.setItem(index2, 1, getStateText(pd1.state));
        setState(index1, pd2);
        setState(index2, pd1);
    }

    PluginStateToDlg& getRef(int index)
    {
        return plugins_list->at( convertIndex(index) );
    }

    PluginStateToDlg getState(int index)
    {
        return plugins_list->at( convertIndex(index) );
    }

    void setState(int index, const PluginStateToDlg& state)
    {
        PluginStateToDlg& r = getRef(index);
        r.p = state.p;
        r.state = state.state;
    }

    int convertIndex(int index)
    {
        return m_hidden_indexes[index];
    }
};
