#include "stdafx.h"
#include "clickpad.h"
#include "selectImageDlg.h"

void ImageCollection::scanImages()
{
    u8string sdir;
    base::getResource(getLuaState(), "", &sdir);
    tstring dir(TU2W(sdir.c_str()));


    

    WIN32_FIND_DATA fd;
    memset(&fd, 0, sizeof(WIN32_FIND_DATA));
    HANDLE file = FindFirstFile(L"*.*", &fd);
    if (file != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                if (Plugin::isPlugin(fd.cFileName))
                    files.push_back(fd.cFileName);
            }
        } while (::FindNextFile(file, &fd));
        ::FindClose(file);
    }
}