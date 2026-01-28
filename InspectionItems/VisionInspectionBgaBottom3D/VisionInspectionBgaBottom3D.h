#pragma once
//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "ParaDefine.h"

//HDR_2_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/Ball.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionWarpageShapeResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspection.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CPackageSpec;
class VisionInspectionBgaBottom3DPara;
class CDlgVisionInspectionBgaBottom3D;
class VisionAlignResult;
class VisionImageLot;
class VisionInspectionBgaBottom3DResult;

//HDR_6_________________________________ Header body
//
class __VISION_INSP_BGA_3D_INSPECTION_CLASS__ VisionInspectionBgaBottom3D : public VisionInspection
{
public:
    VisionInspectionBgaBottom3D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspectionBgaBottom3D(void);

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
    VisionInspectionBgaBottom3DPara* m_pVisionPara;
    CDlgVisionInspectionBgaBottom3D* m_pVisionInspDlg;

    VisionAlignResult* m_sEdgeAlignResult;

    float m_fCalcTime;

    VisionWarpageShapeResult m_visionWarpageShapeResult;

    std::vector<CString> m_vecstrInvalidBallInfo_BH;
    std::vector<CString> m_vecstrInvalidBallInfo_Copl;
    void SetInvalidBallInfo(CString strCurBallInfo, std::vector<CString>& vecstrInvalidBallInfo); //kircheis_AIBC

public:
    void ResetInspItem(long nNum);
    void DisplayResultOverlay();

    long GetMaxValidPointCount(long ballIndex, enumGetherBallZType zType);

    BOOL Initialize();

    BOOL MakeSpecObjectROI_ByPKGSize(std::vector<Ipvm::Rect32s>& vecrtObjectSpecROI,
        std::vector<Ipvm::Rect32s>& vecrtObjectSearchROI, long ballTypeIndex = -1);

    void InvalidSequence();

    void PrintOutResult(VisionInspectionResult* pResult, LPCTSTR szItemName, BOOL& bBallMeasured,
        BOOL bDeviceLevelResult, CString& strText, CString& strLogText);
    BOOL IsValid(int nResult);

    BOOL CheckROI(Ipvm::Rect32s& rtROI, long i_width, long i_height);

    BOOL InspRun(const bool detailSetupMode);
    BOOL FindBallBlob(
        std::vector<Ipvm::Rect32s>& vecrtObjectSearchROI, std::vector<Ipvm::Point32r2>& ballBlobPositions);
    BOOL AlignBall(const std::vector<Ipvm::Point32r2>& allBlobPositions, std::vector<Ipvm::Point32r2>& allBallPositions,
        std::vector<Ipvm::Point32r2>& validBallPositions, std::vector<Ipvm::Rect32s>& validBallROIs);
    BOOL GatherBallZ(const std::vector<Ipvm::Point32r2>& alignedBallPositions,
        const std::vector<Ipvm::Point32r3>& srPosData, enumGetherBallZType type,
        std::vector<Ipvm::Point32r3>& vec3DObjectPos, Ipvm::Image8u& o_ballLogicalPixelImage);
    BOOL GatherSrZ(const std::vector<Ipvm::Point32r2>& allBallPositions, std::vector<Ipvm::Rect32s>& validSubstrateROIs,
        std::vector<Ipvm::Point32r3>& validSubstrate3DData, Ipvm::Image8u& o_substrateMaskImage);
    float CalcSrZ(const Ipvm::Rect32s& rtROI, const Ipvm::Image8u& maskImage, float* tempBuffer);
    float CalcSrZ_SortAvr(const Ipvm::Rect32s& rtROI, const Ipvm::Image8u& maskImage, float* tempBuffer); //kircheis_Tan
    float CalcBallZ(const Ipvm::Image32r& zmap, const Ipvm::Image16u& vmap, const Ipvm::Rect32s& rtROI,
        const float pixelPercent, long maxValidCount, const float srZPos, const float heightThreshold,
        const Ipvm::Image8u& phygicalPixelMask, Ipvm::Image8u& o_logicalPixel);

    BOOL InspBallCoplanarity(const std::vector<Ipvm::Point32r3>& vecObjectPos,
        const std::vector<Ipvm::Rect32s>& vecrtObject); //DoInsp에서 호출하는 함수.
    BOOL InspBallCoplanarity(const BOOL ReadDataType, const std::vector<Package::Ball> vecBallMapData,
        const std::vector<Ipvm::Point32r3>& vecObjectPos, const std::vector<Ipvm::Rect32s>& vecrtObject,
        const LPCTSTR strCoplInspName, const LPCTSTR strUnitCoplInspName,
        const CString strCompName); //kircheis_MED2.5 //상황에 따라 전체 그룹 or 단일 그룹 or 지정 그룹 검사
    BOOL InspBallHeight(const std::vector<Ipvm::Rect32s>& vecrtObjectROI,
        const std::vector<Ipvm::Point32r3>& vec3DObjectData,
        const std::vector<Ipvm::Point32r3>& vec3DSRData); //DoInsp에서 호출하는 함수.
    BOOL InspBallHeight(const std::vector<Package::Ball> vecBallMapData,
        const std::vector<Ipvm::Rect32s>& vecrtObjectROI, const std::vector<Ipvm::Point32r3>& vec3DObjectData,
        const std::vector<Ipvm::Point32r3>& vec3DSRData, const CString strInspcetionName,
        const CString strCompName); //kircheis_MED2.5 //상황에 따라 전체 그룹 or 단일 그룹 or 지정 그룹 검사
    BOOL InspWarpage(const std::vector<Ipvm::Point32r3>& vecSrPos, const std::vector<Ipvm::Rect32s>& vecrtSr,
        const bool detailSetupMode); //DoInsp에서 호출하는 함수.
    BOOL InspWarpage(const std::vector<Package::Ball> vecBallMapData, const std::vector<Ipvm::Point32r3>& vecSrPos,
        const std::vector<Ipvm::Rect32s>& vecrtSr, const LPCTSTR strWarpageInspName,
        const LPCTSTR strUnitWarpageInspName,
        const bool detailSetupMode); //전체 그룹 검사 함수
    BOOL InspWarpage(const std::vector<Package::Ball> vecBallMapData, const std::vector<Ipvm::Point32r3>& vecSrPos,
        const std::vector<Ipvm::Rect32s>& vecrtSr, const LPCTSTR strWarpageInspName,
        const LPCTSTR strUnitWarpageInspName,
        const CString strCompName); //kircheis_MED2.5//그룹별 검사 함수

    void GetSrIgnoreQRT(Ipvm::Quadrangle32r& o_qrtValidSR);

public:
    VisionInspectionBgaBottom3DResult* m_result;

public:
    void DeleteSpec(long specIndex);
    void UpdateSpec();
    void UpdateSpec(const CString i_strInspectionName);
    BOOL MakeInspectionName(long targetindex, CString& o_InspectionName);
    void GetVisionInspectionSpecs();
    void SetVisionInspectionSpecs();

private:
    bool CollectBodyAlignResult();
    void MakeBallPhysicalPixel(const Ipvm::Point32r2& center, float radiusX, float radiusY, Ipvm::Image8u& mask);
    void MakeLogicalPixelMask_Ball(const Ipvm::Rect32s& roi, const Ipvm::Image16u& vismapImage,
        const Ipvm::Image8u& physicalPixelMask, Ipvm::Image8u& logicalPixelMask);
};
