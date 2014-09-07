#pragma once

class ChangeDir
{
    tstring m_currentDir;
    bool m_dirChanged;
public:
    ChangeDir() : m_dirChanged(false)
    {
        DWORD buffer_required = GetCurrentDirectory(0, NULL);
        MemoryBuffer buffer(buffer_required * sizeof(WCHAR));
        WCHAR* ptr = (WCHAR*)buffer.getData();
        GetCurrentDirectory(buffer_required, ptr);
        m_currentDir.assign(ptr);
    }
    ~ChangeDir()
    {
        if (m_dirChanged)
            SetCurrentDirectory(m_currentDir.c_str());
    } 

    bool changeDir(const tstring& dir)
    {
        BOOL result = SetCurrentDirectory(dir.c_str());
        if (result)
            m_dirChanged = true;
        return (result) ? true : false;
    }

    const tstring& getCurrentDir() const
    {
        return m_currentDir;
    }
};
