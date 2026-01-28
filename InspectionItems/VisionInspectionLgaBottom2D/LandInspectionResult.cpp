//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "LandInspectionResult.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Rect32s.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
LandInspectionResult::LandInspectionResult()
{
    Clear();
}

void LandInspectionResult::Clear()
{
    sfrtPackageBody = FPI_RECT(
        Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f));

    vecstrID.clear();
    vecstrGroupID.clear(); //kircheis_MED2.5

    vec2fptSpecLandbyOrigin.clear();
    vec2fptLandbyOrigin.clear();

    vec2fptSpecLand.clear();
    vec2sfrtSpecLand.clear();

    vec3sfrtSearchLand.clear();
    vec3ftEdgeLand.clear();
    vec2sfrtLand.clear();
    vec2fptLandCenter.clear();

    vec2rtBlobObj.clear();
    vec2fptBlobMassCenter.clear();

    vec2bAlignResult.clear(); //kircheis_LandMissing
    vec2fWidth.clear();
    vec2fLength.clear();

    vec2fAverage.clear();
    vec2fFMAreaRatio.clear();
}
