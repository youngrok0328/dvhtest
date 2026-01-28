#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32r.h>
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class VisionProcessingAlign3D;

//HDR_6_________________________________ Header body
//
class Result
{
public:
    Result();
    ~Result();

    struct SFovInfo
    {
        Ipvm::Rect32r m_deviceImageRoi;
        Ipvm::Rect32s m_imageRoi;
        Ipvm::Rect32r m_fovRoi_mm;
        Ipvm::Rect32r m_validUnitRoi_mm;
    };

    int m_fovImageSizeX;
    int m_fovImageSizeY;
    float m_deviceLeftInImage;
    float m_deviceRightInImage;
    double m_paneOffsetX_um;
    double m_paneOffsetY_um;

    std::vector<SFovInfo> m_fovs;
    void Prepare(VisionProcessingAlign3D& process, double paneOffsetX_um, double paneOffsetY_um);
};
