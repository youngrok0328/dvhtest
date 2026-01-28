#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Rect32r.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class StitchMatchingInfo
{
public:
    StitchMatchingInfo();

    double m_angle_diff;
    double m_shiftX;
    double m_shiftY;
    double m_rotate_origin_x;
    double m_rotate_origin_y;

    //---------------------------------------------------------------
    // Matching - Debugging 결과
    //---------------------------------------------------------------

    double m_curCenX;
    double m_curCenY;
    float m_fMatchScore[2];
    Ipvm::Rect32r m_rtMatched[2]; // 찾은 Rect 위치정보를 갖는다
    Ipvm::Point32r2 m_ptShift[2]; // 찾은 위치정보를 갖는다
};

class StitchResultROI
{
public:
    StitchMatchingInfo m_AccMatchingInfo; // accumulated matching info.

    StitchMatchingInfo m_verMatchingInfo;
    StitchMatchingInfo m_horMatchingInfo;

    void UpdateMatchingInfo();
    void UpdateAccumulateMatchingInfoForMatching(StitchResultROI& preVerInfo, StitchResultROI& preHorInfo);
    double GetAngle();
    Ipvm::Point32r2 GetOrigin();
    Ipvm::Point32r2 GetShift();
};
