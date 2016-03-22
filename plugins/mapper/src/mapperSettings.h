#pragma once

class MapperSettings :  public CDialogImpl<MapperSettings>
{
    PropertiesMapper* propData;
    CEdit m_begin_name, m_end_name, m_begin_descr, m_end_descr, m_begin_exits, m_end_exits;
    CEdit m_begin_vnum, m_end_vnum;
    CEdit m_north,m_south,m_west,m_east,m_up,m_down;
    CEdit m_ex_north,m_ex_south,m_ex_west,m_ex_east,m_ex_up,m_ex_down;
    CEdit m_dark_room;
    CEdit m_begin_prompt, m_end_prompt;

public:
   enum { IDD = IDD_MAPPER };
   MapperSettings(PropertiesMapper* props) : propData(props)
   {
       assert(props);
   }

private:
    BEGIN_MSG_MAP(MapperSettings)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        attach(&m_begin_name, IDC_EDIT_BEGIN_NAME, &propData->begin_name);
        attach(&m_end_name, IDC_EDIT_END_NAME, &propData->end_name);
        attach(&m_begin_descr, IDC_EDIT_BEGIN_DESCR, &propData->begin_descr);
        attach(&m_end_descr, IDC_EDIT_END_DESCR, &propData->end_descr);
        attach(&m_begin_exits, IDC_EDIT_BEGIN_EXITS, &propData->begin_exits);
        attach(&m_end_exits, IDC_EDIT_END_EXITS, &propData->end_exits);
        attach(&m_begin_prompt, IDC_EDIT_BEGINPROMPT, &propData->begin_prompt);
        attach(&m_end_prompt, IDC_EDIT_ENDPROMPT, &propData->end_prompt);
        attach(&m_begin_vnum, IDC_EDIT_BEGIN_VNUM, &propData->begin_vnum);
        attach(&m_end_vnum, IDC_EDIT_END_VNUM, &propData->end_vnum );

        attach(&m_north, IDC_EDIT_CMD_NORTH, &propData->north_cmd);
        attach(&m_south, IDC_EDIT_CMD_SOUTH, &propData->south_cmd);
        attach(&m_west, IDC_EDIT_CMD_WEST, &propData->west_cmd);
        attach(&m_east, IDC_EDIT_CMD_EAST, &propData->east_cmd);
        attach(&m_up, IDC_EDIT_CMD_UP, &propData->up_cmd);
        attach(&m_down, IDC_EDIT_CMD_DOWN, &propData->down_cmd);

        attach(&m_ex_north, IDC_EDIT_EXIT_NORTH, &propData->north_exit);
        attach(&m_ex_south, IDC_EDIT_EXIT_SOUTH, &propData->south_exit);
        attach(&m_ex_west, IDC_EDIT_EXIT_WEST, &propData->west_exit);
        attach(&m_ex_east, IDC_EDIT_EXIT_EAST, &propData->east_exit);
        attach(&m_ex_up, IDC_EDIT_EXIT_UP, &propData->up_exit);
        attach(&m_ex_down, IDC_EDIT_EXIT_DOWN, &propData->down_exit);
        attach(&m_dark_room, IDC_EDIT_DARK_ROOM, &propData->dark_room);

        CenterWindow(GetParent());
        return 0;
    }

    LRESULT OnCloseCmd(WORD, WORD wID, HWND, BOOL&)
    {
        if (wID != IDOK)
		    EndDialog(wID);
        for (int i=0,e=elements.size(); i<e; ++i)
            elements[i].get();
        EndDialog(wID);
        return 0;
    }

private:   
    struct el
    {
        void get() 
        {
            HWND hwnd = *cntrl;
            int text_len = ::GetWindowTextLength(hwnd);
            MemoryBuffer tmp((text_len + 2)*sizeof(WCHAR));
            WCHAR *buffer = (WCHAR*)tmp.getData();
            ::GetWindowText(hwnd, buffer, text_len + 1);
            label->assign(buffer);
        }
        CEdit* cntrl;
        tstring* label;
    };
    std::vector<el> elements;

    void attach(CEdit* edit, UINT id, tstring* text)
    {
        el e;
        e.cntrl = edit;
        e.label = text;
        elements.push_back(e);
        edit->Attach(GetDlgItem(id));
        edit->SetWindowText(text->c_str());
    }
};
