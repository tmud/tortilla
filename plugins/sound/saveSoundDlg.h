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
    CButton m_cat_mud;
    CButton m_cat_public;
    CEdit m_recording_file;
    std::wstring m_temp_file;
    bool m_recording;
    int m_recording_id;
    CTrackBarCtrl m_sens_slider;
    CEdit m_sens_level;
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
        m_sens_slider.Attach(GetDlgItem(IDC_SLIDER_SENSIVITY));
        m_sens_slider.SetRange(0, 100);
        m_sens_level.Attach(GetDlgItem(IDC_EDIT_SENSIVITY));
        m_sens_level.SetLimitText(3);

        m_cat_mud.Attach(GetDlgItem(IDC_RADIO_MUDCAT));
        m_cat_public.Attach(GetDlgItem(IDC_RADIO_PUBLICCAT));
        m_recording_file.Attach(GetDlgItem(IDC_EDIT_FILENAME));

        int level = player->recordingParams().sensitivity;
        m_sens_level.SetWindowText(int_to_wstring(level));

        int destination = player->recordingParams().destfolder;
        if (destination == 1)
            m_cat_public.SetCheck(1);
        else
            m_cat_mud.SetCheck(1);

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
        int pos = m_sens_slider.GetPos();
        wchar_t buffer[8];
        swprintf(buffer, L"%d", pos);
        m_block_update = true;
        m_sens_level.SetWindowText(buffer);
        m_block_update = false;
        player->recordingParams().sensitivity = pos;
        return 0;
    }

    LRESULT OnSensChanging(WORD, WORD, HWND, BOOL&)
    {
        if (m_block_update)
            return 0;
        int len = m_sens_level.GetWindowTextLength();
        wchar_t *buffer = new wchar_t[len+1];
        m_sens_level.GetWindowText(buffer, len+1);
        bool check = false;
        int value = wstring_to_int(buffer, &check);
        if (check)
        {
            value = max(min(value, 100), 0);
            m_block_update = true;
            m_sens_slider.SetPos(value);
            player->recordingParams().sensitivity = value;
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
        int len = m_recording_file.GetWindowTextLength();
        if (len == 0)
        {
            MessageBox(L"Введите имя файла для записи!", L"Запись звука", MB_ICONSTOP|MB_OK);
            m_recording_file.SetFocus();
            return 0;
        }
        wchar_t *buffer = new wchar_t[len+1];
        m_recording_file.GetWindowText(buffer, len+1);
        std::wstring filname(buffer);
        delete []buffer;
        filname.append(L".wav");
        saveParameters();
        bool result = player->saveFile(m_temp_file.c_str(), filname.c_str());       
        if (!result)
             MessageBox(L"Не удалось сохранить файл!", L"Запись звука", MB_ICONSTOP|MB_OK);
        else
        {
            deleteTempFile();
            m_error_label.SetWindowText(L"Успешно сохранено.");
            m_recording_file.SetWindowText(L"");
        }
        return 0;
    }

    void stopRecord()
    {
        BOOL h = FALSE;
        OnStopRecord(0, 0, 0, h);
    }

    void saveParameters()
    {
        int level = m_sens_slider.GetPos();
        player->recordingParams().sensitivity = level;
        int desination = 0;
        if (m_cat_public.GetCheck() == BST_CHECKED)
            desination = 1;
        player->recordingParams().destfolder = desination;
    }

    LRESULT OnOk(WORD, WORD, HWND, BOOL&)
    {
        stopRecord();
        saveParameters();
        EndDialog(IDOK);
        return 0;
    }

    LRESULT OnCancel(WORD, WORD, HWND, BOOL&)
    {
        stopRecord();
        saveParameters();
        EndDialog(IDCANCEL);        
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
