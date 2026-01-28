#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/EllipseEq32r.h> //kircheis_MED3
#include <Ipvm/Base/LineSeg32r.h>
#include <Ipvm/Base/Point32r2.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class Result
{
public:
    Result();
    ~Result();

    void reset();

    std::vector<Ipvm::LineSeg32r> m_rough_vecSweepLine;
    std::vector<Ipvm::Point32r2> m_rough_vecfptBallWidthEdgeTotalPoint;
    std::vector<Ipvm::EllipseEq32r> m_rough_vecellipseAlignCircle; //kircheis_MED3

    std::vector<Ipvm::LineSeg32r> m_vecSweepLine;
    std::vector<Ipvm::Point32r2> m_vecfptBallWidthEdgeTotalPoint;
    std::vector<Ipvm::Point32r2> m_vecfptBallWidthEdgeValidPoint;
};
