//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionBgaBottom3DResult.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Point32r3.h>
#include <Ipvm/Base/Rect32s.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionInspectionBgaBottom3DResult::VisionInspectionBgaBottom3DResult()
{
}

VisionInspectionBgaBottom3DResult::~VisionInspectionBgaBottom3DResult()
{
}

void VisionInspectionBgaBottom3DResult::Init()
{
    m_allBallSpecROIs.clear();
    m_allBallSearchROIs.clear();
    m_allBlobPositions.clear();
    m_allBallPositions.clear();

    m_validBallPositions.clear();
    m_validBallROIs.clear();
    m_validSubstrateROIs.clear();
    m_validSubstrate3DData.clear();

    m_validSubstrateDataZ.clear();

    for (long type = 0; type < long(enumGetherBallZType::END); type++)
    {
        m_validBall3DData[type].clear();
    }
}
