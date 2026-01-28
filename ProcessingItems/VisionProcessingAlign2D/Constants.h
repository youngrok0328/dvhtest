#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class VisionProcessingAlign2D;

//HDR_6_________________________________ Header body
//
class Constants
{
public:
    Constants(VisionProcessingAlign2D* processor);
    ~Constants();

    bool Make();

    long m_rawImageSizeX;
    long m_rawImageSizeY;
    Ipvm::Point32r2 m_rawImageCenter;

    float m_packageSizeX_px;
    float m_packageSizeY_px;
    float m_floatOverlapSizeX_px;
    float m_floatOverlapSizeY_px;

    long m_stitchCountX;
    long m_stitchCountY;
    long m_sensorSizeX;
    long m_sensorSizeY;
    float m_foverlapUmX;
    float m_foverlapUmY;
    long m_overlapPixelX;
    long m_overlapPixelY;
    long m_InterpolationLengthX;
    long m_InterpolationLengthY;
    long m_InterpolationOffsetX;
    long m_InterpolationOffsetY;
    long m_frameIndexForStitchInfo;

    // Overlap ROI (현재 FOV를 왼쪽에 붙일때, Dst 의 overlap ROI와  현재 FOV의 overlap ROI)
    const Ipvm::Rect32s& GetHorPreOverlapROI(long stitchIndex) const;
    const Ipvm::Rect32s& GetHorCurOverlapROI(long stitchIndex) const;

    // Overlap ROI (현재 FOV를 위쪽에 붙일때, Dst 의 overlap ROI와  현재 FOV의 overlap ROI)
    const Ipvm::Rect32s& GetVerPreOverlapROI(long stitchIndex) const;
    const Ipvm::Rect32s& GetVerCurOverlapROI(long stitchIndex) const;
    const Ipvm::Rect32s& GetSimpleCopyRoi(long stitchIndex) const;

private:
    VisionProcessingAlign2D* m_processor;

    struct PreapreStitchInfo
    {
        Ipvm::Rect32s m_rtHorPre;
        Ipvm::Rect32s m_rtHorCur;
        Ipvm::Rect32s m_rtVerPre;
        Ipvm::Rect32s m_rtVerCur;

        Ipvm::Rect32s m_rtSimpleCopy; // 현재 FOV에서 interpolation이 필요없이 단순 복사할 영역
    };

    std::vector<PreapreStitchInfo> m_prepareStitchInfo;

    void MakePrepareStitchInfo(long x, long y, PreapreStitchInfo& info);
};
