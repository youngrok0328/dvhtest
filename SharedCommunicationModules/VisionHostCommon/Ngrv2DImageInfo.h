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
class __VISION_HOST_COMMON_API__ Ngrv2DImageInfo
{
public:
    Ngrv2DImageInfo();
    ~Ngrv2DImageInfo();

    void Init();

    long m_visionID;
    long m_imageSizeX;
    long m_imageSizeY;
    long m_totalImageSize;
    float m_scaleX;
    float m_scaleY;
    BOOL m_isImageColor;
    std::vector<BYTE> m_vecbyImage;

private:
    friend __VISION_HOST_COMMON_API__ CArchive& operator<<(CArchive& ar, const Ngrv2DImageInfo& src);
    friend __VISION_HOST_COMMON_API__ CArchive& operator>>(CArchive& ar, Ngrv2DImageInfo& dst);
};
