#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "NgrvEachFrameInfo.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __VISION_HOST_COMMON_API__ NgrvFrameInfo
{
public:
    NgrvFrameInfo();
    ~NgrvFrameInfo();

    void Init();

    long m_nBeginIRFrame;
    std::vector<NgrvEachFrameInfo> m_vecEachFrameInfo;

private:
    friend __VISION_HOST_COMMON_API__ CArchive& operator<<(CArchive& ar, const NgrvFrameInfo& src);
    friend __VISION_HOST_COMMON_API__ CArchive& operator>>(CArchive& ar, NgrvFrameInfo& dst);
};
