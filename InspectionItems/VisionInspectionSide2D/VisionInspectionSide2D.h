#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspection.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionInspectionSide2DPara;
class CDlgVisionInspectionSide2D;
class CPackageSpec;
class CPI_EdgeDetect;
class CPI_Blob;
class VisionAlignResult;
class VisionImageLot;
class VisionInspectionSpec;
class VisionInspectionResult;

//HDR_6_________________________________ Header body
//
enum enumSideAlignObject
{
    ALIGN_OBJECT_SIDE_START = 0,
    ALIGN_OBJECT_SIDE_TOTAL_INFO = ALIGN_OBJECT_SIDE_START,
    ALIGN_OBJECT_SIDE_GLASS_INFO,
    ALIGN_OBJECT_SIDE_TOP_SUBSTRATE_INFO,
    ALIGN_OBJECT_SIDE_BOTTOM_SUBSTRATE_INFO,
    ALIGN_OBJECT_SIDE_END,
};

class __VISION_INSP_SIDE_INSPECTION_2D_CLASS__ VisionInspectionSide2D : public VisionInspection
{
public:
    VisionInspectionSide2D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspectionSide2D(void);

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

    VisionInspectionSide2DPara* m_VisionPara;
    CDlgVisionInspectionSide2D* m_pVisionInspDlg;

    CPI_EdgeDetect* m_pEdgeDetect;

    float m_fCalcTime;

private:
    BOOL InspTotalThickness(const bool detailSetupMode); //입력 인자는 Rej ROI에 사용할 Rect
    BOOL InspGlassThickness(const bool detailSetupMode); //입력 인자는 Rej ROI에 사용할 Rect
    BOOL InspTopSubstrateThickness(const bool detailSetupMode); //입력 인자는 Rej ROI에 사용할 Rect
    BOOL InspBottomSubstrateThickness(const bool detailSetupMode); //입력 인자는 Rej ROI에 사용할 Rect
    BOOL InspSideWarpage(const bool detailSetupMode); //입력 인자는 Rej ROI에 사용할 Rect

    BOOL GetDivideToTopBottomPoint(const bool detailSetupMode, const std::vector<Ipvm::Point32r2>& i_vecptAlignInfo,
        std::vector<Ipvm::Point32r2>& o_vecptTopPoint, std::vector<Ipvm::Point32r2>& o_vecptBottomInfo);

    BOOL GetSideAlignInfo(const long& i_nObject, Ipvm::Rect32s& o_rtROI, std::vector<Ipvm::Point32r2>& o_vecPoint);

    void SetDebugInfo(const bool detailSetupMode);
    VisionAlignResult* m_sEdgeAlignResult;

    std::vector<Ipvm::Point32r2> m_vecptDetailAlign;
    std::vector<Ipvm::Point32r2> m_vecptGlassCoreAlign;
    std::vector<Ipvm::Point32r2> m_vecptTopSubstrateAlign;
    std::vector<Ipvm::Point32r2> m_vecptBottomSubstrateAlign;

    void ResetNormalModelInspectionItem();
    BOOL VerifyNormalUnitInspection(const bool detailSetupMode, const CString strInspItemName);
};
