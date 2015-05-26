#pragma once
#include <vector>

class BassObject
{
    std::wstring m_filename;
public:
    BassObject() {}
    virtual ~BassObject() {}
    virtual bool play() = 0;
    const std::wstring& getname() const { return m_filename; }
    void setname(const wchar_t* fname) { m_filename.assign(fname); }
};

class BassStream : public BassObject
{
    HSTREAM m_stream;
public:
    BassStream() : m_stream(NULL) {}
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
    bool play()
    {
        if (!m_stream)
            return false;
        return (BASS_ChannelPlay(m_stream, TRUE)) ? true : false;
    }
};
class BassSample : public BassObject
{
    HSAMPLE m_sample;
public:
    BassSample() : m_sample(NULL) {}
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
    bool play()
    {
        if (!m_sample)
            return false;
        HCHANNEL ch = BASS_SampleGetChannel(m_sample, FALSE);
        //BASS_ChannelSetAttribute(ch, BASS_ATTRIB_VOL, 0.5f);
        //BASS_ChannelSetAttribute(ch, BASS_ATTRIB_PAN, ((rand() % 201) - 100) / 100.f);
        return (BASS_ChannelPlay(ch, FALSE)) ? true : false;
    }
};

class BassMusic : public BassObject
{
    HMUSIC m_music;
public:
    BassMusic() : m_music(NULL) {}
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

    bool play()
    {
        if (!m_music)
            return false;
         return (BASS_ChannelPlay(m_music, TRUE)) ? true : false;
    }
};
