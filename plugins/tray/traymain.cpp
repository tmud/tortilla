#include "stdafx.h"

void show_message(const u8string& msg)
{
    MessageBox(NULL, TU2W(msg.c_str()), L"test", MB_OK);
}
