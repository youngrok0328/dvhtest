//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Win32Error.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CString Win32Error::GetErrorMessage(DWORD code)
{
    TCHAR message[10 * MAX_PATH]{};

    ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, code, 0, message, 10 * MAX_PATH, nullptr);

    return CString{message};
}
