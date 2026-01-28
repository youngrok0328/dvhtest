#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __VISION_HOST_COMMON_API__ VisionWarpageShapeResult
{
public:
    VisionWarpageShapeResult();
    VisionWarpageShapeResult(CString strSign, CString strShape, double R, double B4_um, double B5_um, double angle_deg);
    ~VisionWarpageShapeResult();
    VisionWarpageShapeResult& operator=(const VisionWarpageShapeResult& rhs) = default;

public:
    void Init();
    void GetResult(CString& strSign, CString& strShape, double& R, double& B4_um, double& B5_um, double& angle_deg);
    void SetResult(CString strSign, CString strShape, double R, double B4_um, double B5_um, double angle_deg);
    void SetResult(VisionWarpageShapeResult visionWarpageShapeResult);

    CString m_sign;
    CString m_shape;
    double m_R;
    double m_B4_um;
    double m_B5_um;
    double m_angle_deg;

private:
    friend __VISION_HOST_COMMON_API__ CArchive& operator<<(CArchive& ar, const VisionWarpageShapeResult& src);
    friend __VISION_HOST_COMMON_API__ CArchive& operator>>(CArchive& ar, VisionWarpageShapeResult& dst);
};
