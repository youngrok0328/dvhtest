#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/FPI_RECT.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//

class LandInspectionResult
{
public:
    LandInspectionResult();
    void Clear();

public:
    std::vector<CString> vecstrID;
    std::vector<CString> vecstrGroupID; //kircheis_MED2.5

    FPI_RECT sfrtPackageBody;

    std::vector<std::vector<Ipvm::Point32r2>> vec2fptSpecLandbyOrigin;
    std::vector<std::vector<Ipvm::Point32r2>> vec2fptLandbyOrigin;

    std::vector<std::vector<Ipvm::Point32r2>> vec2fptSpecLand;
    std::vector<std::vector<FPI_RECT>> vec2sfrtSpecLand;

    std::vector<std::vector<std::vector<FPI_RECT>>> vec3sfrtSearchLand;
    std::vector<std::vector<std::vector<Ipvm::Point32r2>>> vec3ftEdgeLand;

    std::vector<std::vector<FPI_RECT>> vec2sfrtLand;
    std::vector<std::vector<Ipvm::Point32r2>> vec2fptLandCenter;

    std::vector<std::vector<Ipvm::Rect32s>> vec2rtBlobObj;
    std::vector<std::vector<Ipvm::Point32r2>> vec2fptBlobMassCenter;

    std::vector<std::vector<BOOL>> vec2bAlignResult; //kircheis_LandMissing
    std::vector<std::vector<float>> vec2fWidth;
    std::vector<std::vector<float>> vec2fLength;

    std::vector<std::vector<float>> vec2fAverage;
    std::vector<std::vector<float>> vec2fFMAreaRatio;
};
