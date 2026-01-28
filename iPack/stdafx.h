#pragma once

//HDR_0_________________________________ Configuration header
#include "_buildsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../Preprocessor.h"

//HDR_3_________________________________ External library headers
#include <afxcmn.h> // Windows 공용 컨트롤에 대한 MFC 지원입니다.
#include <afxcontrolbars.h> // MFC의 리본 및 컨트롤 막대 지원
#include <afxext.h> // MFC 확장입니다.
#include <afxwin.h> // MFC 핵심 및 표준 구성 요소입니다.

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker, \
    "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker, \
    "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker, \
    "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

static const UINT MsgSendFinalREsultDataVsToMv = ::RegisterWindowMessage(_T("UserMessageSendFinalResultDataToMv"));
static const UINT MsgSendDataCheck = ::RegisterWindowMessage(_T("UserMessageSendDataCheck"));
