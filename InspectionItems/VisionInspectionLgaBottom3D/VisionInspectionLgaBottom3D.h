#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "VisionInspectionLgaBottom3DPara.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/FPI_RECT.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../InformationModule/dPI_DataBase/Land.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionWarpageShapeResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspection.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32r3.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CDlgVisionInspectionLgaBottom3D;
class CPI_EdgeDetect;
class CPI_Blob;
class CPackageSpec;
class Land;
class CDlgVision3DInspection;
class CVisionBGA3DInspection;
class CVisionLeadLess3DInspection;
class VisionImageLot;
class VisionAlignResult;

//HDR_6_________________________________ Header body
//
class C3DLandInspResult
{
public:
    std::vector<CString> vecstrID;
    std::vector<CString> vecstrGroupID; //kircheis_MED2.5

    Ipvm::Point32r2 m_packageCenter;
    float m_packageAngle_rad;

    FPI_RECT sfrtPackageBody;
    FPI_RECT sfrtRotatePackageBody;

    std::vector<std::vector<Ipvm::Point32r2>> vecfptSpecLand;
    std::vector<Ipvm::Point32r2> vecfptSpecLandUM;
    std::vector<std::vector<FPI_RECT>> vecsfrtSpecLand;
    std::vector<std::vector<Ipvm::Point32r2>> vecptObjectLandCenter;
    std::vector<std::vector<Ipvm::Point32r2>> vecptObjectValidLandCenter;
    std::vector<std::vector<BOOL>> vecptObjectValidLand;
    std::vector<std::vector<Ipvm::Rect32s>> vecrtBlobObj;
    std::vector<std::vector<FPI_RECT>> vecsfrtBlobLand;
    std::vector<std::vector<FPI_RECT>> vecsfrtSR;
    std::vector<Ipvm::Rect32s> vecrtSR;
    std::vector<Ipvm::Point32r3> vec3DObj;
    std::vector<Ipvm::Point32r3> vec3DSR;
    std::vector<Ipvm::Rect32s> vecrtLand;

    void Clear()
    {
        vecstrID.clear();
        vecstrGroupID.clear(); //kircheis_MED2.5

        m_packageCenter = Ipvm::Point32r2(0.f, 0.f);
        m_packageAngle_rad = 0.f;

        sfrtPackageBody = FPI_RECT(
            Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f));
        sfrtRotatePackageBody = FPI_RECT(
            Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f));

        vecfptSpecLand.clear();
        vecfptSpecLandUM.clear();
        vecptObjectLandCenter.clear();
        vecptObjectValidLandCenter.clear();
        vecptObjectValidLand.clear();
        vecsfrtSpecLand.clear();
        vecrtBlobObj.clear();
        vecsfrtBlobLand.clear();
        vecsfrtSR.clear();
        vecrtSR.clear();
        ;
        vec3DObj.clear();
        vec3DSR.clear();
        vecrtLand.clear();
        vecrtSR.clear();
    }

    C3DLandInspResult()
    {
        Clear();
    }
};

class __VISION_3DINSP_LAND_CLASS__ VisionInspectionLgaBottom3D : public VisionInspection
{
public:
    VisionInspectionLgaBottom3D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspectionLgaBottom3D(void);

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
    VisionInspectionLgaBottom3DPara m_VisionPara;
    CDlgVisionInspectionLgaBottom3D* m_pVisionInspDlg;

    std::vector<Package::Land> m_vecLandData; //Ignor상태가 있을수 있으므로 따로 저장하여 사용한다.
    std::vector<CString> m_group_id_list;
    std::vector<std::vector<Package::Land>> m_vec2LandData;

    VisionAlignResult* m_sEdgeAlignResult;

    //Align Function
    BOOL DoAlign(const bool detailSetupMode);
    BOOL MakePreInspInfo(const bool detailSetupMode);
    BOOL GetBodyAlignInfo(float& o_fAngle, Ipvm::Rect32r& o_frtBody, Ipvm::Point32r2& o_fptCenter);
    BOOL MakeSpecObject_ByPKGSize(FPI_RECT i_sfrtPackageBody,
        std::vector<std::vector<Ipvm::Point32r2>>& o_vec2ptObjectSpec, Ipvm::Point32r2& o_fptGravityCenter);
    BOOL RotateLandROI(const std::vector<std::vector<Ipvm::Point32r2>>& i_vecfptSpecLand,
        std::vector<std::vector<PI_RECT>>& o_vecpirtSpecLand, std::vector<std::vector<FPI_RECT>>& o_vecfpirtSpecLand,
        float i_fSizeOffsetWidthRatio = 0.f, float i_fSizeOffsetLengthRatio = 0.f);
    BOOL MakeLandSearchROI(std::vector<std::vector<FPI_RECT>> i_vecfpirtSpecLand,
        std::vector<std::vector<Ipvm::Rect32s>>& o_vecrtSearchLand);
    BOOL FindObjectBlob2(Ipvm::Image8u& binImage, Ipvm::Image8u& grayImage,
        std::vector<std::vector<FPI_RECT>> i_vecfrtSpecLandROI,
        std::vector<std::vector<Ipvm::Rect32s>>& i_vecrtObjSearch, std::vector<std::vector<Ipvm::Rect32s>>& o_vecrtObj,
        std::vector<std::vector<Ipvm::Point32r2>>& o_vecfptObjCenter, Ipvm::Point32r2& o_fptGravityCenter);
    BOOL MakeLandMaskROI(std::vector<std::vector<Ipvm::Point32r2>> i_vecfptSpecLand,
        std::vector<std::vector<PI_RECT>>& o_vec2pirtSpecLand, std::vector<std::vector<FPI_RECT>>& o_vec2fpirtSpecLand,
        long& o_nLayerNum, float i_fSizeOffsetWidthRatio = 0.f, float i_fSizeOffsetLengthRatio = 0.f);
    BOOL FillArbitraryRect(const PI_RECT& srtFillRect, BYTE byVal, Ipvm::Image8u& io_image); //Create Mask Image
    BOOL MakeGravityCenter(const Ipvm::Image32s i_imageLabel, const Ipvm::Rect32s i_rtObjectROI, const long i_nlable,
        Ipvm::Point32r2& o_GravityCenter);
    float GetAverageValueByImage(std::vector<Ipvm::Image8u> i_vecCalcImage);
    BOOL GetAverageImageImage(std::vector<Ipvm::Image8u> i_vecCalcImage, Ipvm::Image8u& o_AverageImage);
    void CalcZPosSr(const Ipvm::Image32r& zmap, const Ipvm::Image8u& mask, const Ipvm::Rect32s& rtROI, float& fZPos);
    void CalcZPosSrPlane(
        const Ipvm::Image32r& zmap, const Ipvm::Image8u& mask, const Ipvm::Rect32s& rtROI, float& fZPos);
    void CalcZPosLand(const Ipvm::Image32r& zmap, const Ipvm::Image8u& mask, Ipvm::Rect32s rtArea, float& o_fZPos);
    void GetSrIgnoreQRT(Ipvm::Quadrangle32r& o_qrtValidSR);
    bool CollectBodyAlignResult();
    BOOL AlignLand(const std::vector<std::vector<Ipvm::Point32r2>>& i_specLandPositions_px,
        const std::vector<std::vector<Ipvm::Point32r2>>& allBlobPositions,
        std::vector<std::vector<Ipvm::Point32r2>>& allLandPositions,
        std::vector<std::vector<Ipvm::Point32r2>>& validLandPositions,
        std::vector<std::vector<Ipvm::Rect32s>>& validLandROIs);
    BOOL MakeLandMask(const Ipvm::Image8u i_binImage, const Ipvm::Image8u i_grayImage, const bool detailSetupMode);
    void CalcZPos(Ipvm::Image8u i_maskImage, Ipvm::Image8u i_SRmaskImage);
    void ImageCopyinMask(const Ipvm::Image8u i_mask, const Ipvm::Rect32s i_rtDstROI, const Ipvm::Rect32s i_rtMaskROI,
        Ipvm::Image8u& o_dstImage);
    //Inspection Function
    BOOL DoInsp(const bool detailSetupMode);
    BOOL InspLandCoplanarity(const std::vector<Ipvm::Point32r3>& vecObjectPos,
        const std::vector<Ipvm::Rect32s>& vecrtObject); //DoInsp에서 호출하는 함수.
    BOOL InspLandCoplanarity(const BOOL ReadDataType, const std::vector<Ipvm::Point32r3>& vecObjectPos,
        const std::vector<Ipvm::Rect32s>& vecrtObject, const std::vector<CString>& vecObjectID,
        const std::vector<CString>& vecObjectGroupID, const std::vector<Ipvm::Point32r2>& vecObjectSpecUM,
        const LPCTSTR strCoplInspName, const LPCTSTR strUnitCoplInspName,
        const CString strCompName); //kircheis_MED2.5//상황에 따라 전체 그룹 or 단일 그룹 or 지정 그룹 검사
    BOOL InspWarpage(const std::vector<Ipvm::Point32r3>& vec3DSrPos, const std::vector<Ipvm::Rect32s>& vecrtSr,
        const bool detailSetupMode); //DoInsp에서 호출하는 함수.
    BOOL InspWarpage(const std::vector<Ipvm::Point32r3>& vec3DSrPos, const std::vector<Ipvm::Rect32s>& vecrtSr,
        const std::vector<CString>& vecObjectID, const std::vector<CString>& vecObjectGroupID,
        const std::vector<Ipvm::Point32r2>& vecObjectSpecUM, const LPCTSTR strWarpageInspName,
        const LPCTSTR strUnitWarpageInspName, const bool detailSetupMode, const CString strCompName); //kircheis_MED2.5
    BOOL InspWarpage(const std::vector<Ipvm::Point32r3>& vec3DSrPos, const std::vector<Ipvm::Rect32s>& vecrtSr,
        const std::vector<CString>& vecObjectID, const std::vector<CString>& vecObjectGroupID,
        const std::vector<Ipvm::Point32r2>& vecObjectSpecUM, const LPCTSTR strWarpageInspName,
        const LPCTSTR strUnitWarpageInspName, const CString strCompName); //kircheis_MED2.5
    BOOL InspLandHeight(const std::vector<Ipvm::Rect32s>& vecrtObjectROI,
        const std::vector<Ipvm::Point32r3>& vec3DObjectData,
        const std::vector<Ipvm::Point32r3>& vec3DSRData); //DoInsp에서 호출하는 함수.
    BOOL InspLandHeight(const std::vector<Ipvm::Rect32s>& vecrtObjectROI,
        const std::vector<Ipvm::Point32r3>& vec3DObjectData, const std::vector<Ipvm::Point32r3>& vec3DSRData,
        const std::vector<CString>& vecObjectID, const std::vector<CString>& vecObjectGroupID,
        const std::vector<Ipvm::Point32r2>& vecObjectSpecUM, const CString strHeightInspeName,
        const CString strCompName); //kircheis_MED2.5 //상황에 따라 전체 그룹 or 단일 그룹 or 지정 그룹 검사

public:
    CPI_EdgeDetect* m_pEdgeDetect;
    CPI_Blob* m_pBlob;
    Ipvm::BlobInfo* m_psBlobInfo;

    float m_fCalcTime;

    float m_fConvertScale;
    float m_fConvertScaleInten;

    std::vector<Ipvm::Rect32s> m_vecrtReject;

    Ipvm::Point32r2 m_fptPixelperMM;
    float m_fAngle;

    C3DLandInspResult m_result;

    VisionWarpageShapeResult m_visionWarpageShapeResult;

public:
    void DeleteSpec(long specIndex);
    void UpdateSpec();
    void UpdateSpec(const CString i_strInspectionName);
    BOOL MakeInspectionName(long targetindex, CString& o_InspectionName);
    void GetVisionInspectionSpecs();
    void SetVisionInspectionSpecs();

private:
    void remove_noise_point(const Ipvm::Image32r& zmap, const Ipvm::Rect32s& roi, Ipvm::Image8u& io_mask);
    void remove_lowVisibility_point(const Ipvm::Image32r& zmap, const Ipvm::Image16u& vmap, const Ipvm::Rect32s& roi,
        USHORT visibilityLowerBound, Ipvm::Image8u& io_mask);
    void getGroupInfo(std::vector<Package::Land> i_vecLandMap, std::vector<CString>& o_group_id_list);
};
