#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32r.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __VISION_COMMON_API__ VisionAlignResult
{
public:
    VisionAlignResult();
    virtual ~VisionAlignResult();

    void Init(const Ipvm::Point32r2& center);
    void SetAngle(float _fAngle_rad);
    void SetBodyAlign_AngleZero(const Ipvm::Point32r2& shift);

    Ipvm::Rect32r getBodyRect() const;
    Ipvm::Rect32s getBodyRect32s() const;

public:
    //-------------------------------------------------------------
    // Analysis 결과
    //-------------------------------------------------------------
    float m_analysis_packageOffsetX_um;
    float m_analysis_packageOffsetY_um;
    std::vector<int> m_analysis_histogramZ;

    //-------------------------------------------------------------
    // Body Align 결과
    //-------------------------------------------------------------

    BOOL bAvailable;
    Ipvm::Point32r2 m_center;
    float m_angle_rad;
    float fCosVal;
    float fSinVal;

    Ipvm::Point32r2 fptLT;
    Ipvm::Point32r2 fptRT;
    Ipvm::Point32r2 fptLB;
    Ipvm::Point32r2 fptRB;
    Ipvm::Point32r2 fptLowtopLT;
    Ipvm::Point32r2 fptLowtopRT;

    std::vector<float> vecfParallelismDistLeft;
    std::vector<float> vecfParallelismDistRight;
    std::vector<float> vecfParallelismDistTop;
    std::vector<float> vecfParallelismDistBottom;

    //-------------------------------------------------------------
    // Ball 2D
    //-------------------------------------------------------------

    std::vector<float> m_ballConstrasts;
    std::vector<Ipvm::Rect32s> m_ballConstrastROIs;
    std::vector<float> m_ballWidths_px;
    std::vector<Ipvm::Point32r2> m_ballWidthCenterPos_px;

    Ipvm::Point32r2 rotate(const Ipvm::Point32r2& origin, float angle_rad, const Ipvm::Point32r2& object);
};
