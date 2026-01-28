#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "NgrvSingleRunInfo.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __VISION_HOST_COMMON_API__ NgrvGrabInfo
{
public:
    NgrvGrabInfo();
    ~NgrvGrabInfo();

    void Init();

    long m_nTrayID;
    long m_nPackageID;
    CString m_strPackageID;
    std::vector<NgrvSingleRunInfo> m_vecNGRVSingleRunInfo;

private:
    friend __VISION_HOST_COMMON_API__ CArchive& operator<<(CArchive& ar, const NgrvGrabInfo& src);
    friend __VISION_HOST_COMMON_API__ CArchive& operator>>(CArchive& ar, NgrvGrabInfo& dst);
};
