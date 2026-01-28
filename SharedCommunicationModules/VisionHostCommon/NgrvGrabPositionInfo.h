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
class __VISION_HOST_COMMON_API__ NgrvGrabPositionInfo
{
public:
    enum enumNGRVAFMode
    {
        NGRV_AF_MODE_START = 0,
        NGRV_AF_MODE_EACH_POINT = NGRV_AF_MODE_START,
        NGRV_AF_MODE_USE_PLANE,
    };

    NgrvGrabPositionInfo();
    ~NgrvGrabPositionInfo();

    void Init();

    CString m_strModuleName;
    BOOL m_bIsSaveImage;
    long m_nNormalFrameNum;
    long m_nIRFrameNum;
    long m_nAFType;
    long m_nPlaneOffsetUM;

private:
    friend __VISION_HOST_COMMON_API__ CArchive& operator<<(CArchive& ar, const NgrvGrabPositionInfo& src);
    friend __VISION_HOST_COMMON_API__ CArchive& operator>>(CArchive& ar, NgrvGrabPositionInfo& dst);
};
