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
class __VISION_HOST_COMMON_API__ NgrvAfRefInfo
{
public:
    NgrvAfRefInfo();
    ~NgrvAfRefInfo();

    void Init();
    void SetImageSize(long nSizeX, long nSizeY);

    long m_nVisionID;
    BOOL m_bIsValidPlaneRefInfo;
    std::vector<CPoint> m_vecptRefPos_UM;
    long m_nImageSizeX;
    long m_nImageSizeY;
    std::vector<BYTE> m_vecbyImage;

private:
    friend __VISION_HOST_COMMON_API__ CArchive& operator<<(CArchive& ar, const NgrvAfRefInfo& src);
    friend __VISION_HOST_COMMON_API__ CArchive& operator>>(CArchive& ar, NgrvAfRefInfo& dst);
};
