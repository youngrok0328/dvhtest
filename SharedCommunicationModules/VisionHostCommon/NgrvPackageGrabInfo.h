#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "NgrvGrabPointInfo.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __VISION_HOST_COMMON_API__ NgrvPackageGrabInfo
{
public:
    NgrvPackageGrabInfo();
    ~NgrvPackageGrabInfo();

    void Init();

    long m_nTrayID;
    long m_nPackageID;
    CString m_strPackageID;
    long m_nGrabPointNum;
    std::vector<NgrvGrabPointInfo> m_vecGrabPointInfo;

private:
    friend __VISION_HOST_COMMON_API__ CArchive& operator<<(CArchive& ar, const NgrvPackageGrabInfo& src);
    friend __VISION_HOST_COMMON_API__ CArchive& operator>>(CArchive& ar, NgrvPackageGrabInfo& dst);
};
