#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/Ngrv2DImageInfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/NgrvAfRefInfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/NgrvFrameInfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/NgrvGrabInfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/NgrvGrabPositionInfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/NgrvGrabbedImageInfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/NgrvPackageGrabInfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/NgrvSingleRunInfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionSpec.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionMainAgent.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Image8u3.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CVisionPrimaryUI;
class CVisionInlineUI;
class VisionMainMenuAccess;
class VisionInspection;
class VisionReusableMemory;
class VisionTrayScanSpec;
class VisionUnit;
class CDlgDetailSetupList;
class SlitBeam3DParameters;
class SystemConfig;
class ImageSaveOptionData;
class CallBackList_InspectionEnd;
class InlineGrab;
class VisionSystemParameters;
class VisionInspectionResult;
enum class VisionSurfaceCriteria_Column : long;
struct SItemSpec;
struct sGrab3DErrorLogMessageFormat;

//HDR_6_________________________________ Header body
//
enum class ImageSaveStatus
{
    ImageSaveStatus_Unknown = -9999,

    ImageSaveStatus_Success = 1,

    //ImageSave Direcotry
    ImageSaveStatus_CreateDirectory_Success = 11,
    ImageSaveStatus_CreateDirectory_Fail,
};

enum eReviewimageSaveLOGCategory
{
    eReviewimageSaveLOGCatrgory_InsufficientDiskCapacity_D, //D drive Disk 용량부족
    eReviewimageSaveLOGCatrgory_InsufficientDiskCapacity_C, //C drive Disk 용량부족 [Log image Save용]
    eReviewimageSaveLOGCatrgory_SaveOptionDisable, //Review Save option 비활성화
    eReviewimageSaveLOGCatrgory_SaveFrameEmpty, //SaveFrame 없음
    eReviewimageSaveLOGCatrgory_Combine_Insp_image_Empty, //Combine imgae[insp image가 없음]
    eReviewimageSaveLOGCatrgory_Combine_ImageCombine_Fail, //Combine image[image Combine 실패]
    eReviewimageSaveLOGCatrgory_Review_ImageSave_NOT_SUCCESS,
};

static LPCTSTR g_szReviewimageSaveLOGCategoryNames[] = {
    _T("InsufficientDiskCapacity_D"),
    _T("InsufficientDiskCapacity_C"),
    _T("Disable the Save Option"),
    _T("Save Frame is Empty"),
    _T("Combine image[Insp. image is Empty]"),
    _T("Combine image[Failed to combine images]"),
    _T("Review image Save Not Success"),
};

enum enumRecipeType
{
    RECIPE_TYPE_START = 0,
    RECIPE_TYPE_NORMAL = RECIPE_TYPE_START,
    RECIPE_TYPE_BYPASS,
    RECIPE_TYPE_NGRV,
    RECIPE_TYPE_END,
};

class __VISION_MAIN_CLASS__ CGrabItemFrameInfoNGRV //kircheis_NGRV // 얘를 벡터로 만들어 Recipe로 관리 하자.
{
public:
    CGrabItemFrameInfoNGRV();
    virtual ~CGrabItemFrameInfoNGRV(void);

    CString m_strGrabItemName;
    BOOL m_bIsSaveImage;
    std::vector<long> m_vecnFrameID;
    long m_nAFType;
    long m_nPlaneOffsetUM;
};

class __VISION_MAIN_CLASS__ CNGRVImageSet //kircheis_NGRV // 영상 저장을 위해 관리하는 class
{
public:
    CNGRVImageSet();
    virtual ~CNGRVImageSet(void);

    void Init();
    void SetInfo(long nTrayID, long nPackageID, CString strPackageID, long nGrabPointID, CString strInspItemName,
        CString strReviewSideName, std::vector<long> vecnFrameID, long nStitchCntX, long nStitchCntY, long nImageSizeX,
        long nImageSizeY);
    void SetImageBufferSize(std::vector<long> vecnFrameID, long nImageSizeX, long nImageSizeY);
    void SetImage(long nStitchID, std::vector<Ipvm::Image8u3> vecImageForSave, const BOOL& isNGRV_SingleRun);

    long m_nTrayID;
    long m_nPackageID;
    CString m_strPackageID;
    long m_nGrabPointID;
    CString m_strInspectionItemName; // 검사 항목 이름
    CString m_strReviewSideName; // 검사 비전 이름
    long m_nStitchCntX;
    long m_nStitchCntY;
    std::vector<long> m_vecnFrameID;
    std::vector<Ipvm::Image8u3> m_vecImageForSave;
};

class __VISION_MAIN_CLASS__ CVisionMain : public VisionMainAgent
{
public:
    CVisionMain(SystemConfig& systemConfig);
    virtual ~CVisionMain(void);

    LPCTSTR GetLastLinkDataBaseErrorMessage() const;
    BOOL GetLastLinkDataBaseJobVersionChanged() const;

public: // VisionMainAgent virtual functions
    virtual bool IsJobTeachEnabled() override;
    virtual bool IsAllBatchInspectionThreadsIdle() override;
    virtual long PushBatchImage(long batchKey, long paneIndex, LPCTSTR imageFilePath, LPCTSTR batchBarcode) override;
    virtual void RegisterCallBack_InspectionEnd(void* userData, FUNC_INSPECTION_END_CALLBACK* callback) override;
    virtual void UnRegisterCallBack_InspectionEnd(FUNC_INSPECTION_END_CALLBACK* callback) override;
    virtual void CopyImage_From_OtherVisionUnit_Into_PrimaryVisionUnit(long oterhVisionUnitIndex) override;
    virtual bool CheckFreeDiskForImageSave(bool sendErrorMessageToHost) override;

    virtual BOOL OpenImageFileAgent() override;
    virtual BOOL OpenImageFileAgent(CString strFilename) override;
    virtual BOOL OpenJobFileAgent() override;
    virtual BOOL OpenJobFile(CString strFilePath, CString strFileName) override;
    virtual void SetScanSpec(float scanLength_mm) override;
    virtual BOOL LinkDataBase(BOOL bSave, CiDataBase& JobDB) override;
    virtual BOOL LinkDataBase_PackageSpec(BOOL bSave, CiDataBase& JobDB) override;
    virtual void SetVisionJobName(CString strFilePath, CString strFileName) override;
    virtual void DeleteMarkMultiImages() override;
    virtual bool isInline() const override;
    virtual void SetInlineStart(bool isBatchInspection) override;
    virtual void SetInlineStop(bool isBatchInspection) override;
    virtual void SendResult(VisionUnit& visionUnit, VisionDeviceResult& DeviceResult, BOOL& bSendSuccess) override;
    virtual void SendResult(
        VisionUnit& visionUnit, SideVisionDeviceResult& SideVisionDeviceResult, BOOL& bSendSuccess) override;
    virtual void SendImage(const Ipvm::Image8u& image) override; //kircheis_UPH_up
    virtual void SendImage(
        const Ipvm::Image8u& imageFront, const Ipvm::Image8u& imageRear) override; //Side Vision Inline 전송용
    virtual void Send2DAllFrame() override; //mc_MED#3.5 AllFrameSave
    virtual bool isConnect() const override; //kircheis_UPH_up
    virtual BOOL SaveJobFile() override;
    virtual BOOL SaveJobFile(
        CString strFilename, SaveJobMode mode, BOOL bSendJob = FALSE, BOOL bSendPackageSpec = FALSE) override;
    virtual void OnImageGrab() override;
    virtual void SendToHostMarkTeachImage() override;
    virtual BOOL OnJobChanged() override;
    virtual BOOL HardwareSetup() override;
    virtual BOOL HardwareSetupForNGRV(std::vector<long> vecnFrameID, BOOL bUseIR, BOOL bInline) override;
    virtual BOOL OnImagingConditionChanged() override;

    virtual void PostExecutionProcessing(bool isManualInsp, VisionUnit& visionUnit, VisionDeviceResult& deviceResult,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT) override;

    virtual void SaveSystemInfo() override; //	전호빈  - 2020.04.19 : System Info 저장 관련 함수
    virtual bool GetSend2DAllFrame() override; //mc_MED#3.5 AllFrameSave
    virtual void ResetDevicePassCount() override; //mc_MED#3.5 AllFrameSave

    //mc_InlineGrabTime[GPU Test]
    virtual void ResetInlineGrabTime() override;
    virtual void SaveInlineGrabTime() override;

    virtual const InspectionAreaInfo& getInspectionAreaInfo() override;

    // automation 관련 함수
    virtual void SendToAutomationBatchEnd(long nInspTime) override;
    ;

    // Side Front/Rear 구분을 위한 함수 - JHB_2024.04.15
    virtual void SetSideVisionSection(long i_nSideSection) override;
    virtual long GetSideVisionSection() override;

    // MED#6 - JHB_2025.01.06
    void SetPackageInfo_NGRV(std::vector<NgrvPackageGrabInfo> packageInfo);
    std::vector<NgrvPackageGrabInfo>& GetPackageInfo_NGRV();
    void SetAFInfo_NGRV(NgrvAfRefInfo afInfo);
    NgrvAfRefInfo& GetAFInfo();
    void SetGrabInfo_NGRV(std::vector<NgrvGrabInfo> grabInfo);
    std::vector<NgrvGrabInfo>& GetGrabInfo_NGRV();
    void Set2DImageInfo_NGRV(long visionID, Ngrv2DImageInfo imageInfo);
    Ngrv2DImageInfo& Get2DImageInfo_NGRV(long visionID);
    void SetNGRVSingleRunInfo(long visionID, NgrvSingleRunInfo singleRunInfo);
    NgrvSingleRunInfo& GetNGRVSingleRunInfo(long visionID);

    // 20.05.18 Send DL Result
    void SendDLResult(VisionDeviceResult& DeviceResult, BOOL& bSendSuccess);

    CMutex m_mutSend;
    CMutex m_mutNGRV;

public: // UI Show & Hide
    void ShowVisionPrimaryUI(HWND hwndParent);
    void HideVisionPrimaryUI();
    bool IsShowVisionPrimaryUI() const;

    void ShowBatchInspectionUI(HWND hwndParent);

    void ShowIlluminationTeachUI(HWND hwndParent);

    void CloseAccessUI();

    void ShowProcessingTeachingUI(HWND hwndParent);
    void CloseProcessingTeachingUI(BOOL bSendToHostJobMode = FALSE);
    bool IsShowProcessingTeachingUI();

    void InitializeVisionInlineUI(HWND hwndParent);
    bool IsShowVisionInlineUI() const;

    void CloseAllUI();
    void ReturnToPrimaryUI(BOOL bCallLinkDatabase = TRUE);

    void SetColorImageToInlineUIforNGRV(Ipvm::Image8u3 image); //kircheis_NGRVINLINE

    std::vector<CString> ExportRecipeToText(); //kircheis_TxtRecipe
    std::vector<CString> ExportRecipe2DIllumToText(); //kircheis_TxtRecipe
    std::vector<CString> ExportRecipe3DIllumToText(); //kircheis_TxtRecipe

public: // 통신 관련
    void iPIS_Send_Job(CiDataBase& jobDB, long nMsg); //CJobFile* 를 전송을 하는 모든 일을 처리함.
    void iPIS_Send_PackageSpec();
    void iPis_Send_JobInfo();
    void iPIS_Send_ImageSize(long nSendImageSizeX, long nSendImageSizeY);
    void iPIS_Send_Image(const Ipvm::Image8u& image);
    void iPIS_Send_Image(const Ipvm::Image8u& imageFront,
        const Ipvm::Image8u& imageRear); //이 함수는 Side Vision의 Inline을 위한 함수다
    void IPIS_Send_NGRV_Image(const Ipvm::Image8u3& image); // Host로 Color 이미지를 송부한다 - JHB_NGRV
    void iPIS_Send_2DAllFrame();
    void iPIS_Send_InspResult(VisionUnit& visionUnit, VisionDeviceResult& CurResult, BOOL& bSendSuccess);
    void iPIS_Send_InspResult(VisionUnit& visionUnit, SideVisionDeviceResult& CurResult, BOOL& bSendSuccess);
    void iPIS_Send_DLInspResult(VisionDeviceResult& CurResult, BOOL& bSendSuccess);
    void iPIS_Send_Start_Handler(long nMessage);
    void iPIS_ChangeDisplay(short nTarget);
    void iPIS_HandlerStop();
    void iPIS_Send_ErrorMessageForPopup(CString message);
    void iPIS_Send_ErrorMessageForLog(CString category, CString message);
    void iPIS_Send_JobdownloadAck(BOOL done, CString message);
    void iPIS_Send_GrabMove(long nGrabDir, long nMessage, float scanLength3D_mm = -1.f, bool isGoldenDevice = false);
    void iPIS_Send_GrabRetry();
    void iPIS_MarkTeach_Skip();
    void iPIS_MarkTeach_Done();
    void SendToHost_MarkImage(std::vector<CString>& vecstrMarkImagePath);
    void SendVersionInfo();
    void iPIS_Send_SystemParameters();
    VisionSystemParameters MakeVisionSystemParameters();
    VisionSystemParameters MakeVisionSystemParametersFor2DandSide();
    void AddVisionSystemParametersFor2DIllum(const BOOL bIsRef, VisionSystemParameters& o_visionParams);
    void AddVisionSystemParametersForSideIllum(const BOOL bIsRef, VisionSystemParameters& o_visionParams);
    VisionSystemParameters MakeVisionSystemParametersFor3D();
    VisionSystemParameters MakeVisionSystemParametersForNGRV(); //kircheis_820.190
    CString GetIllumCalTypeString(long nIllumCalType);
    long GetIllumCalTypeLong(CString strIllumCalType);
    void iPIS_Send_MultiGrabProbePosZ(
        long nProbePosZ); //Normal Pos : 0(_MULTI_GRAB_POS_NORMAL)  High Pos : 1(_MULTI_GRAB_POS_HIGH)
    void iPIS_Send_SystemParametersMatchResult(VisionSystemParameters& receivedSysParam);
    void SetVisionSystemParameters(VisionSystemParameters visionSysParams);
    void iPIS_Send_SystemParametersChanged(BOOL bChanged);
    void iPIS_Send_TextRecipe(); //kircheis_TxtRecipe
    void iPIS_Send_VisionLogFile(const CString i_strFileName,
        const CString i_strFilePath); //mc_Vision Log File을 Host에게 전송한다 저장되는 이름은 i_strFileName
    void
    iPIS_Send_3D_Vision_StartIO_NotOff(); //mc_3D Vision에서 Start I/O가 꺼지지 않으면 Handler에 이상하다고 알려준다

    //{{//kircheis_NGRV_MSG
    void
    iPIS_Send_InspectionItemsName(); //일반 비전용. vector CString 전송. 각 Vision이 검사하는 검사 Module 이름을 벡터에 담아 송부.Recipe Upload시 송부
    void iPIS_Send_WarningBTM2DRecipe(); //NGRV Lot Start가 수신되었을 때, BTM2D가 NGRV Recipe가 아님을 알림
    void iPIS_Send_NGRV_WarningVisionName(); //NGRV Vision 전용. 검사 모듈 이름에 Vision Name이 누락된 경우 발생
    void
    iPIS_Send_NGRV_WarningRecipeType(); //NGRV Vision 전용. 검사 Download 받은 Recipe가 NGRV 전용 Recipe가 아닐 경우 발생
    void iPIS_Send_NGRV_GrabZposInfo(std::vector<NgrvGrabPositionInfo>
            vecGrabPosInfo); //NGRV Vision 전용. vector Class NgrvGrabPositionInfo. IR 조명 사용에 따른 Grab Height 관련 정보를 전달.  NGRV의 Recipe Upload 시 송부
    void iPIS_Send_NGRV_CompleteImageSave(NgrvGrabbedImageInfo
            grabbedImgFileInfo); //NGRV Vision 전용. Class NgrvGrabbedImageInfo. 하나의 Defect에 대한 영상 저장이 완료되었음을 알림
    void iPIS_Send_NGRV_GrabFrameInfo(NgrvFrameInfo
            ngrvGrabFrameInfo); //NGRV Vision 전용. Class NgrvFrameInfo. 각 Frame에서 획득하는 검사 모듈에 대한 정보를 전송..Recipe Upload시 송부
    void
    iPIS_Send_PackageGrabInfo_Receive_Done(); //NGRV Vision 전용. PackageGrabInfo를 Host에서 받은 후, 해당 정보를 잘 받았음을 전달
    void iPIS_Send_NGRV_AF_Info(); //kircheis_NGRVAF//2D Vision만 사용. NGRV AF Setup 정보를 송부
    //}}

    //{{mc_Version Misamtch
    void iPIS_Send_VersionMismatchErrorPopup();
    //}}

    //{{JHB_NGRV SingleRun 2D Info - MED#6
    void iPIS_Send_NGRV_SingleRunInfo(); // NGRV SingleRunInfo를 Host로 송부한다
    void iPIS_Send_2D_Vision_Image_Info_To_NGRV(); // 2D Vision에서의 Image 정보를 NGRV로 넘겨주기 위해 Host로 정보 전달
    void
    iPIS_Send_NGRV_2DVisionInfo_Receive_Done(); // NGRV Vision 전용. 2D Vision Info를 Host에서 받은 후, 해당 정보를 잘 받았음을 전달
    void iPIS_Send_NGRV_RunMode_Receive_Done(); // NGRV RunMode 정보를 Host에서 받은 후, 해당 정보를 잘 받았음을 전달
    void
    iPIS_Send_NGRV_SingleRun_Visiontype_Receive_Done(); // NGRV 영상 획득 Vision Type 정보를 Host에서 받은 후, 해당 정보를 잘 받았음을 전달
    void SetSocketInfo(Ipvm::SocketMessaging*
            pMessageSocket); // Host와 Connect되었을 시에 Socket 정보를 NGRV SingleRun쪽에 전달해주기 위한 함수
    //}} MED#6

    void SetSpecNormalValue(std::vector<VisionInspectionSpec*>& specList);
    std::vector<float> GetPackageSizeSpec(CString strSpecName);
    std::vector<float> Get3DInspectionSpec(CString strSpecName);
    std::vector<float> GetBallInspectionSpec(CString strSpecName);
    std::vector<float> GetBallPixelQualityInspectionSpec(CString strSpecName); //kircheis_BPQ
    std::vector<float> GetMarkInspectionSpec(CString strMarkName);
    void GetSurfaceCriteriaSpec(CString strSurfaceName, std::vector<std::vector<SurfaceSpec>>& vecSurfaceSoec);
    SurfaceSpec GetCiriteriaSpec(VisionSurfaceCriteria_Column columnID, long nColor, SItemSpec iTemSpecMin,
        SItemSpec iTemSpecMax, CString strUnit);

    bool IsInlineMode() const;

    bool CheckIntegrityOfCompMapData(); //kircheis_Comp3DHeightBug

    bool Init_NGRV_SingleRunInfo();
    bool Copy_NGRV_SingleRunInfo(const long& VisionType, const VisionScale& scale, const long& imageSizeX,
        const long& imageSizeY, const long& defectNumber, const std::vector<CString>& vecDefectName,
        const std::vector<CString>& vecItemID, const std::vector<CString>& vecDefectCode,
        const std::vector<long>& vecDisposition, const std::vector<long>& vecDecision,
        const std::vector<Ipvm::Point32s2>& vecStitchCount, const std::vector<long>& vecAFtype,
        const std::vector<float>& vecGrabHeightOffset_um, const std::vector<std::vector<BOOL>>& vec2IllumUsage,
        const std::vector<Ipvm::Rect32s>& vecGrabPosROI);

    void SetSampleImageOption(const long i_nSampleImageOption); //mc_MED#3.5 AllFrameSave

    //{{//kircheis_NGRV_MSG
    std::vector<CString>
        m_vecstrInspModuleNameNGRV; //NGRV Vision 전용. Host로 부터 검사 Vision들이 검사하는 Insp Module의 이름을 받아 관리
    std::vector<NgrvPackageGrabInfo>
        m_vecPackageGrabInfo; //NGRV Vision 전용. Host로 부터 Tray에 담긴 각 Package의 Grab 정보를 얻는다.
    std::vector<CString> m_vecstrEachVisionName; //NGRV Vision 전용. Host로 부터 다른 검사 비젼의 이름을 받는다.
    std::vector<NgrvGrabbedImageInfo> m_vecGrabbedImageInfo;
    std::vector<NgrvGrabPositionInfo>
        m_vecGrabPositionInfo; //NGRV Vision 전용. 각 검사 모듈 영상의 Frame 수를 관리한다. Normal Pos + IR Pos
    NgrvFrameInfo m_NGRVFrameInfo;

    BOOL m_bIsRecipeForNGRVVision; //Recipe에 저장. Recipe가 NGRV Vision용임을 확인.
    BOOL
        m_bIsReadOnly2DMatMode; //Recipe에 저장. Recipe가 2D BTM에서 2D Matrix 전용 임을 확인. 위의 변수와 연계하여 둘다 On이면 안된다. 우선권은 위의 변수다.

    std::vector<CGrabItemFrameInfoNGRV>
        m_vecGrabItemFrameInfo; //각 검사 항목 별 조명 Frame 번호를 저장하는 변수로  Recipe로 관리가 필요
    std::vector<std::vector<CString>> m_vec2InspItemNGRV; //각 Vision의 검사 항목 리스트
    std::vector<CNGRVImageSet> m_vecNGRVImageSet; //이미지 저장 클래스
    CNGRVImageSet m_ngrvImageSetBuffer;

    NgrvAfRefInfo m_ngrvAFRefInfor_BTM;
    NgrvAfRefInfo m_ngrvAFRefInfor_TOP;

    BOOL ClassifyInspItemInfo(std::vector<CString>
            vecstrReceivedInspItemName); //Host에서 받은 검사항목 List를 분류하고 이를 기반으로 m_vec2InspItemNGRV와 m_vecGrabItemFrameInfo를 재구성
    BOOL ClassifyInspItemInfo(std::vector<CGrabItemFrameInfoNGRV>
            vecGrabItemFrameInfoNGRV); //Recipe Open 후 m_vecGrabItemFrameInfo로 m_vec2InspItemNGRV를 재구성
    BOOL NGRV_FrameInfoLinkDataBase(
        BOOL bSave, long nIndex, CiDataBase& JobDB, CGrabItemFrameInfoNGRV& io_grabItemFrameInfoNGRV);
    BOOL NGRV_AFInfoLinkDataBase(BOOL bSave, CiDataBase& JobDB);
    BOOL NGRV_2DImageInfoLinkDataBase(BOOL bSave, CiDataBase& JobDB); // MED#6 SingleRun 2DImageInfo
    void SetNGRV_SaveOption(CString strInspModuleName, BOOL bSave);
    BOOL GetNGRV_SaveOption(CString strInspModuleName);
    long GetVisionID_InspModule(CString strInspModuleName);

    BOOL GetFrameInfoNGRV(CString i_strInspModuleName, BOOL& o_bUseIR, std::vector<long>& o_vecFrameID);
    BOOL GetFrameInfoNGRV(CString i_strInspModuleName, BOOL& o_bUseIR, BOOL& o_bUseUV, std::vector<long>& o_vecFrameID,
        std::vector<long>& o_vecUVIRFrameID); //kircheis_NGRVGain
    BOOL GetFrameInfoNGRV_SingleRun(CString i_strInspModuleName, BOOL& o_bUseIR, std::vector<long>& o_vecFrameID);
    BOOL GetFrameInfoNGRV_SingleRun(CString i_strInspModuleName, BOOL& o_bUseIR, BOOL& o_bUseUV,
        std::vector<long>& o_vecFrameID, std::vector<long>& o_vecUVIRFrameID); //NGRV SingleRun - MED#6

    void GetNGRVFrameInfo(NgrvFrameInfo& o_ngrvFrameInfo);
    void GetNGRVGrabPositionInfo(std::vector<NgrvGrabPositionInfo>& o_vecGrabPositionInfo);

    void SetReadySignal(BOOL bActive); //NGRV와 다른 비전들의 Ready I/O 번호가 달라 통합관리다 필요하다.
    //}}

public:
    void ShowVisionInspSpec();

    // 영훈 20130911 ; inspection overlay를 관리하는 기능을 하나 추가한다.
    void ShowOverlayManager();

    long GetThreadNum();

    void OnGotoHost();

    //  현식 마크티치관련 수정
    void OnLogin();

    void OnMarkTeachSkip();
    void OnMarkTeachDone();

    // 영훈 20131226 :: 버전 관리를 한다.
    void SetFileVersion(CString strFileVersion);
    CString GetFileVersion();
    CString m_strDetailFileVersion;

    // Primary UI Dialog 표시

    void ReviewImageViewer();
    void SystemSetup();

    CWnd* GetVisionInlineDialog(long nSelect);

    // Primary UI (Docking Tool Bar Menu 관련)
    BOOL VisionPrimaryUI_OpenJobFile();
    BOOL VisionPrimaryUI_SaveJobFile();
    BOOL VisionPrimaryUI_OpenImageFile();
    BOOL VisionPrimaryUI_SaveImageFile();

    // Image File 을 선택하여 Open
    BOOL OpenImageFile();
    BOOL OpenImageFile(CString strFilename);
    BOOL DoInspection();

    void SetLiveMain();
    void ResetLiveMain();

    // Job 관련 함수
    BOOL OpenJobFile();
    void SaveDumpImageFiles(LPCTSTR dumpPath, LPCTSTR fileVersion);
    BOOL SynchronizeJobFile(
        BOOL bCallLinkDatabase = TRUE); // 기본 VisionUnit 의 내용을 다른 녀석들에 전파시키기 위한 것임

    // Package Type 관련 함수
    BOOL SetPackageType();

    // Grab 함수들
    bool ManualGrab2D(VisionImageLot& imageLot,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    bool ManualGrab3D(VisionImageLot& imageLot);
    bool ManualDualGrab3D(VisionImageLot& imageLot);
    bool ManualGrabNGRV(VisionImageLot& imageLot);
    bool ManualGrab3DSequence(VisionImageLot& imageLot);

    bool Check3DNoise(
        VisionImageLot& imageLot, bool bFullSizeInsp); // 3D Vision에서의 2D 그랩을 통한 3D Noise 탐지를 진행하는 함수

public:
    SystemConfig& m_systemConfig;
    long m_nThreadNum;
    CVisionPrimaryUI* m_pVisionPrimaryUI;
    CVisionInlineUI* m_pVisionInlineUI;
    VisionMainMenuAccess* m_menuAccessUI;

    long m_nSendImageSizeX;
    long m_nSendImageSizeY;

    SlitBeam3DParameters* m_pSlitBeam3DCommon;

    /////////////////쓰레드 관련

    //<< Thread

    InlineGrab* m_inline_grab;

    virtual long GetInlineRunCnt() const override;
    virtual void SetInlineRunCnt(long inlineRunCnt) override;
    void SetScanTrayIndex(long trayIndex);
    void SetInspectionRepeatIndex(
        long repeatIndex); //mc_KMAT Crash, Host에서 repeat검사 진행 간, crash발생으로 인하여 vision에서 정보를 넘겨줌
    long GetInspectionRepeatIndex();

    void SetTrayScanSpec(const VisionTrayScanSpec& spec);
    void Iocheck();

    CString GetPrevJob();

    void TestSetup();
    virtual void SetImageSaveOption(ImageSaveOptionData i_save) override;

    void RawImageSave(VisionUnit& visionUnit, const CString i_strImagePath, CString& o_strSaveRawImageFilePath);
    void ResultTextSave(
        const CString i_strLogData, const long i_nTrayIndex, const long i_scanID, const long i_paneID, LPCTSTR i_path);
    void DrawRectangle(Ipvm::Image8u3& image, const Ipvm::Rect32s& roi, COLORREF rgbColor, long inflate = 0);
    void PathToSaveImage(VisionUnit& visionUnit, const long i_nTrayIndex, const long i_nScanIndex, long i_nPaneIndex,
        CString* o_strRawImagePath, CString* o_strRawJabPath, CString* o_strReviewImagePath,
        CString* o_strCroppingImagePath);
    BOOL NameToSaveImageforReview(ImageSaveOptionData& option, const long i_nFrameID, const long i_nInspMin,
        const long i_nInspMax, const std::vector<VisionInspectionResult*> i_pvecVisonInspResult,
        CString* o_strSaveImageName);
    void NameToSaveImageforCropping(const CString i_strInspName, const CString i_strResultName, CString i_fileRealPath,
        const CString i_strObjType, const long i_nFrameID, const BOOL i_bIsColor, CString* o_strSaveImageName,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    BOOL InspectionResult(const long i_nThreadNum, const long i_nInspMin, const long i_nInspMax,
        const std::vector<VisionInspectionResult*> i_pvecVisonInspResult, long* o_nFrameID, long* o_nResult);
    BOOL GetFreeSpaceCheck(const long i_nFreeSpaceMinGB, LPCTSTR checkDiskPath);

    BOOL RawImageSaveResult(
        const ImageSaveOptionData& option, const long i_nTotalResult, const bool i_bAllFrameSaveMode = false);
    long ConvertResult(const long i_nTotalResult);

    VisionUnit& GetPrimaryVisionUnit();

    //kk Grab Retry 관련 변수
    BOOL m_bGrabRetry;

    BOOL GetErrorInfo(long& o_nErrorCode,
        CString& o_srtErrorContent); //kircheis_MED3_SendInfo //Error이 없으면 이 함수는 return false
    void GetIllumInfo(std::vector<CString>& vecstrIllumExposureTimeMS); //kircheis_MED3_SendInfo

    CString m_strRecipeFileVersion;
    CString GetRecipeFileVersion();

    std::vector<CString> getAllFrame_2D_FilePath(const bool i_bisRawFilePath = false);

    NgrvAfRefInfo m_ngrvAFRefInfo; //kircheis_NGRVAF

    //{{ JHB_MED#6 - NGRV SingleRun 2D Info
    Ngrv2DImageInfo m_ngrv2DImageInfo;
    Ngrv2DImageInfo m_ngrv2DImageInfo_BTM;
    Ngrv2DImageInfo m_ngrv2DImageInfo_TOP;
    //}}

private:
    VisionUnit* m_primaryVisionUnit;
    VisionUnit** m_unitBuffers;
    ImageSaveOptionData m_manualImageSaveOption;

    CString m_lastLinkDataBaseErrorMessage;
    BOOL m_lastLinkDataBaseJobVersionChanged;

    const ImageSaveOptionData& GetImageOption(bool isManualInsp) const;

    Ipvm::Rect32s GetReviewRegion(bool isManualInsp, VisionUnit& visionUnit);
    void SetImageSave(VisionUnit& visionUnit, const ImageSaveOptionData& option, const Ipvm::Rect32s& reviewRegion,
        const long i_nTrayIndex, const long i_scanID, const long i_paneID, const CString i_strLogData,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void InlineImageSave_Review(VisionUnit& visionUnit, const ImageSaveOptionData& option,
        const Ipvm::Rect32s& reviewRegion, LPCTSTR realReviewImagePath, LPCTSTR logData,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void InlineImageSave_Cropping(VisionUnit& visionUnit, const ImageSaveOptionData& option, LPCTSTR croppingImagePath,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    BOOL ImageSave_Review(VisionUnit& visionUnit, const ImageSaveOptionData& option, const Ipvm::Rect32s& reviewRegion,
        LPCTSTR moduleGuid, LPCTSTR moduleName, const long frameIndex,
        const std::vector<Ipvm::Rect32s>& i_vecrtRejectROI, const std::vector<Ipvm::Rect32s>& i_vecrtMarginalROI,
        LPCTSTR imageFilePath, CString i_strRejectNames,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void InlineImageSave_AllFrame(VisionUnit& visionUnit, const ImageSaveOptionData& option,
        std::vector<CString>& o_vecstr2DReviewAll_FrameFilePath, CString& o_str2DReview_AllFrameColorImageFilePath,
        std::vector<CString>& o_vecstr2DRaw_AllFrameColorImageFilePath,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    BOOL ImageSave_Review_AllFrame_2D(VisionUnit& visionUnit, const ImageSaveOptionData& option, LPCTSTR moduleGuid,
        LPCTSTR moduleName, LPCTSTR imageFilePath, std::vector<CString>& o_vecstr2DReviewAll_FrameFilePath,
        CString& o_str2DReview_AllFrameColorImageFilePath,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    bool SaveCombinedImage_2D(VisionUnit& visionUnit, const LPCTSTR moduleName, const LPCTSTR moduleGuid,
        LPCTSTR imageFilePath, const long i_nReviewImageQuality,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    bool SaveColorImage_2D(VisionUnit& visionUnit, const long i_nColorFrameidx_Red, const long i_nColorFrameidx_Green,
        const long i_nColorFrameidx_Blue, LPCTSTR imageFilePath, const long i_nReviewImageQuality,
        CString& o_str2DReview_AllFrameColorImageFilePath,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void CroppingImageSave(VisionUnit& visionUnit, const ImageSaveOptionData& option, LPCTSTR moduleGuid,
        const long i_nFrameID, const CString i_strImagePath,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void CroppingSetImageFrame(VisionUnit& visionUnit, const ImageSaveOptionData& option, LPCTSTR moduleGuid,
        Ipvm::Image8u3& io_Image, long i_FrameID, long* ImageSaveOption,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void CroppingSetRejectROI(
        VisionInspection* inspection, const ImageSaveOptionData& option, Ipvm::Image8u3& io_Image);
    void CheckColorImageSetup(); //kk 20190710 2D Vision일 경우 Color Image Setup이 안되어 있다고 한다면 경고를 날리자.

    ImageSaveStatus CreateImageSave_Directory(VisionUnit& visionUnit, const long i_nTrayIndex, const long i_scanID,
        const long i_paneID, CString& o_strRaw_Image_FilePath, CString& o_strReview_Image_FilePath,
        CString& o_strCropping_Image_FilePath);

    long m_nDevicePassCount;

    //mc_MED#3.5 AllFrameSave
    long m_nPassDeviceSaveCount;
    bool m_bSend2DAllFrame;
    std::vector<CString> m_vecstr2DReview_AllFrameFilePath;
    std::vector<CString> m_vecstr2DRaw_AllFrameFilePath;

    //mc_KMAT Repeat Mode Crash
    long m_nInspectionRepeatIndex;

public:
    static UINT ThreadSendResultFunc(LPVOID pParam);
    static UINT ThreadExecuteDLSW(LPVOID pParam);
    static UINT ThreadReconnectDLSW(LPVOID pParam);

    BOOL bThreadExitCheckDLSW;

    void ThreadSendResult();

    // 검사 결과를 받기 위한 Shared Memory 관련 변수
    HANDLE m_handleUpdateDevResult;
    HANDLE m_handleChkUpdateResult;

    BYTE* m_pbySharedUpdateDevResult;
    BOOL* m_pbChkUpdateResult;

    // DevResult 변환위한 버퍼
    BYTE* m_pbySendResultBuffer;

    void RuniDL();
    void TerminateDLSW();

    void ConvertDeviceResultToBuffer(BYTE* pbyBuffer, VisionDeviceResult& deviceResult, BOOL bSave);

    void SaveGrabInfoLog(std::vector<NgrvPackageGrabInfo> i_vecPackageGrabInfo, BOOL i_bFlip);
    BOOL m_bNGRV_SideFlip;
    long m_nRecipeType; // 0 : Normal, 1 : ByPass, 2 : Review
    CString m_strDefaultRecipePathNameForNGRV; // NGRV Default Recipe Name
    CString m_strDefaultRecipePathNameForByPass; // ByPass Default Recipe Name
    BOOL m_bHostConnected;

    long m_nSideVisionSection; // Side Vision Front/Rear 선택 관련 변수 : Side Front 0, Side Rear 1 JHB_2024.04.15
    BOOL m_isNGRVSingleRun; // NGRV SingleRun인지 Normal Run인지 구분하는 변수 : Normal Run 0, Single Run 1 MED#6
    BOOL
        m_isNGRVSingleRunVisionType; // NGRV SingleRun의 VisionType이, BTM인지 TOP인지 구분하는 변수 : TOP 0, BTM 1 MED#6

private: //mc_Review image Save LOG
    void SaveReviewImage_DeubgLOG(VisionUnit& visionUnit, const CString i_strRejectNames, const CString i_strModuleName,
        const CString i_strSKD_NotSuccessMSG, const eReviewimageSaveLOGCategory i_eReviewimageSaveLogCatrgory,
        const bool i_bNeedRawimageSave);

private: //mc_3D Log
    void Send_ManualGrab3D_Sequence_Error_LogMessage(
        const CString i_strCategory, const sGrab3DErrorLogMessageFormat i_sGrab3DErrorForamt);


    bool CheckVisionTypeInRecipe(const BOOL& bSave, CiDataBase& jobDB, CString& errorText);
};
