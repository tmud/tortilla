#pragma  once
#include "common.h"

class  BassObject;
struct RecordParams;
class  BassPlayer
{
    bool bass_loaded;
    std::vector<BassObject*> m_objects;
    std::vector<int> m_indexes;
    std::wstring m_error_msg;

    DWORD m_freq_record;
    DWORD m_chans_record;
    DWORD m_sensivity_record;
    HRECORD m_record;
    RecordParams *m_record_params;

public:
    BassPlayer() : bass_loaded(false), m_freq_record(44100), m_chans_record(2), m_sensivity_record(30), m_record(NULL), m_record_params(NULL) {}
    ~BassPlayer() { deleteRecordParams(); }

    bool loadBass();
    bool unloadBass();
    int  load(const wchar_t* file, bool as_sample);
    bool unload(int id);
    bool play(int id, int volume, BassObjectEvents *callback);
    bool stop(int id);
    void stopAll();
    int  isSample(int id);
    int  isPlaying(int id);
    bool isHandle(int id) const;
    bool getPath(int id, std::wstring* path);

    bool canRecord();
    bool setRecord(const wchar_t* param, int value);
    bool startRecord(const wchar_t* file);
    void stopRecord();
    bool isRecording();

    const wchar_t* getLastError() const;
private:
    int loadStream(const wchar_t* file);
    int loadSample(const wchar_t* file);
    int loadMusic(const wchar_t* file);
    BassObject* get(int id) const;
    int push(BassObject* obj);
    void deleteRecordParams();

    bool error_bass(const wchar_t* error_text, const wchar_t* file);
    bool error(const wchar_t* error_text);
    bool error_id(int id);    
    int  error_file(const wchar_t* error_text, const wchar_t* file);
};
