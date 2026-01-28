//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "StitchPara_Structure.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
StitchMatchingInfo::StitchMatchingInfo()
    : m_angle_diff(0.f)
    , m_shiftX(0.f)
    , m_shiftY(0.f)
    , m_rotate_origin_x(0.f)
    , m_rotate_origin_y(0.f)
{
}

void StitchResultROI::UpdateMatchingInfo()
{
    m_AccMatchingInfo.m_angle_diff = (m_verMatchingInfo.m_angle_diff + m_horMatchingInfo.m_angle_diff) * 0.5;
    m_AccMatchingInfo.m_shiftX = (m_verMatchingInfo.m_shiftX + m_horMatchingInfo.m_shiftX) * 0.5;
    m_AccMatchingInfo.m_shiftY = (m_verMatchingInfo.m_shiftY + m_horMatchingInfo.m_shiftY) * 0.5;
    m_AccMatchingInfo.m_rotate_origin_x
        = (m_verMatchingInfo.m_rotate_origin_x + m_horMatchingInfo.m_rotate_origin_x) * 0.5;
    m_AccMatchingInfo.m_rotate_origin_y
        = (m_verMatchingInfo.m_rotate_origin_y + m_horMatchingInfo.m_rotate_origin_y) * 0.5;
}
void StitchResultROI::UpdateAccumulateMatchingInfoForMatching(StitchResultROI& preVerInfo, StitchResultROI& preHorInfo)
{
    auto& preVerAccInfo = preVerInfo.m_AccMatchingInfo;
    auto& preHorAccInfo = preHorInfo.m_AccMatchingInfo;

    // 원래는 (PreVer.angle + curVer.angle ) 과 ( preHor.angle + curHor.angle) 의 평균값, 아래와 결과식이 같다.
    m_AccMatchingInfo.m_angle_diff += (preVerAccInfo.m_angle_diff + preHorAccInfo.m_angle_diff) * 0.5;

    m_AccMatchingInfo.m_shiftX += (preVerAccInfo.m_shiftX + preHorAccInfo.m_shiftX) * 0.5;
    m_AccMatchingInfo.m_shiftY += (preVerAccInfo.m_shiftY + preHorAccInfo.m_shiftY) * 0.5;

    //// 회전중심위치는 변하지 않는다 ??
    // 회전중심위치는 누적된 Shift 값 만큼 이동하는게 맞을 듯
    m_AccMatchingInfo.m_rotate_origin_x += (preVerAccInfo.m_shiftX + preHorAccInfo.m_shiftX) * 0.5;
    m_AccMatchingInfo.m_rotate_origin_y += (preVerAccInfo.m_shiftY + preHorAccInfo.m_shiftY) * 0.5;
}

double StitchResultROI::GetAngle()
{
    //		return (m_verMatchingInfo.m_angle_diff + m_horMatchingInfo.m_angle_diff)*0.5;
    return m_AccMatchingInfo.m_angle_diff;
}

Ipvm::Point32r2 StitchResultROI::GetOrigin()
{
    //		return Ipvm::Point32r2((float)((m_verMatchingInfo.m_rotate_origin_x + m_horMatchingInfo.m_rotate_origin_x) * 0.5), (float)((m_verMatchingInfo.m_rotate_origin_y + m_horMatchingInfo.m_rotate_origin_y) * 0.5));
    return Ipvm::Point32r2((float)(m_AccMatchingInfo.m_rotate_origin_x), (float)(m_AccMatchingInfo.m_rotate_origin_y));
}

Ipvm::Point32r2 StitchResultROI::GetShift()
{
    //		return Ipvm::Point32r2((float)((m_verMatchingInfo.m_shiftX + m_horMatchingInfo.m_shiftX) * 0.5), (float)((m_verMatchingInfo.m_shiftY + m_horMatchingInfo.m_shiftY) * 0.5) );
    return Ipvm::Point32r2((float)(m_AccMatchingInfo.m_shiftX), (float)(m_AccMatchingInfo.m_shiftY));
}
