#pragma once

class PropertyCommon : public CDialogImpl<PropertyCommon>
{
    PropertiesData *propData;
    CBevelLine m_bl1;
    CBevelLine m_bl2;
    CEdit m_history_size;
    CEdit m_cmd_size;
    CButton m_show_system_cmds;
    CButton m_clear_bar;
    CButton m_disable_ya;
    CButton m_history_tab;
    CButton m_plugins_logs;
    CEdit   m_plugins_logs_window;
    CComboBox m_codepage;
    CButton m_prompt_iacga;
    CButton m_prompt_pcre;
    CEdit   m_prompt_pcre_template;

public:
    enum { IDD = IDD_PROPERTY_COMMON };
    PropertyCommon(PropertiesData *data) : propData(data) {}

private:
    BEGIN_MSG_MAP(PropertyCommon)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_HANDLER(IDC_EDIT_VIEW_HISTORY, EN_KILLFOCUS, OnViewHistory)
        COMMAND_HANDLER(IDC_EDIT_CMD_HISTORY, EN_KILLFOCUS, OnCmdHistory)
        COMMAND_ID_HANDLER(IDC_CHECK_SHOW_SYSTEM_CMDS, OnShowSysCmds)
        COMMAND_ID_HANDLER(IDC_CHECK_CLEAR_BAR, OnClearBar)
        COMMAND_ID_HANDLER(IDC_CHECK_DISABLE_DOUBLE_YA, OnDisableYa)
        COMMAND_ID_HANDLER(IDC_CHECK_HISTORYTAB, OnHistoryTab)
        COMMAND_ID_HANDLER(IDC_CHECK_PLUGINSLOGS, OnPluginsLogs)
        COMMAND_HANDLER(IDC_EDIT_PLUGINSLOGS, EN_KILLFOCUS, OnPluginsWindow)
        COMMAND_HANDLER(IDC_COMBO_CODEPAGE, CBN_SELCHANGE, OnCodePage)
        COMMAND_ID_HANDLER(IDC_RADIO_PROMT_GA, OnPromptGA)
        COMMAND_ID_HANDLER(IDC_RADIO_PROMT_PCRE, OnPromptPcre)
        COMMAND_HANDLER(IDC_EDIT_PROMPT_PCRE, EN_KILLFOCUS, OnPromptTemplate)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_bl1.SubclassWindow(GetDlgItem(IDC_STATIC_BL1));
        m_bl2.SubclassWindow(GetDlgItem(IDC_STATIC_BL2));
        m_history_size.Attach(GetDlgItem(IDC_EDIT_VIEW_HISTORY));
        m_history_size.SetLimitText(5);
        m_cmd_size.Attach(GetDlgItem(IDC_EDIT_CMD_HISTORY));
        m_cmd_size.SetLimitText(3);
        m_show_system_cmds.Attach(GetDlgItem(IDC_CHECK_SHOW_SYSTEM_CMDS));
        m_clear_bar.Attach(GetDlgItem(IDC_CHECK_CLEAR_BAR));        
        m_history_tab.Attach(GetDlgItem(IDC_CHECK_HISTORYTAB));
        m_disable_ya.Attach(GetDlgItem(IDC_CHECK_DISABLE_DOUBLE_YA));
        m_plugins_logs.Attach(GetDlgItem(IDC_CHECK_PLUGINSLOGS));
        m_plugins_logs_window.Attach(GetDlgItem(IDC_EDIT_PLUGINSLOGS));
        m_plugins_logs_window.SetLimitText(1);

        m_prompt_iacga.Attach(GetDlgItem(IDC_RADIO_PROMT_GA));
        m_prompt_pcre.Attach(GetDlgItem(IDC_RADIO_PROMT_PCRE));
        m_prompt_pcre_template.Attach(GetDlgItem(IDC_EDIT_PROMPT_PCRE));
        m_prompt_pcre_template.SetWindowText(propData->recognize_prompt_template.c_str());
        if (propData->recognize_prompt == 0)
        {
            m_prompt_iacga.SetCheck(BST_CHECKED);
            m_prompt_pcre_template.EnableWindow(FALSE);
        }
        else
        {
            m_prompt_pcre.SetCheck(BST_CHECKED);
        }

        m_codepage.Attach(GetDlgItem(IDC_COMBO_CODEPAGE));
        m_codepage.AddString(L"win");
        m_codepage.AddString(L"utf8");
        if (propData->codepage == L"utf8")
            m_codepage.SetCurSel(1);
        else
            m_codepage.SetCurSel(0);

        wchar_t buffer[8];
        _itow(propData->view_history_size, buffer, 10);
        m_history_size.SetWindowText(buffer);
        _itow(propData->cmd_history_size, buffer, 10);
        m_cmd_size.SetWindowText(buffer);
        m_show_system_cmds.SetCheck(propData->show_system_commands ? BST_CHECKED : BST_UNCHECKED);
        m_clear_bar.SetCheck(propData->clear_bar ? BST_CHECKED : BST_UNCHECKED);
        m_history_tab.SetCheck(propData->history_tab ? BST_CHECKED : BST_UNCHECKED);
        m_disable_ya.SetCheck(propData->disable_ya ? BST_CHECKED : BST_UNCHECKED);        
        m_plugins_logs.SetCheck(propData->plugins_logs ? BST_CHECKED : BST_UNCHECKED);
        _itow(propData->plugins_logs_window, buffer, 10);
        m_plugins_logs_window.SetWindowText(buffer);
        if (!propData->plugins_logs)
            m_plugins_logs_window.EnableWindow(FALSE);
        return 0;
    }

    LRESULT OnViewHistory(WORD, WORD, HWND, BOOL&)
    {
        tstring count_text;
        getWindowText(m_history_size, &count_text);
        int count = _wtoi(count_text.c_str());
        if (count < MIN_VIEW_HISTORY_SIZE || count > MAX_VIEW_HISTORY_SIZE)
        {
            if (count < MIN_VIEW_HISTORY_SIZE)
                count = MIN_VIEW_HISTORY_SIZE;
            else
                count = MAX_VIEW_HISTORY_SIZE;
            wchar_t buffer[8];
            _itow(count, buffer, 10);
            m_history_size.SetWindowText(buffer);
        }
        propData->view_history_size = count;
        return 0;
    }

    LRESULT OnCmdHistory(WORD, WORD, HWND, BOOL&)
    {
        tstring count_text;
        getWindowText(m_cmd_size, &count_text);
        int count = _wtoi(count_text.c_str());
        if (count < MIN_CMD_HISTORY_SIZE || count > MAX_CMD_HISTORY_SIZE)
        {
            if (count < MIN_CMD_HISTORY_SIZE)
                count = MIN_CMD_HISTORY_SIZE;
            else
                count = MAX_CMD_HISTORY_SIZE;
            wchar_t buffer[8];
            _itow(count, buffer, 10);
            m_cmd_size.SetWindowText(buffer);
        }
        propData->cmd_history_size = count;
        return 0;
    }

    LRESULT OnShowSysCmds(WORD, WORD, HWND, BOOL&)
    {
        int state = (m_show_system_cmds.GetCheck() == BST_CHECKED) ? 1 : 0;
        propData->show_system_commands = state;
        return 0;
    }

    LRESULT OnClearBar(WORD, WORD, HWND, BOOL&)
    {
        int state = (m_clear_bar.GetCheck() == BST_CHECKED) ? 1 : 0;
        propData->clear_bar = state;
        return 0;
    }

    LRESULT OnDisableYa(WORD, WORD, HWND, BOOL&)
    {
        int state = (m_disable_ya.GetCheck() == BST_CHECKED) ? 1 : 0;
        propData->disable_ya = state;
        return 0;
    }

    LRESULT OnHistoryTab(WORD, WORD, HWND, BOOL&)
    {
        int state = (m_history_tab.GetCheck() == BST_CHECKED) ? 1 : 0;
        propData->history_tab = state;
        return 0;
    }

    LRESULT OnPluginsLogs(WORD, WORD, HWND, BOOL&)
    {
        int state = (m_plugins_logs.GetCheck() == BST_CHECKED) ? 1 : 0;
        propData->plugins_logs = state;
        m_plugins_logs_window.EnableWindow(state ? TRUE : FALSE);
        return 0;
    }

    LRESULT OnPluginsWindow(WORD, WORD, HWND, BOOL&)
    {
        tstring window;
        getWindowText(m_plugins_logs_window, &window);
        int value = _wtoi(window.c_str());
        if (value < 0 || value > OUTPUT_WINDOWS)
        {
            wchar_t buffer[8];
            _itow(propData->plugins_logs_window, buffer, 10);
            m_plugins_logs_window.SetWindowText(buffer);
        }
        else
        {
            propData->plugins_logs_window = value;
        }
        return 0;
    }
    LRESULT OnCodePage(WORD, WORD, HWND, BOOL&)
    {
        int item = m_codepage.GetCurSel();
        if (item == 0) propData->codepage = L"win";
        else propData->codepage = L"utf8";
        return 0;
    }

    LRESULT OnPromptGA(WORD, WORD, HWND, BOOL&)
    {
        m_prompt_pcre_template.EnableWindow(FALSE);
        propData->recognize_prompt = 0;
        return 0;
    }

    LRESULT OnPromptPcre(WORD, WORD, HWND, BOOL&)
    {
        m_prompt_pcre_template.EnableWindow(TRUE);
        tstring text;
        getWindowText(m_prompt_pcre_template, &text);
        int cursor = text.size();
        m_prompt_pcre_template.SetFocus();
        m_prompt_pcre_template.SetSel(cursor, cursor);
        propData->recognize_prompt = 1;
        return 0;
    }

    LRESULT OnPromptTemplate(WORD, WORD, HWND, BOOL&)
    {
        tstring text;
        getWindowText(m_prompt_pcre_template, &text);
        propData->recognize_prompt_template = text;
        if (text.empty())
        {            
            m_prompt_iacga.SetCheck(BST_CHECKED);
            m_prompt_pcre.SetCheck(BST_UNCHECKED);
            propData->recognize_prompt = 0;
            m_prompt_pcre_template.EnableWindow(FALSE);
        }
        return 0;
    }
};
