#pragma once
#include "resource.h"
#include "soundPlayer.h"

class SaveSoundDlg : public CDialogImpl<SaveSoundDlg>, public SoundPlayerCallback
{
    SoundPlayer* player;
    CEdit m_error_label;
    CButton m_start;
    CButton m_stop;
    CButton m_play;
    CButton m_save;
    std::wstring m_temp_file;
    bool m_recording;
    int m_recording_id;
    CTrackBarCtrl m_sensivity_slider;
    CEdit m_sensivity_level;
    bool m_block_update;

public:
    SaveSoundDlg(SoundPlayer* p) : player(p), m_recording(false), m_recording_id(-1), m_block_update(false) {}
    ~SaveSoundDlg() { deleteTempFile(); }
    enum { IDD = IDD_SAVE_SOUND };

private:
    BEGIN_MSG_MAP(SaveSoundDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        COMMAND_ID_HANDLER(IDOK, OnOk)
        COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
        COMMAND_ID_HANDLER(IDC_BUTTON_STARTRECORD, OnStartRecord)
        COMMAND_ID_HANDLER(IDC_BUTTON_STOPRECORD, OnStopRecord)       
        COMMAND_ID_HANDLER(IDC_BUTTON_PLAYRECORD, OnPlayRecord)
        COMMAND_ID_HANDLER(IDC_BUTTON_SAVERECORD, OnSaveRecord)
        MESSAGE_HANDLER(WM_HSCROLL, OnHScroll)
        COMMAND_HANDLER(IDC_EDIT_SENSIVITY, EN_CHANGE, OnSensChanging)
        COMMAND_HANDLER(IDC_EDIT_SENSIVITY, EN_KILLFOCUS, OnSensChanged)
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_error_label.Attach(GetDlgItem(IDC_EDIT_ERROR));
        m_start.Attach(GetDlgItem(IDC_BUTTON_STARTRECORD));
        m_stop.Attach(GetDlgItem(IDC_BUTTON_STOPRECORD));
        m_play.Attach(GetDlgItem(IDC_BUTTON_PLAYRECORD));
        m_save.Attach(GetDlgItem(IDC_BUTTON_SAVERECORD));
        m_sensivity_slider.Attach(GetDlgItem(IDC_SLIDER_SENSIVITY));
        m_sensivity_slider.SetRange(0, 100);
        m_sensivity_level.Attach(GetDlgItem(IDC_EDIT_SENSIVITY));
        m_sensivity_level.SetLimitText(3);
        
        m_stop.EnableWindow(FALSE);
        m_play.EnableWindow(FALSE);
        m_save.EnableWindow(FALSE);

        m_start.SetFocus();
        CenterWindow(GetParent());
        return 0;
    }

    LRESULT OnHScroll(UINT, WPARAM, LPARAM, BOOL&)
    {
        if (m_block_update)
            return 0;
        int pos = m_sensivity_slider.GetPos();
        wchar_t buffer[8];
        swprintf(buffer, L"%d", pos);
        m_block_update = true;
        m_sensivity_level.SetWindowText(buffer);
        m_block_update = false;
        return 0;
    }

    LRESULT OnSensChanging(WORD, WORD, HWND, BOOL&)
    {
        if (m_block_update)
            return 0;
        int len = m_sensivity_level.GetWindowTextLength();
        wchar_t *buffer = new wchar_t[len+1];
        m_sensivity_level.GetWindowText(buffer, len+1);
        bool check = false;
        int value = wstring_to_int(buffer, &check);
        if (check)
        {
            value = max(min(value, 100), 0);
            m_block_update = true;
            m_sensivity_slider.SetPos(value);
            m_block_update = false;
        }
        return 0;
    }

    LRESULT OnSensChanged(WORD, WORD, HWND, BOOL&)
    {
        BOOL b = FALSE;
        OnHScroll(0, 0, 0, b);
        return 0;
    }

    LRESULT OnStartRecord(WORD, WORD, HWND, BOOL&)
    {
        deleteTempFile();

        wchar_t buffer[MAX_PATH+1];
        GetTempPath(MAX_PATH, buffer);
        std::wstring path(buffer);

        path.assign(L"D:\\");

        SYSTEMTIME st;
        GetLocalTime(&st);
        swprintf(buffer, L"tw%x%x%x.wav", st.wSecond, st.wMinute, st.wMilliseconds );
        path.append(buffer);
        m_temp_file.assign(path);

        m_play.EnableWindow(FALSE);
        std::wstring error;
        if (!player->startRecord(m_temp_file.c_str(), &error))
        {
            m_error_label.SetWindowText(error.c_str());
            deleteTempFile();
        }
        else
        {
            m_error_label.SetWindowText(L"Запись...");
            m_start.EnableWindow(FALSE);
            m_stop.EnableWindow(TRUE);
            m_save.EnableWindow(FALSE);            
            m_recording = true;
        }        
        return 0;
    }

    LRESULT OnStopRecord(WORD, WORD, HWND, BOOL&)
    {
        if (!m_recording)
        {
            endPlaying();
            return 0;
        }

        std::wstring error;
        if (!player->stopRecord(&error))
            m_error_label.SetWindowText(error.c_str());
        else
        {
            m_recording = false;
            m_error_label.SetWindowText(L"");
            m_start.EnableWindow(TRUE);
            m_stop.EnableWindow(FALSE);
            m_play.EnableWindow(TRUE);
            m_save.EnableWindow(TRUE);
        }
        return 0;
    }

    LRESULT OnPlayRecord(WORD, WORD, HWND, BOOL&)
    {
        bool status = false;
        if (!m_temp_file.empty())
        {
            std::wstring error;
            if (!player->playFile(m_temp_file.c_str(), &error, this))
                m_error_label.SetWindowText(error.c_str());
            else 
            {
                status = true;
            }
        }
        if (status)
        {
            m_start.EnableWindow(FALSE);
            m_stop.EnableWindow(TRUE);
            m_save.EnableWindow(FALSE);
        }
        return 0;
    }

    LRESULT OnSaveRecord(WORD, WORD, HWND, BOOL&)
    {
        return 0;
    }

    void stopRecord()
    {
        BOOL h = FALSE;
        OnStopRecord(0, 0, 0, h);
    }

    LRESULT OnOk(WORD, WORD, HWND, BOOL&)
    {        
        EndDialog(IDOK);
        stopRecord();
        return 0;
    }

    LRESULT OnCancel(WORD, WORD, HWND, BOOL&)
    {
        EndDialog(IDCANCEL);
        stopRecord();
        return 0;
    }

private:
    void endPlaying()
    {
        std::wstring error;
        if (!player->stopPlayFile(&error))
            m_error_label.SetWindowText(error.c_str());
        m_recording_id = -1;
        m_error_label.SetWindowText(L"");
        m_start.EnableWindow(TRUE);
        m_stop.EnableWindow(FALSE);
        m_play.EnableWindow(TRUE);
        m_save.EnableWindow(TRUE);
    }

    void deleteTempFile()
    {
        if (!m_temp_file.empty())
            DeleteFile(m_temp_file.c_str());
        m_temp_file.clear();    
    }
};
