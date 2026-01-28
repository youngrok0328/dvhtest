#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/FPI_RECT.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../InformationModule/dPI_DataBase/ChipInfo.h"
#include "../../InformationModule/dPI_DataBase/ChipInfoCollection.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspection.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <map>

//HDR_5_________________________________ Forward declarations
class CComponentAlign;
class CDlgVisionInspectionComponent3D;
class ChipInfo;
class CPI_EdgeDetect;
class CPI_Blob;
class CPI_Geometry;
class ChipInfoCollection;
class CPackageSpec;
class VisionInspectionComponent3DPara;
class VisionImageLot;
class SComponentAlignResult;

//HDR_6_________________________________ Header body
//
enum enumPassiveType
{
    PassiveType_Capacitor = 0,
    PassiveType_Resistor,
    PassiveType_Tantal,
    PassiveType_LSC, //LandSide Capacitor
};

class __VISION_COMPONENT3D_CLASS__ VisionInspectionComponent3D : public VisionInspection
{
public:
    VisionInspectionComponent3D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspectionComponent3D(void);

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
    VisionInspectionComponent3DPara* m_VisionPara;
    CDlgVisionInspectionComponent3D* m_pVisionInspDlg;

    BOOL DoAlign(const bool detailSetupMode);
    BOOL DoInsp(const bool detailSetupMode);
    BOOL MakePreInspInfo(const bool detailSetupMode);
    Ipvm::Point32r2 ConvertSpecPosMMtoUM(float i_fSpecPosX_mm, float i_fSpecPosY_mm);

    // 검사
    BOOL Insp_CompMissing(std::vector<bool>& o_missingCheck);
    BOOL Insp_CompHeight(const std::vector<bool>& i_missingCheck);
    BOOL Judg_Comp_Height(VisionInspectionSpec* spec, VisionInspectionResult* result, sPassive_InfoDB* chipInfo,
        SComponentAlignResult* chipAlign, long ObjectNum, long* totalResult);
    BOOL Insp_CompQuality(const std::vector<bool>& i_missingCheck);
    BOOL Judg_Comp_Quality(VisionInspectionSpec* spec, VisionInspectionResult* result, sPassive_InfoDB* chipInfo,
        SComponentAlignResult* chipAlign, long ObjectNum, long* totalResult);
    BOOL Insp_ElectrodeHeight(const std::vector<bool>& i_missingCheck);
    BOOL Judg_Electrode_Height(VisionInspectionSpec* spec, VisionInspectionResult* result, sPassive_InfoDB* chipInfo,
        SComponentAlignResult* chipAlign, long ObjectNum, long* totalResult, float* maxHeight);
    BOOL Insp_ElectrodeTilt(const std::vector<bool>& i_missingCheck);
    BOOL Judg_Electrode_Tilt(VisionInspectionSpec* spec, VisionInspectionResult* result, sPassive_InfoDB* chipInfo,
        SComponentAlignResult* chipAlign, long ObjectNum, long* totalResult);
    BOOL Insp_BodyHeight(const std::vector<bool>& i_missingCheck);
    BOOL Judg_Body_Height(VisionInspectionSpec* spec, VisionInspectionResult* result, sPassive_InfoDB* chipInfo,
        SComponentAlignResult* chipAlign, long ObjectNum, long* totalResult);
    BOOL Insp_BodyTilt(const std::vector<bool>& i_missingCheck);
    BOOL Judg_Body_Tilt(VisionInspectionSpec* spec, VisionInspectionResult* result, sPassive_InfoDB* chipInfo,
        SComponentAlignResult* chipAlign, long ObjectNum, long* totalResult);
    long SkipMIATypeResult(long i_nPassiveidx, enumPassiveTypeDefine i_ePassiveType, VisionInspectionSpec* i_pSpec,
        float i_fValue, VisionInspectionResult& o_InsepctionResult);

    void MakePackageInfo(const bool detailSetupMode, float i_fBodyAngle_rad, Ipvm::Point32r2 i_fptBodyCenter);
    FPI_RECT GetSpecROI_px(FPI_RECT i_sfrtSpecROI_mm, float i_fBodyAngle_rad, Ipvm::Point32r2 i_fptBodyCenter);

    BOOL GetBodyAlignInfo(float& o_fAngle_rad, Ipvm::Rect32r& o_frtBody, Ipvm::Point32r2& o_fptCenter);

    void GetBallInfo(std::vector<Ipvm::Point32r3>& vec3DObjectPos);

    BOOL MakeDebugROIs(sPassive_InfoDB* i_pPassiveInfoDB, std::vector<PI_RECT>* pvecsrtChip,
        std::vector<PI_RECT>* pvecsrtElectrode, std::vector<PI_RECT>* pvecsrtBody);
    BOOL MakeDebugCalcROIs(sPassive_InfoDB* i_pPassiveInfoDB, std::vector<PI_RECT>* pvecsrtComponent,
        std::vector<PI_RECT>* pvecsrtValidElectrode, std::vector<PI_RECT>* pvecsrtValidBody);

    BOOL SetPackageInfo_SpecLink();

    float GetZValue(float fHmapData, float fConvertScale);

public:
    CPI_EdgeDetect* m_pEdgeDetect;
    CPI_Blob* m_pBlob;

    Chip::ChipInfoCollection* m_PackageInfo;
    CComponentAlign* m_pCompAlign;
    std::vector<sPassive_InfoDB> m_vecsPassiveInfoDB; //passive는 모두 이녀석을 사용하도록 한다

    std::vector<SComponentAlignResult>
        m_vecsCompAlignResult; // SDY 컴포넌트 검사의 결과를 new를 사용하는 부분을 vector로 선언

    float m_fCalcTime;
    float m_fConvertScale;
    float m_fConvertScaleInten;

    FPI_RECT m_sfrtPackageBody;
    std::vector<Ipvm::Rect32s> m_vecrtReject;

    Ipvm::Point32r2 m_fptPixelperMM;

    BOOL m_bAbsolute;

    //kk GroupSpec
    void UpdateSpec();
    void GetVisionInspectionSpecs();
    void SetVisionInspectionSpecs();

    //kk type별 검사 진행
    std::vector<CString> m_vecsPassiveTypes;
    std::map<CString, long> m_mapPassiveTypesCount;
};
