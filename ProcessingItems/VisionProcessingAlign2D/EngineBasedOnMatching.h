#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class Constants;
class CPackageSpec;
class StitchMatchingInfo;
class StitchPara;
class StitchResult;
class StitchPara_BasedOnMatching;
class VisionProcessingAlign2D;

//HDR_6_________________________________ Header body
//
class EngineBasedOnMatching
{
public:
    EngineBasedOnMatching(VisionProcessingAlign2D* processor, const Constants& constants);
    ~EngineBasedOnMatching();

    bool GetConstants();
    bool MakeDefaultTemplateROI();
    bool SetROIsInRaw();
    bool FindAllStitchInfo_InRaw(const bool detailSetupMode);

private:
    VisionProcessingAlign2D* m_processor;
    const Constants& m_constants;
    StitchPara& m_parentPara;
    StitchResult& m_result;
    StitchPara_BasedOnMatching& m_para;
    CPackageSpec& m_packageSpec;

    long m_searchOffsetPixel;

    bool findStitchInfo_InRaw(const Ipvm::Rect32r* ppreRefROI_BCU, const Ipvm::Rect32s* pcurSearchROI,
        const Ipvm::Rect32s& preROIOverlap, const Ipvm::Rect32s& curROIOverlap, StitchMatchingInfo& o_MatchingInfo);

    bool ImageMatch(const Ipvm::Image8u& imageOrigin, const Ipvm::Image8u& imageStitch, Ipvm::Rect32s sourceROI,
        Ipvm::Rect32s targetSearchROI, Ipvm::Point32r2& sourcePoint, Ipvm::Point32r2& targetPoint, float& fMatchRate);
    bool ImageMatchSampling(const Ipvm::Image8u& imageOrigin, const Ipvm::Image8u& imageStitch, Ipvm::Rect32s sourceROI,
        Ipvm::Rect32s targetSearchROI, Ipvm::Point32r2& targetPoint, float& fMatchRate);
};
