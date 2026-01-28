#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "ResultAlign.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/LineEq32r.h>
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class ImageProcPara;
class Para;
class ParaEdgeParameters;
class VisionDebugInfoGroup;
class VisionImageLot;
class VisionInspFrameIndex;
class VisionInspectionOverlayResult;
class VisionProcessing;
class VisionReusableMemory;
class CPackageSpec;
class CPI_EdgeDetect;

//HDR_6_________________________________ Header body
//

class Inspection
{
public:
    Inspection(VisionProcessing& processor);
    ~Inspection();

    void resetResult();
    bool run(const Para& para, const bool detailSetupMode);

    Ipvm::Rect32s getImageSourceRoiForNoStitch();
    bool GetImageForAlign(const VisionInspFrameIndex& imageFrameIndex, const ImageProcPara& imageProc, Ipvm::Image8u& dstImage);

    void SetGlobalAlignResult();
    void SetGlobalAlignResult_ZeroAngle(const Ipvm::Point32r2& shift);

    void GetOverlayResult(VisionInspectionOverlayResult* overlayResult, long overlayMode, const bool detailSetupMode);
    void AppendTextResult(CString& textResult) const;

    ResultAlign& GetResult();
    float GetCalculationTime() const;
    LPCTSTR GetErrorLogText() const;

private:
    VisionProcessing& m_processor;
    CPackageSpec& m_packageSpec;

    ResultAlign m_result;

    CString m_errorLogText;

    CPI_EdgeDetect* m_edgeDetector = nullptr;
    float m_fCalcTime;

    ResultAlign* GetNormalizedRotationResult(const Ipvm::Point32r2& shift);
    Ipvm::Quadrangle32r Rotate(const Ipvm::Point32r2& origin, float angle_rad, const Ipvm::Quadrangle32r& object) const;
    Ipvm::Point32r2 Rotate(const Ipvm::Point32r2& origin, float angle_rad, const Ipvm::Point32r2& object) const;

    void SetGlobalAlignResult(ResultAlign* result);
    void setDebugInfo(const bool detailSetupMode);
    bool DoAlign(const Para& para, const bool detailSetupMode);

    bool EdgeDetectCarrierTapeX(const Ipvm::Image8u& image, const Para& para, const Ipvm::Point32r2& paneCenter);
    bool EdgeDetectPocket(const Ipvm::Image8u& image, const Para& para, const Ipvm::Point32r2& paneCenter);
    bool EdgeDetectRoi(const Ipvm::Image8u& image, const ParaEdgeParameters& para, const Ipvm::Rect32s& searchRoi,
        int roiDirection, std::vector<Ipvm::Point32r2>& o_edgePointsTotal, std::vector<Ipvm::Point32r2>& o_edgePoints);

    bool RoughAlign_LineAlign(const ParaEdgeParameters& para, long nDirection,
        const std::vector<Ipvm::Point32r2>& vecfpNewPoints, const std::vector<Ipvm::Point32r2>& vecfpFirstEdge,
        const std::vector<Ipvm::Point32r2>& vecfpBestEdge, std::vector<Ipvm::Point32r2>& accumulatedFilteredPoints) const;

    bool SetCarrierAlignResult();
    bool SetPocketAlignResult();
    void calcParallelism();
};
