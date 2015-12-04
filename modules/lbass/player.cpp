#include "stdafx.h"
#include "bobjects.h"
#include "player.h"

bool BassPlayer::loadBass()
{
    if (HIWORD(BASS_GetVersion()) != BASSVERSION)
        return error(L"An incorrect version of bass.dll");
    if (!BASS_Init(-1, 44100, 0, NULL, NULL))
        return error_bass(L"Can't initialize sound system", NULL);
    BASS_FX_GetVersion();
    //m_fx_plugin = BASS_PluginLoad(L"bass_fx.dll", 0);
    //if (!m_fx_plugin)
      //  return error_bass(L"Can't initialize fx plugin", NULL);
    bass_loaded = true;
    return true;
}

bool BassPlayer::unloadBass()
{
    if (!bass_loaded)
        return true;
    bass_loaded = false;
    for (int i = 0, e = m_objects.size(); i < e; ++i)
        delete m_objects[i];
    m_objects.clear();
    m_indexes.clear();
    if (!BASS_Free())
        return error_bass(L"Can't unload bass", NULL);
    return true;
}

int BassPlayer::load(const wchar_t* file, bool as_sample)
{
    if (!bass_loaded)
    {
        error(L"BASS not initialized"); return -1;
    }

    HANDLE hfile = CreateFile(file, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hfile == INVALID_HANDLE_VALUE)
        return error_file(L"Can't open file", file);
    DWORD hsize = 0;
    DWORD size = GetFileSize(hfile, &hsize);
    CloseHandle(hfile);
    if (hsize != 0)
        return error_file(L"File is huge", file);
    if (size == 0)
        return error_file(L"File is empty", file);

    int index = -1;

    // get extension
    bool correct_ext = false;
    std::wstring f(file);
    int pos = f.rfind(L'.');
    if (pos != -1)
    {
        std::wstring ext(f.substr(pos + 1));
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
        if (ext == L"mp3" || ext == L"wav" || ext == L"ogg" || ext == L"mp2" || ext == L"mp1" || ext == L"aiff")
        {
            correct_ext = true;
            if (as_sample)
                index = loadSample(file);
            else
                index = loadStream(file);
        }
        else if (ext == L"mo3" || ext == L"xm" || ext == L"it" || ext == L"s3m" || ext == L"mtm" || ext == L"mod" || ext == L"umx")
        {
            if (as_sample)
                return error_bass(L"Can't load as sample", file);
            correct_ext = true;
            index = loadMusic(file);
        }
    }
    if (!correct_ext)
        { error_bass(L"Unknown file type", file); return -1; }
    if (index == -1)
        { error_bass(L"Can't open file", file); return -1; }
    return index;
}

bool BassPlayer::unload(int id)
{
    BassObject* object = get(id);
    if (!object)
        return error_id(id);
    m_objects[id] = NULL;
    delete object;
    m_indexes.push_back(id);
    return true;
}

bool BassPlayer::play(int id, int volume, BassObjectEvents *callback)
{
    if (!bass_loaded)
    {
        delete callback;
        return error(L"BASS not initialized");
    }
    int objects = m_objects.size();
    if (id >= 0 && id < objects)
    {
        BassObject *object = m_objects[id];
        object->setcallback(callback);
        if (!object->play(volume_ToFloat(volume)))
            return error(object->geterror().c_str());
        return true;
    }
    return error_id(id);
}

bool BassPlayer::stop(int id)
{
    BassObject *object = get(id);
    if (!object)
        return error_id(id);
    object->stop();
    return true;
}

void BassPlayer::stopAll()
{
    for (int i = 0, e = m_objects.size(); i < e; ++i)
        m_objects[i]->stop();
}

int BassPlayer::isSample(int id)
{
    BassObject *object = get(id);
    if (!object)
    {
        error_id(id); return -1;
    }
    return object->issample() ? 1 : 0;
}

int BassPlayer::isPlaying(int id)
{
    BassObject *object = get(id);
    if (!object)
    {
        error_id(id); return -1;
    }
    return object->isplaying() ? 1 : 0;
}

bool BassPlayer::isHandle(int id) const
{
    BassObject *object = get(id);
    return (object) ? true : false;
}

bool BassPlayer::getPath(int id, std::wstring* path)
{
    BassObject *object = get(id);
    if (!object)
    {
        error_id(id); return false;
    }
    path->assign(object->getname());
    return true;
}

const wchar_t* BassPlayer::getLastError() const
{
    return m_error_msg.c_str();
}

int BassPlayer::loadStream(const wchar_t* file)
{
    BassStream *stream = new (std::nothrow) BassStream();
    if (!stream || !stream->load(file))
    {
        delete stream; return -1;
    }
    return push(stream);
}

int BassPlayer::loadSample(const wchar_t* file)
{
    BassSample *sample = new (std::nothrow) BassSample();
    if (!sample || !sample->load(file))
    {
        delete sample; return -1;
    }
    return push(sample);
}

int BassPlayer::loadMusic(const wchar_t* file)
{
    BassMusic *music = new (std::nothrow) BassMusic();
    if (!music || !music->load(file))
    {
        delete music; return -1;
    }
    return push(music);
}

BassObject* BassPlayer::get(int id) const
{
    int count = m_objects.size();
    return (id >= 0 && id < count) ? m_objects[id] : NULL;
}

int BassPlayer::push(BassObject* obj)
{
    if (!m_indexes.empty()) {
        int last = m_indexes.size() - 1;
        int index = m_indexes[last];
        m_indexes.pop_back();
        m_objects[index] = obj;
        return index;
    }
    int index = m_objects.size();
    m_objects.push_back(obj);
    return index;
}

bool BassPlayer::error_bass(const wchar_t* error_text, const wchar_t* file)
{
    std::wstring msg(error_text);
    if (file) {
        msg.append(L" ");
        msg.append(file);
    }
    msg.append(L" (error code: ");
    wchar_t buffer[16];
    msg.append(_itow(BASS_ErrorGetCode(), buffer, 10));
    msg.append(L")");
    return error(msg.c_str());
}

bool BassPlayer::error(const wchar_t* error_text)
{
    m_error_msg.assign(error_text);
    return false;
}

bool BassPlayer::error_id(int id)
{
    wchar_t buffer[16];
    _itow(id, buffer, 10);
    m_error_msg.assign(L"Incorrect sound id: ");
    m_error_msg.append(buffer);
    return false;
}

int BassPlayer::error_file(const wchar_t* error_text, const wchar_t* file)
{
    m_error_msg.assign(error_text);
    m_error_msg.append(L": ");
    m_error_msg.append(file);
    return -1;
}
