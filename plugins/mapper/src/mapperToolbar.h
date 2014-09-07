#pragma once

class MapperToolbar : public CDialogImpl<MapperToolbar>, public CMessageFilter
{

public:
    enum { IDD = IDD_MAPPER_TOOLBAR };    
    BOOL PreTranslateMessage(MSG* pMsg) {
        return CWindow::IsDialogMessage(pMsg); 
    }

	BEGIN_MSG_MAP(MapperToolbar)
		//MESSAGE_HANDLER(WM_INITDIALOG, OnCreate)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

   

private:
	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&bHandled)
	{   
        

        bHandled = FALSE;
        return 0;
	}

    LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&bHandled)
	{
       
        bHandled = FALSE;
        return 0;
    }
};
