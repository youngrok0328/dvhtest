#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __VISION_COMMON_API__ DevelopmentLog
{
public:
    enum class Type
    {
        Warning,
        Develop,
        Notice,
        Event,
        Thread,
        TCP,
        ImageGrab,
        End
    };

    static bool IsTypeEnable(Type type);
    static void SetTypeEnabled(Type type, bool enable);
    static void AddLog(Type type, LPCTSTR szText, ...);
    static void PopLog(std::vector<std::pair<Type, CString>>& o_log);
    static void SetMonitoringHwnd(HWND hwnd, UINT message);
};
