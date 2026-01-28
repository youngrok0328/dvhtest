#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __VISION_HOST_COMMON_API__ VisionInspectionSurfaceResult
{
public:
    VisionInspectionSurfaceResult();
    ~VisionInspectionSurfaceResult();

public:
    BOOL m_bIs3DVision;
    CString m_strCriteriaName;
    CString m_strCriteriaResult;
    CString m_strCriteriaColor;
    Ipvm::Rect32s m_rtCriteriaROI;
    float m_fCriteriaROICenX;
    float m_fCriteriaROICenY;
    std::vector<CString> m_vecstrCriteriaValue;

    void Init();
    void ResizeCriteria(bool bIs3DVision);
    void SetData(const VisionInspectionSurfaceResult src);

private:
    friend __VISION_HOST_COMMON_API__ CArchive& operator<<(CArchive& ar, const VisionInspectionSurfaceResult& src);
    friend __VISION_HOST_COMMON_API__ CArchive& operator>>(CArchive& ar, VisionInspectionSurfaceResult& dst);
};
