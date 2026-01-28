#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "Result.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CPI_Blob;
class CPI_EdgeDetect;
class ImageProcPara;
class Para;
class ParaDeviceAlign;
class ParaEdgeParameters;
class ParaEdgeSearchRoi;
class ParaPocketAlign;
class ParaSprocketHoleAlign;
class SurfaceLayerMask;
class VisionInspFrameIndex;
class VisionProcessing;
class VisionTapeSpec;
//HDR_6_________________________________ Header body
//
class Inspection
{
public:
    Inspection(VisionProcessing& processor);
    void ResetResult();
    bool Align(const bool detailSetupMode, const Para& para);

    Result m_result;

private:
    VisionProcessing& m_processor;
    CPI_EdgeDetect* m_edgeDetector = nullptr;

    bool GetImageForAlign(const VisionInspFrameIndex& imageFrameIndex, const ImageProcPara& imageProc, Ipvm::Image8u& o_image);
    bool AlignSprocketHole(const bool detailSetupMode, const VisionTapeSpec& tapeSpec, const Para& para);
    bool AlignPocket(const bool detailSetupMode, const VisionTapeSpec& tapeSpec, const Para& para);
    bool AlignDevice(const bool detailSetupMode, const Para& para);

    bool MakeSpecSprocketHoleRois(const bool detailSetupMode, const VisionTapeSpec& tapeSpec);
    void MakeSpecPocketRoi(const bool detailSetupMode, const VisionTapeSpec& tapeSpec);
    void MakeSpecDeviceRoi(const bool detailSetupMode);

    bool MakeSearchSprocketHoleRois(const bool detailSetupMode, const VisionTapeSpec& tapeSpec, const Para& para);
    bool MakeSearchPocketRoi(const bool detailSetupMode, const VisionTapeSpec& tapeSpec, const Para& para);
    bool MakeSearchDeviceRoi(const bool detailSetupMode, const Para& para);

    bool FindSprocketHoleRois(const Ipvm::Image8u& image, const bool detailSetupMode, const ParaSprocketHoleAlign& para);
    bool FindPocketRoi(const Ipvm::Image8u& image, const bool detailSetupMode, const ParaPocketAlign& para);
    bool FindDeviceRoi(const Ipvm::Image8u& image, const bool detailSetupMode, const ParaDeviceAlign& para);
    bool FindRectangleUsingEdgeDetection(const Ipvm::Image8u& image,
        const ParaEdgeParameters& edgeParams,
        const std::vector<Ipvm::Rect32s>& edgeSearchRois, std::vector<Ipvm::Point32r2>& edgePointsTotal,
        std::vector<Ipvm::Point32r2>& edgePointsFiltered,
        Ipvm::Quadrangle32r& alignedRect);

    void CalculateSprocketOffset();

    bool EdgeDetectRoi(const Ipvm::Image8u& image, const ParaEdgeParameters& para, const Ipvm::Rect32s& searchRoi,
        int roiDirection, std::vector<Ipvm::Point32r2>& o_edgePointsTotal, std::vector<Ipvm::Point32r2>& o_edgePoints);

    bool RoughAlign_LineAlign(const ParaEdgeParameters& para, long nDirection,
        const std::vector<Ipvm::Point32r2>& vecfpNewPoints, const std::vector<Ipvm::Point32r2>& vecfpFirstEdge,
        const std::vector<Ipvm::Point32r2>& vecfpBestEdge, std::vector<Ipvm::Point32r2>& accumulatedFilteredPoints) const;
};
