#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/FPI_RECT.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspection.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Rect32r.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionInspectionComponent2DPara;
class CDlgVisionInspectionComponent2D;
class CPI_EdgeDetect;
class CPI_Blob;
class ChipInfoCollection;
class CPackageSpec;
class PassiveAlignResult;
class ChipInfo;
class CPassiveAlign;
class VisionImageLot;
class Result;
struct DMSPassiveChip;
struct ResultPassiveItem;

//HDR_6_________________________________ Header body
//
class __VISION_INSP_CHIP_CLASS__ VisionInspectionComponent2D : public VisionInspection
{
public:
    VisionInspectionComponent2D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspectionComponent2D(void);

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
    void CreateSpec();
    //kk GroupSpec
    void UpdateSpec();
    void GetVisionInspectionSpecs();
    void SetVisionInspectionSpecs();

    void ClearInspResult();

    void VerifyFrame();
    BOOL m_bFrameInitalize; //TRUE : 기존에 있던 Frame외 변경된 사항이 있다. FALSE; 없다. Defalut : FALSE

    VisionInspectionComponent2DPara* m_VisionPara;
    CDlgVisionInspectionComponent2D* m_pVisionInspDlg;
    CPassiveAlign* m_pPassiveAlign;
    std::vector<PassiveAlignResult> m_PassiveAlignResult;

    BOOL SetPackageInfo_SpecLink();

    BOOL DoAlign(const bool detailSetupMode);
    //BOOL DoAlign_Passive(sPassive_InfoDB *i_pPassiveInfoDB, ResultPassiveItem& o_result);
    BOOL DoAlign_Passive(
        sPassive_InfoDB* i_pPassiveInfoDB, ResultPassiveItem& o_result, PassiveAlignResult& o_PassiveAlignResult);
    void SetDebugInfo(const bool detailSetupMode);
    BOOL SetMaskImage(const bool detailSetupMode);
    void SetPassiveTypeMask(); //mc_DMS Type Mask
    std::vector<CString> GetPassiveTypeName(std::vector<sPassive_InfoDB> i_sPassiveInfoDB);
    Ipvm::Image8u GetPassiveTypeMaskImage(CString i_strPassiveTypeName,
        std::vector<ResultPassiveItem> i_vecResultPassiveItem, std::vector<sPassive_InfoDB> i_sPassiveInfoDB);

    BOOL MakePreInspInfo(const bool detailSetupMode);
    BOOL GetBodyAlignInfo(float& o_fAngle_rad, Ipvm::Rect32r& o_frtBody, Ipvm::Point32r2& o_fptCenter);
    void MakePackageInfo(const bool detailSetupMode, float i_fBodyAngle_rad, Ipvm::Point32r2 i_fptBodyCenter);
    FPI_RECT GetSpecROI_px(FPI_RECT i_sfrtSpecROI_mm, float i_fBodyAngle_rad, Ipvm::Point32r2 i_fptBodyCenter);
    Ipvm::Point32r2 ConvertSpecPosMMtoUM(float i_fSpecPosX_mm, float i_fSpecPosY_mm);

    //mc_PAD Mask
    BOOL CreateMaskInPadArea(sPassive_InfoDB* i_pPassiveInfoDB, float i_fAngleType, PI_RECT i_rtSpecROI,
        long i_nDircetion, Ipvm::Image8u& o_MaskImage); //mc_Pad까지 고려된 Component Mask 생성(Spec Value 이용)

    // 검사
    BOOL DoInsp(const bool detailSetupMode);
    BOOL Insp_Passive_Missing();
    BOOL Insp_Passive_Rotate();
    BOOL Judg_Passive_Rotate(VisionInspectionSpec* spec, VisionInspectionResult* result, sPassive_InfoDB* chipInfo,
        PassiveAlignResult* chipAlign, long ObjectNum);
    BOOL Insp_Passive_Shift_Width();
    BOOL Judg_Passive_Shift_Width(VisionInspectionSpec* spec, VisionInspectionResult* result, sPassive_InfoDB* chipInfo,
        PassiveAlignResult* chipAlign, long ObjectNum, float* fMaxOffsetValue);
    BOOL Insp_Passive_Shift_Length();
    BOOL Judg_Passive_Shift_Length(VisionInspectionSpec* spec, VisionInspectionResult* result,
        sPassive_InfoDB* chipInfo, PassiveAlignResult* chipAlign, long ObjectNum, float* fMaxOffsetValue);
    BOOL Insp_Passive_Tolerance_Width();
    BOOL Judg_Passive_Tolerance_Width(VisionInspectionSpec* spec, VisionInspectionResult* result,
        sPassive_InfoDB* chipInfo, PassiveAlignResult* chipAlign, long ObjectNum, long* totalResult);
    BOOL Insp_Passive_Tolerance_Length();
    BOOL Judg_Passive_Tolerance_Length(VisionInspectionSpec* spec, VisionInspectionResult* result,
        sPassive_InfoDB* chipInfo, PassiveAlignResult* chipAlign, long ObjectNum, long* totalResult);

public:
    CPI_EdgeDetect* m_pEdgeDetect;
    std::vector<sPassive_InfoDB> m_vecsPassiveInfoDB; //passive는 모두 이녀석을 사용하도록 한다
    FPI_RECT m_sfrtPackageBody;
    std::vector<Ipvm::Rect32s> m_vecrtReject;
    Result* m_result;

    float m_fCalcTime;
    long m_nTotalResult;

    //kk type별 검사 진행
    std::vector<CString> m_vecsPassiveTypes;
    std::map<CString, long> m_mapPassiveTypesCount;
};
