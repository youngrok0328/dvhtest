#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CVisionProcessingGlobalMaskMakerPara;
class CDlgSetupUI;
class ImageProcPara;
class CPackageSpec;
class CPI_EdgeDetect;
class CPI_Blob;
class VisionAlignResult;
class VisionImageLot;
class ImageLotView;
class SpecLayer;
class ResultData;
class ResultLayer;
class ResultLayerPre;
enum class SpecType;
struct ResultUserROI;

//HDR_6_________________________________ Header body
//
class __VISION_GLOBAL_MASK_MAKER_CLASS VisionProcessingGlobalMaskMaker : public VisionProcessing
{
public:
    VisionProcessingGlobalMaskMaker(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionProcessingGlobalMaskMaker(void);

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
    CVisionProcessingGlobalMaskMakerPara* m_VisionPara;
    CDlgSetupUI* m_pVisionInspDlg;

    bool CreateSetupImage(long imageIndex, Ipvm::Image8u& o_image);
    bool CreateLayersBeforeIndex(const bool detailSetupMode, long limitLayerIndex);
    bool CreateSingleLayer(long currentLayerIndex, ResultLayer& layer);

    bool isLayerValid(long layerIndex);

    CPI_EdgeDetect* m_pEdgeDetect;
    CPI_Blob* m_pBlob;
    CString m_inspErrorMessage;

    VisionAlignResult* m_sEdgeAlignResult;
    void GetEdgeAlignResult(); //kircheis_USI_OA

    // Align Result...
    float m_fCalcTime;

    ResultData* m_result;

private:
    Ipvm::Rect32s GetOperationRegion();
    bool GenerateLayers(const bool detailSetupMode, long limitLayerIndex, Ipvm::Image8u* o_totalImage = nullptr);
    bool CreatePreparedMask(
        const ResultUserROI& userRoi, long currentLayerIndex, ResultLayer& layer, Ipvm::Image8u& io_maskImage);
};
