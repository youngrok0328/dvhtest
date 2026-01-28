#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../InformationModule/dPI_DataBase/IllumInfo2D.h"
#include "../../SharedCommunicationModules/VisionHostCommon/ImageSaveOptionData.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;
class DebugOptionData;
class VisionDeviceResult;
class SideVisionDeviceResult;
class VisionUnit;
class VisionImageLot;
class ImageSaveOptionData;
class InspectionAreaInfo;

//HDR_6_________________________________ Header body
//
class __VISION_COMMON_API__ VisionMainAgent
{
public:
    enum class SaveJobMode
    {
        Normal,
        SaveOnly,
    };

    VisionMainAgent(int32_t mainType = 0);
    virtual ~VisionMainAgent();

    typedef void FUNC_INSPECTION_END_CALLBACK(void* userData, VisionUnit& visionUnit, long threadIndex);

    virtual bool IsJobTeachEnabled() = 0;
    virtual bool IsAllBatchInspectionThreadsIdle() = 0;
    virtual long PushBatchImage(long batchKey, long paneIndex, LPCTSTR imageFilePath, LPCTSTR batchBarcode) = 0;
    virtual void RegisterCallBack_InspectionEnd(void* userData, FUNC_INSPECTION_END_CALLBACK* callback) = 0;
    virtual void UnRegisterCallBack_InspectionEnd(FUNC_INSPECTION_END_CALLBACK* callback) = 0;
    virtual void CopyImage_From_OtherVisionUnit_Into_PrimaryVisionUnit(long oterhVisionUnitIndex) = 0;
    virtual bool CheckFreeDiskForImageSave(bool sendErrorMessageToHost) = 0;

    virtual BOOL OpenImageFileAgent() = 0;
    virtual BOOL OpenImageFileAgent(CString strFilename) = 0;
    virtual BOOL OpenJobFileAgent() = 0;
    virtual BOOL OpenJobFile(CString strFilePath, CString strFileName) = 0;
    virtual void SetScanSpec(float scanLength_mm) = 0;
    virtual BOOL LinkDataBase(BOOL bSave, CiDataBase& JobDB) = 0;
    virtual BOOL LinkDataBase_PackageSpec(BOOL bSave, CiDataBase& JobDB) = 0;
    virtual void SetVisionJobName(CString strFilePath, CString strFileName) = 0;
    virtual void DeleteMarkMultiImages() = 0;
    virtual bool isInline() const = 0;
    virtual void SetInlineStart(bool isBatchInspection) = 0;
    virtual void SetInlineStop(bool isBatchInspection) = 0;
    virtual void SendResult(VisionUnit& visionUnit, VisionDeviceResult& DeviceResult, BOOL& bSendSuccess) = 0;
    virtual void SendResult(VisionUnit& visionUnit, SideVisionDeviceResult& SideVisionDeviceResult, BOOL& bSendSuccess)
        = 0;
    virtual void SendImage(const Ipvm::Image8u& image) = 0; //kircheis_UPH_up
    virtual void SendImage(const Ipvm::Image8u& imageFront, const Ipvm::Image8u& imageRear)
        = 0; //Side Vision Inline 전송용
    virtual void Send2DAllFrame() = 0; //mc_MED#3.5 AllFrameSave
    virtual bool isConnect() const = 0; //kircheis_UPH_up
    virtual BOOL SaveJobFile() = 0;
    virtual BOOL SaveJobFile(
        CString strFilename, SaveJobMode mode, BOOL bSendJob = FALSE, BOOL bSendPackageSpec = FALSE)
        = 0;
    virtual void OnImageGrab() = 0;
    virtual void SendToHostMarkTeachImage() = 0;
    virtual BOOL OnJobChanged() = 0;
    virtual BOOL HardwareSetup() = 0;
    virtual BOOL HardwareSetupForNGRV(std::vector<long> vecnFrameID, BOOL bUseIR, BOOL bInline) = 0;
    virtual BOOL OnImagingConditionChanged() = 0;

    virtual void PostExecutionProcessing(bool isManualInsp, VisionUnit& visionUnit, VisionDeviceResult& deviceResult,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT)
        = 0;

    virtual void SaveSystemInfo() = 0; //	전호빈 - 2020.04.19 : System Info 저장 관련 함수
    virtual bool GetSend2DAllFrame() = 0; //mc_MED#3.5 AllFrameSave
    virtual void ResetDevicePassCount() = 0; //mc_MED#3.5 AllFrameSave

    virtual long GetInlineRunCnt() const = 0;
    virtual void SetInlineRunCnt(long inlineRunCnt) = 0;

    //mc_InlineGrabTime[GPU Test]
    virtual void ResetInlineGrabTime() = 0;
    virtual void SaveInlineGrabTime() = 0;

    //SDY_Automation
    virtual void SendToAutomationBatchEnd(long nInspTime) = 0;

    // Side Front/Rear 구분을 위한 함수 - JHB_2024.04.15
    virtual void SetSideVisionSection(long i_nSideSection) = 0;
    virtual long GetSideVisionSection() = 0;

    virtual const InspectionAreaInfo& getInspectionAreaInfo() = 0;

    virtual void SetImageSaveOption(ImageSaveOptionData i_save) = 0;
    ImageSaveOptionData m_saveOption;

    BOOL m_bMarkTeach;

    CString m_lotID; // Lot ID
    CString m_lotReviewFileHeader; // Lot 에 대한 Review File저장시 헤더부분 (Host에서 받음)
    CTime m_timeLotStart; // Lot 시작 시간

    CString m_strJobFileName;
    CString m_strJobFilePath;

    DebugOptionData* m_ImageSaveOption;
    DebugOptionData* m_DebugStopOption;
    Ipvm::SocketMessaging* m_pMessageSocket;
    Ipvm::SocketMessaging* m_tuningMessageSocket;
    Ipvm::SocketMessaging* m_automationMessageSocket;

    HWND m_hwndMainFrame;
    UINT m_msgShowTeach;
    UINT m_msgShowMain;
    UINT m_msgAccessModeChanged;

    IllumInfo2D m_illum2D;

    //<< Inspection Result Thread Control // ksy 20.05.18 딥러닝으로 업데이트된 검사 결과를 받기 위한 Thread
    struct SResultSendThreadControl
    {
        HANDLE m_Signal_KillThread{}; //
        HANDLE m_Signal_ThreadKilled{};
        HANDLE m_Flag_ThreadRun{};
        HANDLE m_Signal_SendResult{};
    };

    SResultSendThreadControl m_Control_SendResult;
    SResultSendThreadControl m_Control_SendConfirm;

    const int32_t m_mainType;
};
