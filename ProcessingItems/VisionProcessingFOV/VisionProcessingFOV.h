#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CDlgVisionProcessingFOV;
class VisionImageLot;

//HDR_6_________________________________ Header body
//
class __VISION_PROCESSING_ALIGN_CLASS VisionProcessingFOV : public VisionProcessing
{
public:
    VisionProcessingFOV(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionProcessingFOV(void);

public: // VisionProcessing virtual functions
    virtual BOOL DoInspection(const bool detailSetupMode,
        const enSideVisionModule i_ModuleStatus = enSideVisionModule::SIDE_VISIONMODULE_FRONT) override;
    virtual BOOL OnInspection() override;
    virtual BOOL LinkDataBase(BOOL bSave, CiDataBase& db) override;
    virtual void ResetSpecAndPara() override;
    virtual void ResetResult() override;
    virtual void GetOverlayResult(VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode) override;
    virtual void AppendTextResult(CString& textResult) override;
    virtual long ShowDlg(const ProcessingDlgInfo& procDlgInfo) override;
    virtual void CloseDlg() override;
    virtual std::vector<CString> ExportRecipeToText() override; //kircheis_TxtRecipe

public:
    CDlgVisionProcessingFOV* m_pVisionInspDlg;
};
