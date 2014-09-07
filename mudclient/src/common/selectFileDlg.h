#pragma once

class SelectFileDlg
{
    HWND m_parentWnd;
    std::wstring m_mask;
    std::wstring m_file;
    bool m_multiselect;
    std::wstring m_workdir;
public:
    enum { SFILE_OK = 0, SFILE_CANCEL };

    SelectFileDlg(HWND parent, const std::wstring& mask, bool multiselect=false) :  
    m_parentWnd(parent), m_mask(mask), m_multiselect(multiselect)
    {
        WCHAR path[MAX_PATH+1];
        GetCurrentDirectory(MAX_PATH, path);
        m_workdir.assign(path);
    }
    ~SelectFileDlg() 
    {
        SetCurrentDirectory(m_workdir.c_str());    
    }

    std::wstring GetFile() const
    {
        return m_file; 
    }

    bool DoModal()
    {
        OPENFILENAMEW ofn;
        int path_size = 65536;
        wchar_t *szFile = new wchar_t[path_size+1];
        szFile[0] = 0;

        memset(&ofn, 0, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.lpstrFile = szFile;
        ofn.hwndOwner = m_parentWnd;
        ofn.nMaxFile = path_size;

        int len = m_mask.size()+1;
        wchar_t *buffer = new wchar_t[len];
        wcscpy(buffer, m_mask.c_str());
        for (int i=0; i<len; ++i)
        {
            if (buffer[i] == L'|')
                buffer[i] = 0;
        }                

        ofn.lpstrFilter = buffer;
        ofn.nFilterIndex = 1;
        ofn.lpstrInitialDir = NULL;
        ofn.lpstrFileTitle = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
        if (m_multiselect)
            ofn.Flags |= OFN_ALLOWMULTISELECT;

        BOOL result = GetOpenFileNameW(&ofn);
        if (result)
        {   
            if (!m_multiselect)
            {
                m_file.assign(szFile);
                return true;
            }

            int len = 0;
            wchar_t *p = szFile;

            while(true) 
            {   wchar_t s = *p;
                 wchar_t s2 = *(p+1);
                 if (s == 0 && s2 == 0)
                     break;
                 len++; p++;
            } 

            std::vector<int> indexes;
            for (int i=0; i<len; ++i)
            {
                if (szFile[i]==0) 
                    indexes.push_back(i);
            }

            if (indexes.size() == 0)
                m_file.assign(szFile);
            else
            {
                std::wstring path(szFile);
                int last = path.size() - 1;
                if (path.at(last) != L'\\')
                    path.append(L"\\");
                for (int i=0,e=indexes.size(); i<e; ++i)
                {
                    int pos = indexes[i] + 1;
                    std::wstring file(path);
                    file.append(&szFile[pos]);
                    m_file.append(file);
                    if (i != (e-1))
                        m_file.append(L"|");
                }
            }
        }
        delete []buffer;
        delete []szFile;         
        return result ? true : false;

    }
};
