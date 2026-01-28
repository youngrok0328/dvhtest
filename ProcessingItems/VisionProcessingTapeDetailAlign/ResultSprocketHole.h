#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Rect32r.h>
#include <Ipvm/Base/Rect.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class ResultSprocketHole
{
public:
    // Spec 정보로 가공된 데이터
    float m_specHoleSizeX_px = 0.f;
    float m_specHoleSizeY_px = 0.f;
    Ipvm::Rect32r m_specHoleRect_px = {0.f, 0.f, 0.f, 0.f};
    Ipvm::Rect32r m_searchHoleRect_px = {0.f, 0.f, 0.f, 0.f};
    std::vector<Ipvm::LineSeg32r> m_searchHoleLines_px; // Search Line 들

    // Align 정보로 가공된 데이터
    std::vector<Ipvm::Point32r2> m_edgePoints; // 검출된 Edge Point 들
    Ipvm::EllipseEq32r m_alignedHole{
        Ipvm::k_noiseValue32r, Ipvm::k_noiseValue32r, Ipvm::k_noiseValue32r, Ipvm::k_noiseValue32r};
    Ipvm::Rect m_alignedHoleROI = {0, 0, 0, 0};
};
