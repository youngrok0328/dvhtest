#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class VisionInspectionSpec;
class VisionInspectionResult;

//HDR_6_________________________________ Header body
//
class __VISION_COMMON_API__ VisionInspectionResultGroup
{
public:
    void Set(const std::vector<VisionInspectionSpec>& visionInspectionSpecs);
    void Add(const std::vector<VisionInspectionSpec>& visionInspectionSpecs);

    VisionInspectionResult* GetResultByGuid(LPCTSTR guid);
    VisionInspectionResult* GetResultByName(LPCTSTR name);
    VisionInspectionResult* GetResultBySpec(const VisionInspectionSpec& spec);

    std::vector<VisionInspectionResult> m_vecResult;
};
