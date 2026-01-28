//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "AutoHidePopup.h"

//CPP_2_________________________________ This project's headers
#include "AutoHidePopupEngine.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
static AutoHidePopupEngine g_popupEngine;

void AutoHidePopup::add(AutoHidePopupType type, const wchar_t* format, ...)
{
    CString message;

    va_list argList;
    va_start(argList, format);
    message.FormatV(format, argList);
    va_end(argList);

    g_popupEngine.add(type, message);
}
