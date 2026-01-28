#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "StitchPara_Structure.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class StitchResult
{
public:
    StitchResult();
    ~StitchResult();

    bool Prepare(long stitchCount);

    CString m_additionalCriticalLog;

    std::vector<StitchResultROI> m_stitchRois;

    long m_nSaveTemplateImageIndex;
    long m_nSaveInterpolationImageIndex;
};
