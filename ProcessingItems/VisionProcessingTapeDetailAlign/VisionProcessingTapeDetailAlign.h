#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/Pad.h"
#include "../../InformationModule/dPI_DataBase/PadCollection.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class SurfaceLayerMask;
class DlgVisionProcessingTapeDetailAlign;
class Inspection;
class CPackageSpec;
class VisionImageLot;
class PadCollection;
class Pad;
class Para;
class AlignPara;
struct FPI_RECT;

//HDR_6_________________________________ Header body
//
class __VISION_PACKAGE_ALIGN_TR_CLASS VisionProcessingTapeDetailAlign : public VisionProcessing
{
public:
    VisionProcessingTapeDetailAlign(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionProcessingTapeDetailAlign(void);

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
    Para* m_para;
    DlgVisionProcessingTapeDetailAlign* m_pVisionInspDlg;
    float m_fCalcTime;

private:
    Inspection* m_inspector;
    Ipvm::EdgeDetection* m_edgeDetection;
    CString m_errorLogText;
};
