#pragma once
#include <vector>

class BassObject
{
    std::wstring m_filename;
    std::wstring m_error;
    bool m_sample;
public:
    BassObject(bool sample) : m_sample(sample) {}
    virtual ~BassObject() {}
    virtual bool play(float volume) = 0;
    virtual bool isplaying() = 0;
    virtual void stop() = 0;
    bool issample() const { return m_sample; }
    const std::wstring& getname() const { return m_filename; }
    void setname(const wchar_t* fname) { m_filename.assign(fname); }
    const std::wstring& geterror() const { return m_error; }
    void seterror(const wchar_t* error) { m_error.assign(error); }
};

class BassStream : public BassObject
{
    HSTREAM m_stream;
public:
    BassStream() : BassObject(false), m_stream(NULL) {}
    ~BassStream() {
        if (m_stream)
            BASS_StreamFree(m_stream);
    }
    bool load(const wchar_t* file)
    {
        if (m_stream)
            return false;
        m_stream = BASS_StreamCreateFile(FALSE, file, 0, 0, 0);
        if (!m_stream)
            return false;
        setname(file);
        return true;
    }
    bool play(float volume)
    {
        if (!m_stream)
            return false;
        return (BASS_ChannelPlay(m_stream, TRUE)) ? true : false;
    }
    bool isplaying()
    {
        if (!m_stream) return false;
        DWORD status = BASS_ChannelIsActive(m_stream);
        return (status == BASS_ACTIVE_PLAYING) ? true : false;
    }
    void stop()
    {
        if (m_stream)
            BASS_ChannelStop(m_stream);
        m_stream = NULL;
    }
};
class BassSample : public BassObject
{
    HSAMPLE m_sample;
public:
    BassSample() : BassObject(true), m_sample(NULL) {}
    ~BassSample() {
        if (m_sample)
            BASS_SampleFree(m_sample);
    }
    bool load(const wchar_t* file)
    {
        if (m_sample)
            return false;
        m_sample = BASS_SampleLoad(FALSE, file, 0, 0, 1, 0);
        if (!m_sample)
            return false;
        setname(file);
        return true;
    }
    bool play(float volume)
    {
        if (!m_sample)
            return false;
        HCHANNEL ch = BASS_SampleGetChannel(m_sample, FALSE);
        BASS_ChannelSetAttribute(ch, BASS_ATTRIB_VOL, volume);
        //BASS_ChannelSetAttribute(ch, BASS_ATTRIB_PAN, ((rand() % 201) - 100) / 100.f);
        return (BASS_ChannelPlay(ch, FALSE)) ? true : false;
    }
    bool isplaying()
    {
        if (!m_sample) return false;
        DWORD status = BASS_ChannelIsActive(m_sample);
        return (status == BASS_ACTIVE_PLAYING) ? true : false;
    }
    void stop()
    {
        if (m_sample)
            BASS_ChannelStop(m_sample);
        m_sample = NULL;    
    }
};

class BassMusic : public BassObject
{
    HMUSIC m_music;
public:
    BassMusic() : BassObject(true), m_music(NULL) {}
    ~BassMusic() {
        if (m_music) 
            BASS_MusicFree(m_music);
    }
    bool load(const wchar_t* file)
    {
        if (m_music)
            return false;
        m_music = BASS_MusicLoad(FALSE, file, 0, 0, 0, 0);
        if (!m_music)
            return false;
        setname(file);
        return true;
    }
    bool play(float volume)
    {
        if (!m_music)
            return false;
         return (BASS_ChannelPlay(m_music, TRUE)) ? true : false;
    }
    bool isplaying()
    {
        if (!m_music) return false;
        DWORD status = BASS_ChannelIsActive(m_music);
        return (status == BASS_ACTIVE_PLAYING) ? true : false;
    }
     void stop()
    {
        if (m_music)
            BASS_ChannelStop(m_music);
        m_music = NULL;    
    }
};
