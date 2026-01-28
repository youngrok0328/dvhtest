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
class __VISION_HOST_COMMON_API__ NgrvGrabbedImageInfo
{
public:
    NgrvGrabbedImageInfo();
    ~NgrvGrabbedImageInfo();
    void Init();

    long m_nTrayID;
    long m_nPackageID;
    CString m_strPackageID;
    long m_nGrabPointID;
    long m_nSendImageSizeX;
    long m_nSendImageSizeY;
    BOOL m_bTopImage;
    std::vector<CString> m_vecstrImageFileName;

private:
    friend __VISION_HOST_COMMON_API__ CArchive& operator<<(CArchive& ar, const NgrvGrabbedImageInfo& src);
    friend __VISION_HOST_COMMON_API__ CArchive& operator>>(CArchive& ar, NgrvGrabbedImageInfo& dst);
};
