#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "VisionProcessingSpecPara.h"

//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CPI_Blob;
class CPackageSpec;
class CDlgVisionProcessingSpec;
class VisionImageLot;

//HDR_6_________________________________ Header body
//
class __VISION_PROCESSING_SPEC_CLASS VisionProcessingSpec : public VisionProcessing
{
public:
    VisionProcessingSpec(
        LPCTSTR moduleGuid, LPCTSTR moduleName, VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionProcessingSpec(void);

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
    CVisionProcessingSpecPara m_VisionPara;
    CDlgVisionProcessingSpec* m_pVisionInspDlg;
    //-----------------------------
    // Align 관련
    //-----------------------------

public:
    void SetModuleName(CString strName);
};
