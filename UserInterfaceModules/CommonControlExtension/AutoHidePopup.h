#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
enum class AutoHidePopupType
{
    Normal,
    Error,
};

class __COMMON_CONTROL_EXTENSION_CLASS__ AutoHidePopup
{
public:
    static void add(AutoHidePopupType type, const wchar_t* format, ...);
};
