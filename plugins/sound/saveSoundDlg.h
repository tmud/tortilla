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

public:
    SaveSoundDlg(SoundPlayer* p) : player(p), m_recording(false), m_recording_id(-1) {}
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
    END_MSG_MAP()

    LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&)
    {
        m_error_label.Attach(GetDlgItem(IDC_EDIT_ERROR));
        m_start.Attach(GetDlgItem(IDC_BUTTON_STARTRECORD));
        m_stop.Attach(GetDlgItem(IDC_BUTTON_STOPRECORD));
        m_play.Attach(GetDlgItem(IDC_BUTTON_PLAYRECORD));
        m_save.Attach(GetDlgItem(IDC_BUTTON_SAVERECORD));
        
        m_stop.EnableWindow(FALSE);
        m_play.EnableWindow(FALSE);
        m_save.EnableWindow(FALSE);

        m_start.SetFocus();
        CenterWindow(GetParent());
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
