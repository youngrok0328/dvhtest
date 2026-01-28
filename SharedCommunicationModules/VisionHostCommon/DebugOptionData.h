#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class ArchiveAllType;

//HDR_6_________________________________ Header body
//
static LPCTSTR g_DebugOptionString[] = {
    _T("EMPTY"),
    _T("INVALID"),
    _T("PASS"),
    _T("NOTPASS"),
    _T("UNCONDITIONAL"),
};

class __VISION_HOST_COMMON_API__ DebugOptionData
{
public:
    DebugOptionData();
    ~DebugOptionData();
    void Init();
    void Serialize(ArchiveAllType& ar);

public:
    enum DebugOptionType
    {
        HANDLER_STOP,
        IMAGE_SAVE
    };

    enum debug_t
    {
        DEBUG_BEGIN_,
        DEBUG_EMPTY = DEBUG_BEGIN_,
        DEBUG_INVALID,
        DEBUG_PASS,
        DEBUG_NOTPASS,
        DEBUG_UNCONDITIONAL,
        DEBUG_END_,
    };

    BOOL bUse;
    long nTotalItemCheckStatus;

    // 여기에 어떤걸 할건지 결정
    std::vector<long> vecnItemCheckStatus;
};
