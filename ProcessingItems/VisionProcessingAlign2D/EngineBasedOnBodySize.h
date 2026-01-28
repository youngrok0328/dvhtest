#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class Constants;
class CPackageSpec;
class EdgeSearchRoiPara;
class StitchPara;
class StitchMatchingInfo;
class StitchResult;
class StitchResultROI;
class StitchPara_BasedOnBodySize;
class VisionProcessingAlign2D;

//HDR_6_________________________________ Header body
//
class EngineBasedOnBodySize
{
public:
    EngineBasedOnBodySize(VisionProcessingAlign2D* processor, const Constants& constants);
    ~EngineBasedOnBodySize();

    bool GetConstants();
    bool SetROIsInRaw();
    bool FindAllStitchInfo_InRaw(const bool detailSetupMode);

private:
    VisionProcessingAlign2D* m_processor;
    const Constants& m_constants;
    StitchPara& m_parentPara;
    StitchResult& m_result;
    StitchPara_BasedOnBodySize& m_para;
    CPackageSpec& m_packageSpec;

    bool EdgeDetectInROIs();
    bool FittingLineUsingEdgePoints();
    bool MakeAllStitchInformation();
    void MakeStitchInformation(long x, long y, EdgeSearchRoiPara& object, StitchMatchingInfo& o_MatchingInfo);
};
