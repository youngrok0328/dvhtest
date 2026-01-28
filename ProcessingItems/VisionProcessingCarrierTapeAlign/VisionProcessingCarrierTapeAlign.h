#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "Constants.h"
#include "Para.h"

//HDR_2_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/CustomPolygon.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Point32s2.h> //kircheis_NGRVAF

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class Inspection;
class CustomFixed;
class CustomPolygonMap;
class CPackageSpec;
class DlgVisionProcessingCarrierTapeAlign;
class ImageProcPara;
class VisionImageLot;
struct FPI_RECT;

//HDR_6_________________________________ Header body
//
class __VISION_PROCESSING_CARRIER_TAPE_ALIGN_TR_CLASS VisionProcessingCarrierTapeAlign : public VisionProcessing
{
public:
    VisionProcessingCarrierTapeAlign(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionProcessingCarrierTapeAlign(void);

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
    Inspection* m_inspector;

    Para m_visionPara;
    DlgVisionProcessingCarrierTapeAlign* m_pVisionInspDlg;

    // Align Result...

    float m_fCalcTime;

    BOOL m_bImageSaveMode;

    BOOL AlignAllImages(const Ipvm::Rect32s& alignImageRoi, const bool detailSetupMode);

    bool GetConstants();

    //////////////////////////////////////////////////

    //std::vector<std::pair<Ipvm::Rect32s, float>> m_vecfMatchRate;

    Constants m_constants;
};
