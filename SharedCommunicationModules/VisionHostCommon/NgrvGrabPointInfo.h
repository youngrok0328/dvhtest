#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "NgrvDefectData.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __VISION_HOST_COMMON_API__ NgrvGrabPointInfo
{
public:
    NgrvGrabPointInfo();
    ~NgrvGrabPointInfo();

    void Init();

    CString m_strInspModuleName = _T("empty");
    CString m_strInspItemName = _T("empty");
    CString m_strReviewSide = _T("BOTRV");
    long m_nStitchingCountX = 1;
    long m_nStitchingCountY = 1;
    float m_fOverlapLengthX = 0.f;
    float m_fOverlapLengthY = 0.f;
    std::vector<Ipvm::Rect32s> m_vecImageGrabFOV;
    std::vector<std::vector<NgrvDefectData>> m_vecDefectInNGRVFov; // FOV¾EAC DefectAC A¤º¸μe

private:
    friend __VISION_HOST_COMMON_API__ CArchive& operator<<(CArchive& ar, const NgrvGrabPointInfo& src);
    friend __VISION_HOST_COMMON_API__ CArchive& operator>>(CArchive& ar, NgrvGrabPointInfo& dst);
};
