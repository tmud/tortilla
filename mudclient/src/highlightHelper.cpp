#include "stdafx.h"
#include "highlightHelper.h"

HighlightHelperImpl HighlightHelper::m_impl;
bool HighlightHelper::checkText(tstring* param)
{
    return m_impl.checkText(param);
}
