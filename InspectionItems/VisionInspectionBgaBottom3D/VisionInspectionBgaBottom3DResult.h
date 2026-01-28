#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "ParaDefine.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class VisionInspectionBgaBottom3DResult
{
public:
    VisionInspectionBgaBottom3DResult();
    ~VisionInspectionBgaBottom3DResult();

    void Init();

public:
    // 모든 볼 갯수만큼 가지는 녀석들
    std::vector<Ipvm::Rect32s> m_allBallSpecROIs;
    std::vector<Ipvm::Rect32s> m_allBallSearchROIs;
    std::vector<Ipvm::Point32r2> m_allBlobPositions;
    std::vector<Ipvm::Point32r2> m_allBallPositions;

    // 유효 볼 갯수만큼 가지는 녀석들
    std::vector<Ipvm::Point32r2> m_validBallPositions;
    std::vector<Ipvm::Rect32s> m_validBallROIs;
    std::vector<Ipvm::Rect32s> m_validSubstrateROIs;

    std::vector<Ipvm::Point32r3> m_validBall3DData[long(enumGetherBallZType::END)];
    std::vector<Ipvm::Point32r3> m_validSubstrate3DData;

    std::vector<float> m_validSubstrateDataZ;
};
