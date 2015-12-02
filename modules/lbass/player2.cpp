#include "stdafx.h"
#include "player.h"
#include "bass/c/bass.h"
#include <MMSystem.h>

struct RecordParams
{
    RecordParams() : hfile(INVALID_HANDLE_VALUE), error(FALSE), recording(FALSE), stopped(FALSE) {}
    HANDLE hfile;
    BOOL error;
    BOOL recording;
    BOOL stopped;
};

static BOOL CALLBACK MyRecordProc(HRECORD handle, const void *buffer, DWORD length, void *user)
{
    RecordParams *rp = (RecordParams*)user;
    DWORD written = 0;
    if (!WriteFile(rp->hfile, buffer, length, &written, NULL) || written != length)
    {
        rp->error = TRUE;
        rp->stopped = TRUE;
    }
    if (!rp->recording)
        rp->stopped = TRUE;

    if (rp->stopped)
    {
       CloseHandle(rp->hfile);
       rp->hfile = INVALID_HANDLE_VALUE;
       return FALSE;
    }
    return TRUE;
}

int getMicrophoneDevice()
{
    BASS_DEVICEINFO info;
    int device = -1;
    for (int a = 0; BASS_RecordGetDeviceInfo(a, &info); a++) 
    {
        if ((info.flags&BASS_DEVICE_ENABLED) && (info.flags&BASS_DEVICE_TYPE_MASK) == BASS_DEVICE_TYPE_MICROPHONE)
        {
            device = a; break;
        }
    }
    return device;
}

bool BassPlayer::canRecord()
{
    int device = getMicrophoneDevice();
    return (device == -1) ? false : true;
}

bool BassPlayer::setRecord(const wchar_t* param, int value)
{
    if (!wcscmp(param, L"freq")) {
        m_freq_record = value;
        return true;
    }
    if (!wcscmp(param, L"channels")) {
        m_chans_record = value;
        return true;
    }
    return false;
}

bool BassPlayer::startRecord(const wchar_t* file)
{
    if (m_record_params)
        return error(L"Already recording. First stop and start new.");
    int device = getMicrophoneDevice();
    if (device == -1)
        return error(L"Microphone not found.");
    if (!BASS_RecordInit(device))
        return error_bass(L"Can't initialize microphone.", NULL);
    HANDLE hfile = CreateFile(file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hfile == INVALID_HANDLE_VALUE)
    {
        std::wstring message(L"Can't open file to writing :");
        message.append(file);
        return error(message.c_str());
    }

    // write wav header
    WAVEFORMATEX wf;
    wf.wFormatTag = WAVE_FORMAT_PCM;
    wf.wBitsPerSample = 16;
    wf.nChannels = LOWORD(m_chans_record);
    wf.nSamplesPerSec = m_freq_record;
    wf.nBlockAlign = wf.nChannels*wf.wBitsPerSample/8;
    wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;
    wf.cbSize = 0;
    DWORD wf_size = sizeof(WAVEFORMATEX) - sizeof(WORD);

    DWORD headerSize = 20 + wf_size + 8;
    unsigned char* header = new unsigned char [headerSize];
    memcpy(header, "RIFF\0\0\0\0WAVEfmt \20\0\0\0", 20);
    memcpy(header+20, &wf, wf_size);
    memcpy(header+20+wf_size, "data\0\0\0\0", 8);
    DWORD written = 0;
    if (!WriteFile(hfile, header, headerSize, &written, NULL) || written != headerSize)
    {
        CloseHandle(hfile);
        DeleteFile(file);
        std::wstring message(L"Can't start writing :");
        message.append(file);
        return error(message.c_str());
    }
    
    m_record_params = new RecordParams;
    m_record_params->hfile = hfile;
    m_record_params->recording = TRUE;
    m_record = BASS_RecordStart(m_freq_record, m_chans_record, 0, MyRecordProc, m_record_params);
    if (!m_record)
    {
        delete m_record_params;
        m_record_params = NULL;
        CloseHandle(hfile);
        DeleteFile(file);
        return error_bass(L"Can't start recording.", NULL);
    }
    return true;
}

void BassPlayer::stopRecord()
{
    if (!m_record_params)
        return;
    m_record_params->recording = FALSE;
    while (!m_record_params->stopped) { ::Sleep(100);  }
    deleteRecordParams();
}

bool BassPlayer::isRecording()
{
    if (!m_record_params)
        return false;
    return !m_record_params->stopped;
}

void BassPlayer::deleteRecordParams()
{
    if (!m_record_params)
        return;
    HANDLE h = m_record_params->hfile;
    if (h != INVALID_HANDLE_VALUE) CloseHandle(h);
    delete m_record_params;
    m_record_params = NULL;
}
