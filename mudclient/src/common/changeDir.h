#pragma once

class ChangeDir
{
    std::wstring m_currentDir;
    bool m_dirChanged;
public:
    ChangeDir() : m_dirChanged(false)
    {
        DWORD buffer_required = GetCurrentDirectory(0, NULL);
        MemoryBuffer buffer(buffer_required * sizeof(wchar_t));
        wchar_t* ptr = (wchar_t*)buffer.getData();
        GetCurrentDirectory(buffer_required, ptr);
        m_currentDir.assign(ptr);
    }
    ~ChangeDir()
    {
        restoreDir();
    } 

    bool changeDir(const std::wstring& dir)
    {
        BOOL result = SetCurrentDirectory(dir.c_str());
        if (result)
            m_dirChanged = true;
        return (result) ? true : false;
    }

    void restoreDir()
    {
        if (m_dirChanged)
            SetCurrentDirectory(m_currentDir.c_str());
        m_dirChanged = false;
    }

    const std::wstring& getCurrentDir() const
    {
        return m_currentDir;
    }
};
