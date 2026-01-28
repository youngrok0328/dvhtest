#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Quadrangle.h>
#include <Ipvm/Base/Rect32r.h>

//HDR_4_________________________________ Standard library headers
#include <array>
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class ResultDevice
{
public:
    // Spec 정보로 가공된 데이터
    float m_specPacketSizeX_px = 0.f;
    float m_specPacketSizeY_px = 0.f;
    Ipvm::Rect32r m_specPacketRect_px = {0.f, 0.f, 0.f, 0.f};
    std::vector<Ipvm::Rect32s> m_searchPocketRects;

    // Align 정보로 가공된 데이터
    std::vector<Ipvm::Point32r2> m_edgePointsTotal; // 검출된 Edge Point 들 (전체)
    std::vector<Ipvm::Point32r2> m_edgePointsFiltered; // 검출된 Edge Point 들 (필터링된것)

    Ipvm::Quadrangle32r m_alignedPacket{Ipvm::k_noiseValue32r, Ipvm::k_noiseValue32r, Ipvm::k_noiseValue32r,
        Ipvm::k_noiseValue32r, Ipvm::k_noiseValue32r, Ipvm::k_noiseValue32r, Ipvm::k_noiseValue32r,
        Ipvm::k_noiseValue32r};
};
