#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __VISION_HOST_COMMON_API__ NgrvOnePointGrabInfo
{
public:
    NgrvOnePointGrabInfo();
    ~NgrvOnePointGrabInfo();

    void Init();

    CString m_defectPositionName;
    CString m_itemID;
    CString m_defectID;
    CString m_defectCode;
    CString m_disposition;
    CString m_decision;
    long m_imageSizeX;
    long m_imageSizeY;
    long m_imageCenterX;
    long m_imageCenterY;
    long m_afType;
    float m_grabHeightOffset_um;
    long m_stitchX;
    long m_stitchY;
    std::vector<BOOL> m_vecIllumUse;
    std::vector<Ipvm::Point32r2> m_vecPointGrabPosition_um;

private:
    friend __VISION_HOST_COMMON_API__ CArchive& operator<<(CArchive& ar, const Ipvm::Point32r2& src);
    friend __VISION_HOST_COMMON_API__ CArchive& operator>>(CArchive& ar, Ipvm::Point32r2& dst);
    friend __VISION_HOST_COMMON_API__ CArchive& operator<<(CArchive& ar, const NgrvOnePointGrabInfo& src);
    friend __VISION_HOST_COMMON_API__ CArchive& operator>>(CArchive& ar, NgrvOnePointGrabInfo& dst);
};
