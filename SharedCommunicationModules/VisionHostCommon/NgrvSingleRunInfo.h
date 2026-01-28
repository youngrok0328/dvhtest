#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "NgrvOnePointGrabInfo.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __VISION_HOST_COMMON_API__ NgrvSingleRunInfo
{
public:
    NgrvSingleRunInfo();
    ~NgrvSingleRunInfo();

    void Init();

    std::vector<NgrvOnePointGrabInfo> m_vecOnePointGrabInfo;

private:
    friend __VISION_HOST_COMMON_API__ CArchive& operator<<(CArchive& ar, const Ipvm::Point32r2& src);
    friend __VISION_HOST_COMMON_API__ CArchive& operator>>(CArchive& ar, Ipvm::Point32r2& dst);
    friend __VISION_HOST_COMMON_API__ CArchive& operator<<(CArchive& ar, const NgrvSingleRunInfo& src);
    friend __VISION_HOST_COMMON_API__ CArchive& operator>>(CArchive& ar, NgrvSingleRunInfo& dst);
};
