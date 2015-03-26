#pragma once

class ConstructorWnd : public CWindowImpl <ConstructorWnd>
{
public:

private:
    BEGIN_MSG_MAP(ConstructorWnd)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
    END_MSG_MAP()

    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&bHandled)
    {
        return 0;
    }

    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&bHandled)
    {
        return 0;
    }
};
