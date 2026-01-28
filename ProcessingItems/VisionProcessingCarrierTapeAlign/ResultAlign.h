#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "Enum.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/FPI_RECT.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/LineEq32r.h>
#include <Ipvm/Base/Quadrangle32r.h>

//HDR_4_________________________________ Standard library headers
#include <array>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class ResultAlign : public VisionAlignResult
{
public:
    ResultAlign();
    ~ResultAlign() override = default;

    void Init(const Ipvm::Point32r2& ptCenter);
    void SetImage(
        const Ipvm::Image8u& imageCarrierTape, const Ipvm::Image8u& imagePocket, const Ipvm::Point32r2& paneCenter);
    void FreeMemoryForCalculation();

    const Ipvm::Image8u& getImage(bool isPocket) const;
    const Ipvm::Point32r2& getPaneCenter() const;

    // Edge Search ROIs
    std::array<Ipvm::Rect32s, 2> m_carrierTapeEdgeSearchRois; // 0:Left, 1:Right
    std::array<Ipvm::Rect32s, 4> m_pocketEdgeSearchRois; // 0:UP, 1:DOWN, 2:LEFT, 3:RIGHT

    std::vector<Ipvm::LineEq32r> m_carrierTapeRefLine; // 0:Left, 1:Right
    std::array<Ipvm::LineEq32r, 4> m_pocketRefLine; // 0:UP, 1:DOWN, 2:LEFT, 3:RIGHT

    Ipvm::Quadrangle32r m_carrierTapeInspectionArea = {0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.f};
    Ipvm::Point32r2 m_roughPocketCenter_px;

    // Final Edge Point
    std::array<std::vector<Ipvm::Point32r2>, 2> m_carrierTapeEdgePoints;
    std::array<std::vector<Ipvm::Point32r2>, 4> m_pocketEdgePoints; // 0:UP, 1:DOWN, 2:LEFT, 3:RIGHT

    // Total Edge Point
    std::array<std::vector<Ipvm::Point32r2>, 2> m_carrierTapeEdgePointsTotal;
    std::array<std::vector<Ipvm::Point32r2>, 4> m_pocketEdgePointsTotal; // 0:UP, 1:DOWN, 2:LEFT, 3:RIGHT

    Ipvm::Quadrangle32r m_roughPocketAlignQuadRect; // 핸들러에 전송 필요
    Ipvm::Rect m_roughPocketAlignRect; // 핸들러에 전송 필요

    bool m_inspectioned;
    bool m_success;

private:
    Ipvm::Image8u m_imageCarrierTape;
    Ipvm::Image8u m_imagePocket;
    Ipvm::Point32r2 m_paneCenter;
};
