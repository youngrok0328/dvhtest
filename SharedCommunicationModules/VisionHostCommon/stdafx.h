#pragma once

//HDR_0_________________________________ Configuration header
#include "_buildsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxcmn.h> // Windows 공용 컨트롤에 대한 MFC 지원입니다.
#include <afxext.h> // MFC 확장입니다.
#include <afxwin.h> // MFC 핵심 및 표준 구성 요소입니다.

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
#if defined(USE_BY_HANDLER)
// Host의 경우 Surface 항목은 ItemName을 Surface로 SubItem을 원래 항목이름으로 바꾸어 표시하기를 원한다
#define EXCHANGE_SURFACE_ITEM_NAME
#endif

#define __VISION_HOST_COMMON_EXPORT__
