#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/Ngrv2DImageInfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/NgrvAfRefInfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/NgrvGrabInfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/NgrvPackageGrabInfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/NgrvSingleRunInfo.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//HDR_3_________________________________ External library headers
#include <afxmt.h>

//HDR_4_________________________________ Standard library headers
#include <map>

//HDR_5_________________________________ Forward declarations
class IllumInfo2D;
class CiDataBase;
class CPackageSpec;
class OldScanSpec;
class VisionProcessingSpec;
class VisionProcessingFOV;
class VisionProcessingCarrierTapeAlign;
class VisionProcessingTapeDetailAlign;
class VisionProcessingGlobalMaskMaker;
class VisionInspection2DMatrix;
class VisionInspectionGeometry;
class VisionInspectionInPocketTR;
class VisionInspectionOTITR;
class VisionInspectionSurface;
class VisionInspection;
class VisionInspectionResult;
class VisionInspectionSpec;
class VisionImageLot;
class VisionProcessing;
class VisionReusableMemory;
class VisionTrayScanSpec;
class VisionTapeSpec;
class VisionUnitDlg;
class ImageProcPara;
enum class enPackageType : long;
struct sInfoSharedMemory;

//HDR_6_________________________________ Header body
//
struct sUnitinspectionLogInfo
{
    long m_nThreadindex;
    std::map<CString, std::vector<std::map<CString, CString>>> m_mapProcessingFunctionExcuteTimeLog;
    std::vector<std::map<CString, CString>> m_vecProcessExcuteTimeLog;

    void Init()
    {
        m_nThreadindex = -1;
        m_mapProcessingFunctionExcuteTimeLog.clear();
        m_vecProcessExcuteTimeLog.clear();
    }

    void SetProcessingFunctionExcuteTimeLog(const CString i_strModuleName,
        std::vector<std::map<CString, CString>> i_vecMapProcessingLogData) //mc_Inspection Module의 Log
    {
        m_mapProcessingFunctionExcuteTimeLog[i_strModuleName] = i_vecMapProcessingLogData;
    }

    void SetProcessExcuteTimeLog(
        const CString i_strFunctionName, const float i_fFunctionExcuteTime_ms) //mc_Vision Unit의 Log
    {
        if (i_strFunctionName.IsEmpty() == true
            || i_fFunctionExcuteTime_ms < 0) //함수 이름이 없거나 또는 진행된 시간이 0보다 아래일 수는 없다
            return;

        std::map<CString, CString> mapstrProcessExcuteTimeLog;
        CString strFunctionExcuteTime("");
        strFunctionExcuteTime.Format(_T("%.3f"), i_fFunctionExcuteTime_ms);
        mapstrProcessExcuteTimeLog[i_strFunctionName] = strFunctionExcuteTime;

        m_vecProcessExcuteTimeLog.push_back(mapstrProcessExcuteTimeLog);
    }
};

class __VISION_UNIT_CLASS__ VisionUnit : public VisionUnitAgent
{
public:
    VisionUnit(const IllumInfo2D& illum2D, VisionMainAgent& main);
    ~VisionUnit(void);

public: // VisionUnitAgent virtual functions
    virtual BOOL RunInspectionInTeachMode(VisionProcessing* currentModule, bool includeCurrentModule,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT) override;
    virtual BOOL RunInspection(VisionProcessing* currentModule = nullptr, const bool noNeedSurfaceInspection = false,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT) override;
    virtual LPCTSTR GetLastInspection_Text(BOOL isLogSave = FALSE) override;
    virtual VisionInspectionOverlayResult& GetInspectionOverlayResult() override;
    virtual std::vector<VisionDebugInfo*> GetVisionDebugInfos() override;
    virtual void* GetVisionDebugInfo(
        CString _strVisionModuleGUID, CString _strDebugInfoName, long& o_nDataNum) override;
    virtual VisionProcessing* GetPreviousVisionProcessing(VisionProcessing* targetProc) override;
    virtual VisionProcessing* GetVisionProcessingByGuid(LPCTSTR guid) override;
    virtual VisionProcessing* GetVisionAlignProcessing() override;
    virtual LPCTSTR GetVisionProcessingModuleNameByGuid(LPCTSTR guid) override;
    virtual LPCTSTR GetVisionAlignProcessingModuleGUID() override;
    virtual BOOL IsTheUsingVisionProcessingByGuid(LPCTSTR guid) override;
    virtual void GetSwitchMarkTeachViewIndex(long& o_CurMarkTeachViewIndex) override;
    virtual long GetCurrentPaneID() const override;
    virtual bool SetCurrentPaneID(long nID) override;

    virtual void setImageLot(std::shared_ptr<VisionImageLot> imageLot) override;
    virtual void setImageLotInsp() override;
    virtual const VisionScale& getScale() const override;
    virtual const VisionTapeSpec* getTapeSpec() const override;
    virtual const VisionTrayScanSpec& getTrayScanSpec() const override;
    virtual const InspectionAreaInfo& getInspectionAreaInfo() const override;
    virtual const UnitAreaInfo& getUnitAreaInfo() const override;
    virtual const VisionImageLot& getImageLot() const override;
    virtual const VisionImageLotInsp& getImageLotInsp() const override;
    virtual VisionTapeSpec* getTapeSpec() override;
    virtual VisionTrayScanSpec& getTrayScanSpec() override;
    virtual VisionImageLot& getImageLot() override;
    virtual VisionImageLotInsp& getImageLotInsp() override;
    virtual VisionReusableMemory& getReusableMemory() override;
    virtual const bool IsSideVision() const override;
    virtual const long GetSideVisionNumber() const override;
    virtual void GetPackageSize(const bool& i_bUnitIsPixel, float& o_fBodySizeX, float& o_fBodySizeY) override;

    virtual const IllumInfo2D& getIllumInfo2D() const override;

    // Side Vision 정보 가져오기 - JHB_2024.04.15
    virtual long GetSideVisionSection() override;
    virtual void SetSideVisionSection(long i_nSideVisionSection) override;

    // MED#6 - JHB_2025.01.06
    virtual void SetPackageInfo_NGRV(std::vector<NgrvPackageGrabInfo> packageInfo) override;
    virtual std::vector<NgrvPackageGrabInfo>& GetPackageInfo_NGRV() override;
    virtual void SetAFInfo_NGRV(NgrvAfRefInfo afInfo) override;
    virtual NgrvAfRefInfo& GetAFInfo() override;
    virtual void SetGrabInfo_NGRV(std::vector<NgrvGrabInfo> grabInfo) override;
    virtual std::vector<NgrvGrabInfo>& GetGrabInfo_NGRV() override;
    virtual void Set2DImageInfo_NGRV(long visionID, Ngrv2DImageInfo imageInfo) override;
    virtual Ngrv2DImageInfo& Get2DImageInfo_NGRV(long visionID) override;
    virtual void SetNGRVSingleRunInfo(long visionID, NgrvSingleRunInfo singleRunInfo) override;
    virtual NgrvSingleRunInfo& GetNGRVSingleRunInfo(long visionID) override;
    virtual void SetSocketInfo(Ipvm::SocketMessaging* pMessageSocket) override;
    virtual Ipvm::SocketMessaging& GetSocketInfo() override;
    virtual int32_t GetVisionMainType() override;

public:
    ///============== Buffer 관리 =========================////
    // 입력 Image (획득 Image 또는 File Image) 메모리
    void ShowProcessingTeachingUI(HWND hwndParent);
    void CloseProcessingTeachingUI();

    void OnImagingConditionChanged(const long nVisionType, const long maxImageSizeX, const long maxImageSizeY);

    // 별도로 분기한다
    // Input Image 리셋
    void InspectionClear();

    // Open JobFile
    BOOL LinkDataBase(long jobVersion, BOOL bSave, CiDataBase& JobDB);
    BOOL LinkDataBase_PackageSpec(BOOL bSave, CiDataBase& JobDB);

    // Image File 관련 함수
    BOOL OpenImageFile();
    BOOL OpenImageFile(CString strFilename, bool isOfficialExtension);
    BOOL OpenBmpImageFile(CString strFileName, bool waitPopup);
    BOOL SaveImageFile();

    // Package Type 관련 함수
    BOOL SetPackageType();

    void SetTrayScanSpec(const VisionTrayScanSpec& spec);

    void NewInspItem();
    void GetAvailableFixedItemList(std::vector<VisionProcessing*>& availableFixedItems);
    void UpdateInspItem(
        const std::vector<VisionProcessing*>& availableFixedItems, enPackageType nPackageType, BOOL bDeadBug);

    void SetSpecData(LPCTSTR moduleName, const std::vector<VisionInspectionSpec>& vecVisionItemSpec, long nMarkID = -1);
    void GetSpecData(LPCTSTR moduleName, std::vector<VisionInspectionSpec>& vecVisionItemSpec, long nMarkID = -1);

    BOOL SurfaceJobManager(BOOL bSave, long nSurfaceID, CString strFilePath);
    void RunInspectionCurrentProcessingModule();

    std::vector<VisionInspectionSpec*> GetVisionInspectionSpecs();
    std::vector<VisionInspectionResult*> GetVisionInspectionResults();

    BOOL IsMarkExist();
    BOOL IsNeedMarkTeach();
    bool IsWindowVisible();

    std::vector<CString> ExportRecipeToText(); //kircheis_TxtRecipe

    void SetIntensityCheckerBypassMode(BOOL bUseBypassMode); //SDY_IntensityChecker

    void Set2DMatrixBypassMode(BOOL bUseBypassMode); //kircheis_NGRV Bypass
    BOOL Is2DMatrixReadingBypass(); //kircheis_NGRV Bypass

    BOOL GetMajorRoiCenter(Ipvm::Point32s2& ptCenter);

    BOOL GetNGRVAFInfo(BOOL& o_bIsValidPlaneRefInfo, std::vector<Ipvm::Point32s2>& o_vecptRefPos_UM,
        long& o_nAFImgSizeX, long& o_nAFImgSizeY,
        std::vector<BYTE>&
            o_vecbyAFImage); //kircheis_NGRVAF 리턴값은 Align2D 활성화 여부다. FALSE면 유효하지 않다는 거임

    BOOL IsEnable2DMatrix(); //kircheis_20230126

public:
    const long m_threadIndex;

    CPackageSpec* m_pPackageSpec;
    VisionReusableMemory* m_VisionReusableMemory;

    BOOL m_bEmpty;
    BOOL m_bMarginal;
    BOOL m_bReject;
    BOOL m_bInvalid;
    BOOL m_bDoubleDevice; //kircheis_3DEmpty
    BOOL m_bCoupon; //SDY_Coupon2D

    // Processing modules
    VisionProcessingSpec* m_visionProcessingSpec_TapeReelLayoutEditor;
    VisionProcessingFOV* m_VisionProcessingFOV;
    VisionProcessingCarrierTapeAlign* m_visionProcessingCarrierTapeAlign;
    VisionProcessingTapeDetailAlign* m_visionProcessingTapeDetailAlign;
    VisionProcessingGlobalMaskMaker* m_visionProcessingGlobalMaskMaker;

    // Inspection modules
    VisionInspection2DMatrix* m_visionInspection2DMatrix;
    VisionInspectionInPocketTR* m_visionInspectionInPocket;
    VisionInspectionOTITR* m_visionInspectionOTITR;
    std::vector<VisionInspectionSurface*> m_visionInspectionSurfaces;
    std::vector<VisionInspectionGeometry*> m_visionInspectionGeometries;

    // Vision Processing Module 들을 관리
    std::vector<VisionProcessing*> m_vecVisionModules;
    std::vector<VisionProcessing*> m_vecAllPredefineVisionModulesBuffer;

    long GetVisionProcessingCount() const;
    VisionProcessing* GetVisionProcessing(long index);
    VisionProcessing* GetVisionProcessing(CString strModuleGUID);

    // ColorImage 수정
    CString m_reviewColorImageName;

    long GetInspTotalResult();
    BOOL GetParaData(CString strModuleName, ImageProcPara& i_ImageProcPara, long nMarkID = -1);
    void GenerateLastInspectionInfo(BOOL isLogSave);

    // 2nd Inspection (DeepLearning) 관련 Shared Memory 초기화
    HANDLE m_handleImage;
    HANDLE m_handleInform;
    HANDLE m_handleDevResult;

    sInfoSharedMemory* m_ptrSharedMem;
    BYTE* m_ptrSharedImageMem;
    BYTE* m_pbySharedDevResult;

    CMutex m_Mutex;
    CMutex m_updateMutex;

    void InitSharedMemBuffers();
    void ReleaseSharedMemBuffers();

    sUnitinspectionLogInfo GetUnitinspLogInfo();

    long m_nSideVisionSection; // Side Vision 정보 가져오는 변수 - JHB_2024.04.15

private:
    void* GetDebugInfoData(CString strDebugInfoName);
    long GetIndex_GenerateSpecSurfaceMask(
        VisionProcessing* targetModule); // 몇번째 Item 검사후 Spec Surface Mask를 만들것 인가
    void GenerateSpecSurfaceMask();

    const IllumInfo2D& m_illum2D;
    std::vector<long> m_lastInspection_listToCollect;
    bool m_lastInspection_InfoMake;
    CString m_lastInspeciton_Text;

    VisionUnitDlg* m_visionUnitDlg;
    VisionInspectionOverlayResult* m_runOverlayResult;

    std::shared_ptr<VisionImageLot> m_imageLot;
    VisionImageLotInsp* m_pImageLotInsp;
    VisionTrayScanSpec* m_trayScanSpec;
    VisionTapeSpec* m_tapeSpec;
    UnitAreaInfo* m_unitAreaInfo;

    BOOL RunInspection(VisionProcessing* currentModule, bool includeCurrentModule, bool noNeedSurfaceInspection,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void MakeTraySpecFromOldRecipe(const OldScanSpec& oldScanSpec);

    //mc_Inspection Log
    sUnitinspectionLogInfo m_sUnitinspectionLogInfo;

    // MED#6 - JHB_2025.01.06
    std::vector<NgrvPackageGrabInfo> m_vecPackageInfo_NGRV;
    NgrvAfRefInfo m_afInfo_NGRV;
    std::vector<NgrvGrabInfo> m_vecGrabInfo_NGRV;
    Ngrv2DImageInfo m_ngrv2DImageInfo_BTM;
    Ngrv2DImageInfo m_ngrv2DImageInfo_TOP;

    NgrvSingleRunInfo m_ngrvSingleRunInfo_BTM;
    NgrvSingleRunInfo m_ngrvSingleRunInfo_TOP;
    Ipvm::SocketMessaging* m_pMessageSocket;
};
