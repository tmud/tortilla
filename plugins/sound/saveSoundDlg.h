#pragma once
#include "resource.h"
#include "soundPlayer.h"

class SaveSoundDlg : public CDialogImpl<SaveSoundDlg>
{
    SoundPlayer* player;
    CStatic m_error_label;
    CButton m_start;
    CButton m_stop;
public:
    SaveSoundDlg(SoundPlayer* p) : player(p) {}
    enum { IDD = IDD_SAVE_SOUND };

private:
    BEGIN_MSG_MAP(SaveSoundDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnOk)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_ID_HANDLER(IDC_BUTTON_STARTRECORD, OnStartRecord)
        COMMAND_ID_HANDLER(IDC_BUTTON_STOPRECORD, OnStopRecord)       
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_error_label.Attach(GetDlgItem(IDC_STATIC_ERROR));
        m_start.Attach(GetDlgItem(IDC_BUTTON_STARTRECORD));
        m_stop.Attach(GetDlgItem(IDC_BUTTON_STOPRECORD));
        m_stop.EnableWindow(FALSE);
        CenterWindow(GetParent());
        return 0;
    }

    LRESULT OnStartRecord(WORD, WORD, HWND, BOOL&)
    {
        std::wstring error;
        if (!player->startRecord(L"test.wav", &error))
            m_error_label.SetWindowText(error.c_str());
        else
        {
            m_error_label.SetWindowText(L"Запись...");
            m_start.EnableWindow(FALSE);
            m_stop.EnableWindow(TRUE);
        }
        return 0;
    }

    LRESULT OnStopRecord(WORD, WORD, HWND, BOOL&)
    {
        std::wstring error;
        if (!player->stopRecord(&error))
            m_error_label.SetWindowText(error.c_str());
        else
        {
            m_error_label.SetWindowText(L"");
            m_start.EnableWindow(TRUE);
            m_stop.EnableWindow(FALSE);
        }
        return 0;
    }

    LRESULT OnOk(WORD, WORD, HWND, BOOL&)
    {
        EndDialog(IDOK);
        return 0;
    }

    LRESULT OnCancel(WORD, WORD, HWND, BOOL&)
    {
        EndDialog(IDCANCEL);
        return 0;
    }
};
