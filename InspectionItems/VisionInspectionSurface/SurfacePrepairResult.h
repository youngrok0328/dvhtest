#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image32r.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Point32r2.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class SurfacePrepairResult
{
public:
    SurfacePrepairResult(const bool detailSetupMode);
    ~SurfacePrepairResult();

    bool isDetailSetupMode() const;

    // 공통
    bool m_success;
    bool m_is3D;
    Ipvm::Image8u m_grayImage;
    Ipvm::Image8u m_maskImage;
    Ipvm::Image8u m_backgroundImage;

    // 2D
    Ipvm::Image8u m_aux1GrayImage;
    Ipvm::Image8u m_aux2GrayImage;

    // 3D
    Ipvm::Image32r m_heightMap;

    // Detail Setup 결과
    std::vector<Ipvm::Point32r2> m_profiles;

private:
    bool m_detailSetupMode;
};
