#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspection.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionInspectionPackageSizePara;
class CDlgVisionInspectionPackageSize;
class CPackageSpec;
class CPI_EdgeDetect;
class CPI_Blob;
class VisionAlignResult;
class VisionImageLot;
class VisionInspectionSpec;
class VisionInspectionResult;

//HDR_6_________________________________ Header body
//
class __VISION_INSP_PACKAGE_SIZE_CLASS__ VisionInspectionPackageSize : public VisionInspection
{
public:
    VisionInspectionPackageSize(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspectionPackageSize(void);

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
    BOOL DoInsp(const bool detailSetupMode);

    VisionInspectionPackageSizePara* m_VisionPara;
    CDlgVisionInspectionPackageSize* m_pVisionInspDlg;

    CPI_EdgeDetect* m_pEdgeDetect;

    float m_fCalcTime;

private:
    BOOL InspBodySizeX(const Ipvm::Rect32s& i_rtObject);
    BOOL InspBodySizeY(const Ipvm::Rect32s& i_rtObject);
    BOOL InspParallelism(const Ipvm::Rect32s& i_rtObject);
    BOOL InspOrthogonality(const Ipvm::Rect32s& i_rtObject);

    void SetDebugInfo(const bool detailSetupMode);
    VisionAlignResult* m_sEdgeAlignResult;
};
