//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionMainTR.h"

//CPP_2_________________________________ This project's headers
#include "CallBackList_InspectionEnd.h"
#include "DlgOverlayManager.h"
#include "DlgVisionInspSpec.h"
#include "InlineGrab.h"
#include "InspectionOrderer.h"
#include "MainDefine.h"
#include "VisionIlluminationSetup.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../DefineModules/dA_Base/SimpleFunction.h"
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../HardwareModules/dPI_Framegrabber/Framegrabber.h"
#include "../../HardwareModules/dPI_Framegrabber/Framegrabber_Base.h"
#include "../../HardwareModules/dPI_SyncController/SlitIlluminator.h"
#include "../../HardwareModules/dPI_SyncController/SyncController.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_DataBase/SlitBeam3DParameters.h"
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../InformationModule/dPI_SystemIni/PersonalConfig.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../InspectionItems/VisionInspectionMark/VisionInspectionMark.h"
#include "../../InspectionItems/VisionInspectionMark/VisionInspectionMarkSpec.h"
#include "../../InspectionItems/VisionInspectionSurface/VisionInspectionSurface.h"
#include "../../MainUiModules/VisionBatchInspectionUITR/VisionBatchInspectionUI.h"
#include "../../MainUiModules/VisionInlineUITR/VisionInlineUI.h"
#include "../../MainUiModules/VisionPrimaryUITR/DlgVisionPrimaryUI.h"
#include "../../MainUiModules/VisionPrimaryUITR/VisionPrimaryUI.h"
#include "../../ManagementModules/VisionUnitTR/VisionUnit.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../ProcessingItems/VisionProcessingNGRV/VisionProcessingNGRV.h" //MED#6_NGRV_JHB
#include "../../ProcessingItems/VisionProcessingNGRV/VisionProcessingNGRVPara.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/Analysis.h"
#include "../../SharedCommunicationModules/VisionHostCommon/ImageSaveOption.h"
#include "../../SharedCommunicationModules/VisionHostCommon/ImageSaveOptionData.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionDeviceResult.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionResult.h"
#include "../../SharedCommunicationModules/VisionHostCommon/iPIS_MsgDefinitions.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ImageViewEx/ImageViewEx.h"
#include "../../UtilityMacroFunction.h"
#include "../../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionMainMenuAccess.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"
#include "../../iPack/AutomationMsgDef.h" //SDY_Automation

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/ColorMap.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Image8u3.h>
#include <Ipvm/Base/ImageFile.h>
#include <Ipvm/Base/Point8u3.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Base/TimeCheck.h>
#include <Ipvm/Gadget/Enum.h>
#include <Ipvm/Gadget/Miscellaneous.h>
#include <Ipvm/Gadget/Zipper.h>
#include <Ipvm/Widget/AsyncProgress.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define FloppingImageInflate 20
#define UPDATE_DEVRES_SHARE_MEMNAME _T("iPack2ndInspUpdateDevResultSharedMemorys")
#define CHK_INSP_RESULT_UPDATE_SHAREDMEM _T("iPack2ndInspCheckInspResultUpdate")

//CPP_7_________________________________ Implementation body
//
static CMutex mutex(FALSE, _T("Vision PassImageSave"), nullptr);

void LogMaster(CString _strFileName, CString _strLogData, long nThread, BOOL _bIncludeTimeInName)
{
    SYSTEMTIME time;
    ::GetLocalTime(&time);

    CString strFileName;

    if (_bIncludeTimeInName)
    {
        strFileName.Format(_T("C:\\IntekPlus\\iEasyAI\\Vision_x64\\Log\\%02d_%02d_%02d_%02d_%s%d.csv"), time.wYear,
            time.wMonth, time.wDay, time.wHour, (LPCTSTR)_strFileName, nThread);
    }
    else
    {
        strFileName.Format(_T("C:\\IntekPlus\\iEasyAI\\Vision_x64\\Log\\%s.csv"), (LPCTSTR)_strFileName);
    }

    CT2A asii(strFileName);
    FILE* fp{};
    fopen_s(&fp, asii.m_psz, "a");

    if (fp != NULL)
    {
        ::GetLocalTime(&time);
        fprintf(fp, "%ls\n", (LPCTSTR)_strLogData);
        fclose(fp);
    }

    strFileName.Empty();
}

UINT VisionMainTR::ThreadReconnectDLSW(LPVOID pParam)
{
    VisionMainTR* pMain = (VisionMainTR*)pParam;
    while (!pMain->bThreadExitCheckDLSW)
    {
        HWND hWnd = ::FindWindow(NULL, _T("iEasyAI"));

        if (hWnd == 0)
        {
            pMain->m_systemConfig.m_bStateiDL = FALSE;
        }
        else
        {
            Sleep(30000);
            pMain->m_systemConfig.m_bStateiDL = TRUE;
        }
        Sleep(10);
    }

    return 0;
}

UINT VisionMainTR::ThreadExecuteDLSW(LPVOID pParam)
{
    VisionMainTR* pMain = (VisionMainTR*)pParam;

    STARTUPINFO startUpInfo = {0};
    PROCESS_INFORMATION processInfo;
    startUpInfo.cb = sizeof(STARTUPINFO);

    BOOL bRet = ::CreateProcess(_T("C:\\intekplus\\iEasyAI\\Vision_x64\\Bin\\iEasyAI.exe"), NULL, NULL, NULL, FALSE, 0,
        NULL, NULL, &startUpInfo, &processInfo);
    if (!bRet)
    {
        AfxMessageBox(_T("iEassyAI.exe was not executed!!"));
        return 0;
    }
    else
    {
        pMain->m_systemConfig.m_bStateiDL = TRUE;
    }

    WaitForSingleObject(processInfo.hProcess, INFINITE);

    //pMain->m_systemConfig.m_bUseAiInspection = FALSE;
    pMain->m_systemConfig.m_bStateiDL = FALSE;

    DWORD exitCode;
    GetExitCodeProcess(processInfo.hProcess, &exitCode);

    CloseHandle(processInfo.hProcess); // 프로세스 핸들 해제
    CloseHandle(processInfo.hThread); // 스레드 핸들 해제

    return 0;
}

UINT VisionMainTR::ThreadSendResultFunc(LPVOID pParam)
{
    VisionMainTR* pMain = (VisionMainTR*)pParam;

    pMain->ThreadSendResult();

    return 0;
}

void VisionMainTR::ThreadSendResult()
{
    BOOL bQuit = FALSE;

    HANDLE pHandles[] = {m_Control_SendResult.m_Flag_ThreadRun, m_Control_SendResult.m_Signal_KillThread};
    const DWORD nSignalCount = sizeof(pHandles) / sizeof(HANDLE);

    //CMutex Mutex(FALSE, PGM_MUTEX_NAME);
    while (1)
    {
        switch (::WaitForMultipleObjects(nSignalCount, pHandles, FALSE, INFINITE))
        {
            case WAIT_OBJECT_0:
            {
                m_mutSend.Lock();
                // LOG
                CString strID;
                CString strResult;

                VisionDeviceResult* devResult = new VisionDeviceResult;

                if (m_pbySharedUpdateDevResult != NULL)
                {
                    ConvertDeviceResultToBuffer(m_pbySharedUpdateDevResult, *devResult, FALSE);
                }

                BOOL bSuccess = FALSE;
                CString strLog;
                strLog.Format(_T("DLResult >> QueuePush = TrayID : %d, PocketID : %d, PaneID : %d"),
                    devResult->m_nTrayID, devResult->m_nPocketID, devResult->m_nPane);
                LogMaster(_T("SendFinalResultToHost"), strLog, 0, TRUE);
                SendDLResult(*devResult, bSuccess);
                *m_pbChkUpdateResult
                    = FALSE; // 08.13.20 KSY D/L 검사 지연 시 비전 결과 전송 관련 (iEasyAI 검사 S/W와 동기화 위한 변수)

                delete devResult;

                ::ResetEvent(m_Control_SendResult.m_Flag_ThreadRun);

                strID.Empty();
                strResult.Empty();
                strLog.Empty();

                m_mutSend.Unlock();
            }
            break;
            case WAIT_OBJECT_0 + 1:
                bQuit = TRUE;
                break;
        }

        if (bQuit)
        {
            break;
        }
    }
}

CGrabItemFrameInfoNGRV::CGrabItemFrameInfoNGRV()
{
    m_strGrabItemName.Empty();
    m_bIsSaveImage = false;
    m_vecnFrameID.clear();
    m_vecnFrameID.push_back(0);
    m_nAFType = NgrvGrabPositionInfo::enumNGRVAFMode::NGRV_AF_MODE_EACH_POINT;
    m_nPlaneOffsetUM = 0;
}

CGrabItemFrameInfoNGRV::~CGrabItemFrameInfoNGRV()
{
    m_vecnFrameID.clear();
}

CNGRVImageSet::CNGRVImageSet()
{
    Init();
}

CNGRVImageSet::~CNGRVImageSet(void)
{
}

void CNGRVImageSet::Init()
{
    m_nTrayID = -1;
    m_nPackageID = -1;
    m_strPackageID.Empty();
    m_nGrabPointID = -1;
    m_strInspectionItemName.Empty();
    m_strReviewSideName.Empty();
    m_vecnFrameID.clear();
    m_vecImageForSave.clear();
}

void CNGRVImageSet::SetInfo(long nTrayID, long nPackageID, CString strPackageID, long nGrabPointID,
    CString strInspItemName, CString strReviewSideName, std::vector<long> vecnFrameID, long nStitchCntX,
    long nStitchCntY, long nImageSizeX, long nImageSizeY)
{
    Init();
    SetImageBufferSize(vecnFrameID, nImageSizeX, nImageSizeY);

    m_nTrayID = nTrayID;
    m_nPackageID = nPackageID;
    m_strPackageID = strPackageID;
    m_nGrabPointID = nGrabPointID;
    m_strInspectionItemName = strInspItemName;
    m_strReviewSideName = strReviewSideName;
    m_nStitchCntX = nStitchCntX;
    m_nStitchCntY = nStitchCntY;
}

void CNGRVImageSet::SetImageBufferSize(std::vector<long> vecnFrameID, long nImageSizeX, long nImageSizeY)
{
    Init();
    long nFrameNum = (long)vecnFrameID.size();
    if (nFrameNum < 1)
        return;

    m_vecnFrameID.resize(nFrameNum);
    m_vecImageForSave.resize(nFrameNum);

    for (long nFrame = 0; nFrame < nFrameNum; nFrame++)
    {
        m_vecnFrameID[nFrame] = vecnFrameID[nFrame];
        m_vecImageForSave[nFrame].Create(nImageSizeX, nImageSizeY);
        m_vecImageForSave[nFrame].FillZero();
    }
}

void CNGRVImageSet::SetImage(long nStitchID, std::vector<Ipvm::Image8u3> vecImageForSave, const BOOL& isNGRV_SingleRun)
{
    int nFrameNum = (long)min(m_vecnFrameID.size(), vecImageForSave.size());
    int nImageSizeX = 0;
    int nImageSizeY = 0;
    int nTargetImageX = 0;
    int nTargetImageY = 0;
    int nOffsetX = 0;
    int nOffsetY = 0;
    Ipvm::Rect32s rtDstROI;

    for (int nFrame = 0; nFrame < nFrameNum; nFrame++)
    {
        int Realindex = isNGRV_SingleRun == TRUE ? m_vecnFrameID[nFrame] : nFrame;

        if (Realindex > vecImageForSave.size())
            continue;

        nImageSizeX = vecImageForSave[Realindex].GetSizeX();
        nImageSizeY = vecImageForSave[Realindex].GetSizeY();
        nTargetImageX = m_vecImageForSave[nFrame].GetSizeX();
        nTargetImageY = m_vecImageForSave[nFrame].GetSizeY();

        if (nImageSizeX == nTargetImageX && nImageSizeY == nTargetImageY)
        {
            Ipvm::ImageProcessing::Copy(
                vecImageForSave[Realindex], (Ipvm::Rect32s(vecImageForSave[Realindex])), m_vecImageForSave[nFrame]);
            continue;
        }
        else if (nTargetImageX == (nImageSizeX * m_nStitchCntX) && nTargetImageY == (nImageSizeY * m_nStitchCntY))
        {
            rtDstROI = Ipvm::Rect32s(0, 0, nImageSizeX, nImageSizeY);

            if (nStitchID > 0)
            {
                nOffsetX = (int)(nStitchID % m_nStitchCntX) * nImageSizeX;
                nOffsetY = (int)(nStitchID / m_nStitchCntX) * nImageSizeY;
                rtDstROI.OffsetRect(nOffsetX, nOffsetY);
            }

            Ipvm::ImageProcessing::Copy(vecImageForSave[Realindex], (Ipvm::Rect32s(vecImageForSave[Realindex])),
                rtDstROI, m_vecImageForSave[nFrame]);
        }
    }
}

VisionMainTR::VisionMainTR(SystemConfig& systemConfig, int32_t mainType, HANDLE flag_InlineStart, HANDLE flag_SnycStart)
    : VisionMainAgent(mainType)
    , m_systemConfig(systemConfig)
    , m_nThreadNum(m_systemConfig.GetThreadNum())
    , m_unitBuffers(new VisionUnit*[m_nThreadNum + 1])
    , m_primaryVisionUnit(nullptr)
    , m_pVisionPrimaryUI(nullptr)
    , m_pVisionInlineUI(nullptr)
    , m_menuAccessUI(nullptr)
    , m_pSlitBeam3DCommon(new SlitBeam3DParameters)
    , m_lastLinkDataBaseJobVersionChanged(FALSE)
    , m_bGrabRetry(FALSE)
    , m_nPassDeviceSaveCount(0)
    , m_nDevicePassCount(0)
    , m_bSend2DAllFrame(false)
    , m_mutSend(FALSE, _T("SendDLResult"))
    , bThreadExitCheckDLSW(FALSE)
    , m_bIsRecipeForNGRVVision(false) //kircheis_NGRV
    , m_bIsReadOnly2DMatMode(false) //kircheis_NGRV
    , m_mutNGRV(FALSE, _T("EraseNGRVinfo")) //JHB_NGRV
    , m_bNGRV_SideFlip(FALSE)
    , m_bHostConnected(FALSE) //Host와 Connect 되었는지 확인
    , m_strDefaultRecipePathNameForNGRV(_T(""))
    , m_strDefaultRecipePathNameForByPass(_T(""))
    , m_nInspectionRepeatIndex(0)
    , m_nSideVisionSection(0)
    , m_isNGRVSingleRun(FALSE)
    , m_isNGRVSingleRunVisionType(TRUE)
    , m_trVisionTypeInRecipe(-1)
    , m_Flag_Inline_Start(flag_InlineStart)
    , m_Flag_Sync_Start(flag_SnycStart)
{
    for (long index = 0; index < m_nThreadNum + 1; index++)
    {
        m_unitBuffers[index] = new VisionUnit(m_illum2D, *this);
    }
    
    //{{//kircheis_NGRV
    m_vecstrEachVisionName.resize(4);
    for (long i = 0; i < 4; i++)
        m_vecstrEachVisionName[i].Empty();
    //}}

    m_primaryVisionUnit = m_unitBuffers[m_nThreadNum];
    m_pVisionPrimaryUI = new CVisionPrimaryUI(*this, *m_primaryVisionUnit);

    m_inline_grab = new InlineGrab(this, m_systemConfig.GetGrabBufferNum(), m_unitBuffers, m_nThreadNum);

    if (PersonalConfig::getInstance().isSaveReviewImageOptionEnableInBatchInspection())
    {
        m_manualImageSaveOption.m_bisSaveReviewImage = TRUE;
        m_manualImageSaveOption.m_bReviewImageOption_Pass = TRUE;
        m_manualImageSaveOption.m_bReviewImageOption_Reject = TRUE;
        m_manualImageSaveOption.m_bReviewImageOption_Marginal = FALSE;
        m_manualImageSaveOption.m_nReviewImageROI = ImageSaveOption::ReviewROI_Insert;
        m_manualImageSaveOption.m_bReviewImageCombined = TRUE;
        m_manualImageSaveOption.m_nReviewImageQuality = 30;
        m_manualImageSaveOption.m_nReviewImageMargin = 100;
    }

    if (PersonalConfig::getInstance().isSaveCropImageOptionEnableInBatchInspection())
    {
        m_manualImageSaveOption.m_bisSaveCroppingImage = TRUE;
        m_manualImageSaveOption.m_nCroppingImageSaveOption = ImageSaveOption::CroppingImage_NotPass;
        m_manualImageSaveOption.m_nCroppingImageROI = ImageSaveOption::CroppingROI_Insert;
        m_manualImageSaveOption.m_nCroppingImageFrame_Ball = ImageSaveOption::CroppingFrame_NotSave;
        m_manualImageSaveOption.m_nCroppingImageFrame_Land = ImageSaveOption::CroppingFrame_Inspection;
        m_manualImageSaveOption.m_nCroppingImageFrame_Comp = ImageSaveOption::CroppingFrame_Frame01;
        m_manualImageSaveOption.m_nCroppingImageFrame_Surface = ImageSaveOption::CroppingFrame_Color;
        m_manualImageSaveOption.m_nCroppingImageQuality = 30;
    }

    if (0)
    {
        m_manualImageSaveOption.m_vecRawImageSaveOption.clear();
        m_manualImageSaveOption.m_vecRawImageSaveOption.push_back(ImageSaveOption::RawImage_AllPackages);
    }

    if (m_systemConfig.m_bUseAiInspection)
    {
        RuniDL();
        //m_mutSend =
    }

    // Default NGRV&Bypass Recipe Setting - JHB_NGRV
    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP
        || SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_2D_INSP
        || SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_TR)
    {
        // NGRV Recipe
        m_strDefaultRecipePathNameForNGRV
            = DynamicSystemPath::get(DefineFolder::Temp) + _T("Default_NGRV.vmjob"); // 저장할 vmjob root
        CString strNGRVDefaultRecipeDllPathName
            = DynamicSystemPath::get(DefineFolder::Binary) + _T("Default_NGRV.dll"); // 불러올 Define Dll
        ::CopyFile(strNGRVDefaultRecipeDllPathName, m_strDefaultRecipePathNameForNGRV,
            TRUE); // dll을 vmjob으로 변경하고 Temp folder로 옮긴다

        // ByPass Recipe
        m_strDefaultRecipePathNameForByPass = DynamicSystemPath::get(DefineFolder::Temp) + _T("Default_BYPASS.vmjob");
        CString strByPassDefaultRecipeDllPathName
            = DynamicSystemPath::get(DefineFolder::Binary) + _T("Default_BYPASS.dll"); // 불러올 Define Dll
        ::CopyFile(strByPassDefaultRecipeDllPathName, m_strDefaultRecipePathNameForByPass,
            TRUE); // dll을 vmjob으로 변경하고 Temp folder로 옮긴다
    }
}

void VisionMainTR::TerminateDLSW()
{
    bThreadExitCheckDLSW = TRUE;
    HWND hWnd = ::FindWindow(NULL, _T("iEasyAI"));
    if (hWnd)
    {
        DWORD prID;
        GetWindowThreadProcessId(hWnd, &prID);
        HANDLE hpr = OpenProcess(PROCESS_ALL_ACCESS, FALSE, prID);
        if (hpr)
        {
            TerminateProcess(hpr, 0);
            CloseHandle(hpr);
        }
    }
}

void VisionMainTR::RuniDL()
{
    if (m_systemConfig.m_bStateiDL == TRUE)
    {
        AfxMessageBox(_T("iDL has already started!!"));
        return;
    }

    //if (m_systemConfig.m_bUseAiInspection == FALSE)
    //	m_systemConfig.m_bUseAiInspection = TRUE;

    // Update Result 관련 Shared memory 초기화
    m_handleUpdateDevResult = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, UPDATE_DEVRES_SHARE_MEMNAME);

    // 딥러닝 검사 후 Update될 DeviceResult 데이터 공유 메모리
    if (m_handleUpdateDevResult == NULL)
        m_handleUpdateDevResult = ::CreateFileMapping(
            INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, MAX_SEND_RESULT_SIZE, UPDATE_DEVRES_SHARE_MEMNAME);

    if (m_handleUpdateDevResult != NULL)
    {
        m_pbySharedUpdateDevResult
            = (BYTE*)::MapViewOfFile(m_handleUpdateDevResult, FILE_MAP_ALL_ACCESS, 0, 0, MAX_SEND_RESULT_SIZE);
    }

    // Update Result Flag
    m_handleChkUpdateResult = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, CHK_INSP_RESULT_UPDATE_SHAREDMEM);

    if (m_handleChkUpdateResult == NULL)
        m_handleChkUpdateResult
            = ::CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 1, CHK_INSP_RESULT_UPDATE_SHAREDMEM);

    if (m_handleChkUpdateResult != NULL)
    {
        m_pbChkUpdateResult = (BOOL*)::MapViewOfFile(m_handleChkUpdateResult, FILE_MAP_ALL_ACCESS, 0, 0, 1);
        *m_pbChkUpdateResult = FALSE;
    }

    if (m_pbySendResultBuffer != NULL)
        delete[] m_pbySendResultBuffer;

    m_pbySendResultBuffer = new BYTE[MAX_SEND_RESULT_SIZE];

    m_Control_SendResult.m_Signal_KillThread = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_Control_SendResult.m_Flag_ThreadRun = CreateEvent(NULL, TRUE, FALSE, NULL);

    ::AfxBeginThread(ThreadSendResultFunc, (void*)this, THREAD_PRIORITY_NORMAL);

    ::ResetEvent(m_Control_SendResult.m_Flag_ThreadRun);
    ::ResetEvent(m_Control_SendResult.m_Signal_KillThread);

    // iDL.exe 실행!!
    bThreadExitCheckDLSW = FALSE;
    ::AfxBeginThread(ThreadExecuteDLSW, (void*)this, THREAD_PRIORITY_NORMAL);

    m_Control_SendConfirm.m_Signal_SendResult = CreateEvent(NULL, TRUE, FALSE, NULL);
    ::AfxBeginThread(ThreadReconnectDLSW, (void*)this, THREAD_PRIORITY_NORMAL);

    return;
}

long VisionMainTR::GetThreadNum()
{
    return m_nThreadNum;
}

VisionMainTR::~VisionMainTR(void)
{
    delete m_menuAccessUI;
    delete m_inline_grab;

    delete m_pSlitBeam3DCommon;

    for (long index = 0; index < m_nThreadNum + 1; index++)
    {
        delete m_unitBuffers[index];
    }

    delete[] m_unitBuffers;
    delete[] m_pVisionInlineUI;

    delete m_pVisionPrimaryUI;

    if (m_systemConfig.m_bUseAiInspection)
    {
        if (!m_pbySharedUpdateDevResult != NULL)
            ::UnmapViewOfFile(m_pbySharedUpdateDevResult);
        if (!m_pbChkUpdateResult != NULL)
            ::UnmapViewOfFile(m_pbChkUpdateResult);

        if (m_handleUpdateDevResult != NULL)
            ::CloseHandle(m_handleUpdateDevResult);
        if (m_handleChkUpdateResult != NULL)
            ::CloseHandle(m_handleChkUpdateResult);

        delete[] m_pbySendResultBuffer;

        ::ResetEvent(m_Control_SendResult.m_Flag_ThreadRun);
        ::SetEvent(m_Control_SendResult.m_Signal_KillThread);

        ::CloseHandle(m_Control_SendResult.m_Signal_KillThread);
        ::CloseHandle(m_Control_SendResult.m_Flag_ThreadRun);

        // 09.10.20 KSY ipack종료 시 iEasyAI 딥러닝 검사 소프트웨어도 함께 종료
        TerminateDLSW();
    }

    ::SetEvent(m_Control_SendConfirm.m_Signal_SendResult);
    ::CloseHandle(m_Control_SendConfirm.m_Signal_SendResult);
}

LPCTSTR VisionMainTR::GetLastLinkDataBaseErrorMessage() const
{
    return m_lastLinkDataBaseErrorMessage;
}

BOOL VisionMainTR::GetLastLinkDataBaseJobVersionChanged() const
{
    return m_lastLinkDataBaseJobVersionChanged;
}

bool VisionMainTR::isInline() const
{
    return IsShowVisionInlineUI();
}

void VisionMainTR::SetInlineStart(bool isBatchInspection)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (isBatchInspection)
    {
        m_inline_grab->Start(true);
    }
    else
    {
        if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP)
        {
            FrameGrabber::GetInstance().OpenCameraComm();
        }

        if (m_pVisionPrimaryUI->m_visionUnit.m_currentProcessingModule == nullptr)
        {
            if (IsInlineMode() == false)
            {
                for (long nThread = 0; nThread < m_nThreadNum; nThread++)
                {
                    m_pVisionInlineUI[nThread].ShowWindow(SW_SHOW);
                }

                HideVisionPrimaryUI();

                m_inline_grab->Start(false);

                SetInlineStartFlag(true);
            }
        }
    }
}

void VisionMainTR::SetInlineStop(bool isBatchInspection)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (!isBatchInspection)
    {
        for (long nThread = 0; nThread < m_nThreadNum; nThread++)
        {
            m_pVisionInlineUI[nThread].ShowWindow(SW_HIDE);
        }
    }

    m_inline_grab->Stop();

    SetInlineStartFlag(false);

    static const long snRepeatSetCount = SystemConfig::GetInstance().IsVisionTypeSide() == TRUE ? 2 : 1;

    if (!isBatchInspection)
    {
        if (m_inline_grab->CopyLastGrabBufferTo(GetPrimaryVisionUnit().getImageLot()))
        {
            for (long nidx = 0; nidx < snRepeatSetCount; nidx++)
                GetPrimaryVisionUnit().getImageLotInsp().Set(
                    GetPrimaryVisionUnit().getImageLot(), enSideVisionModule(nidx));
        }

        ShowVisionPrimaryUI(NULL);
        m_pVisionPrimaryUI->ShowCurrentImage(true);
    }
}

void VisionMainTR::ShowVisionInspSpec()
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    auto& primaryUnit = GetPrimaryVisionUnit();

    // 각 검사항목들의 Spec 을 취합하여 UI 로 표시, Spec 편집 가능
    CDlgVisionInspSpec VisionInspSpecDlg(&primaryUnit);
    VisionInspSpecDlg.DoModal();

    primaryUnit.UpdateInspItem(VisionInspSpecDlg.m_availableFixedItems, enPackageType::UNKNOWN, FALSE);

    SynchronizeJobFile();
}

void VisionMainTR::ShowOverlayManager()
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    // 각 검사항목들의 Spec 을 취합하여 UI 로 표시, Spec 편집 가능
    DlgOverlayManager(GetPrimaryVisionUnit()).DoModal();
}

BOOL VisionMainTR::DoInspection()
{
    auto& primaryUnit = GetPrimaryVisionUnit();

    if (m_menuAccessUI)
    {
        return m_menuAccessUI->callInspection();
    }
    else if (primaryUnit.m_currentProcessingModule)
    {
        primaryUnit.RunInspectionCurrentProcessingModule();
    }
    else
    {
        m_pVisionPrimaryUI->OnInspect();
    }

    return TRUE;
}

void VisionMainTR::ShowIlluminationTeachUI(HWND hwndParent)
{
    delete m_menuAccessUI;

    auto& primaryUnit = GetPrimaryVisionUnit();
    m_menuAccessUI
        = new VisionIlluminationSetup(*this, primaryUnit, hwndParent, enSideVisionModule(GetSideVisionSection()));
}

void VisionMainTR::ShowVisionPrimaryUI(HWND hwndParent)
{
    m_pVisionPrimaryUI->ShowVisionPrimaryDlg(hwndParent);
}

void VisionMainTR::HideVisionPrimaryUI()
{
    m_pVisionPrimaryUI->HideVisionPrimaryDlg();
}

void VisionMainTR::ShowBatchInspectionUI(HWND hwndParent)
{
    delete m_menuAccessUI;

    auto& primaryUnit = GetPrimaryVisionUnit();
    m_menuAccessUI = new VisionBatchInspectionUI(*this, primaryUnit, hwndParent);
}

void VisionMainTR::SetFileVersion(CString strFileVersion)
{
    m_strDetailFileVersion = strFileVersion;
}

CString VisionMainTR::GetFileVersion()
{
    return m_strDetailFileVersion;
}

// 영훈 20150512_DeleteMarkImages : Inline Start시 Job에 저장된 Mark 이미지를 모두 지우고 시작하도록 한다.
void VisionMainTR::DeleteMarkMultiImages()
{
    /*for (long nThread = 0; nThread <= m_nThreadNum; nThread++)
    {
        VisionUnit& visionUnit = *m_unitBuffers[nThread];
        long nSize = (long)visionUnit.m_visionInspectionMarks.size();
        for (long n = 0; n < nSize; n++)
        {
            VisionInspectionMark* pMarkInspection = visionUnit.m_visionInspectionMarks[n];

            // 0번은 그냥 놔둔다.
            for (long nMultiMarkID = 1; nMultiMarkID < NUM_OF_MARKMULTI; nMultiMarkID++)
            {
                pMarkInspection->m_VisionSpec[nMultiMarkID]->m_specImage.Free();
                pMarkInspection->m_VisionSpec[nMultiMarkID]->m_bSaveTeachImage = FALSE;
                pMarkInspection->m_VisionSpec[nMultiMarkID]->m_bTeachDone = FALSE;
            }
        }
    }
    */
}

void VisionMainTR::CloseAllUI()
{
    // 묻지도 따지지도 않고, 모두 닫는다.
    CloseAccessUI();
    CloseProcessingTeachingUI();
}

void VisionMainTR::ReturnToPrimaryUI(BOOL bCallLinkDatabase)
{
    CloseAllUI();

    SynchronizeJobFile(bCallLinkDatabase);

    //{{ Set Camera Gain Default
    FrameGrabber::GetInstance().SetDigitalGainValue(SystemConfig::GetInstance().m_fDefault_Camera_Gain, FALSE);
    //}

    // PrimaryUI로 돌아갈 때도, Side Vision일 경우 Section 확인을 하여 UI Update 해줘야함 - JHB_2024.04.15
    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP)
    {
        m_pVisionPrimaryUI->SetPrimaryUISideVisionSection();
    }

    ShowVisionPrimaryUI(NULL);
}

void VisionMainTR::CloseProcessingTeachingUI(BOOL bSendToHostJobMode)
{
    VisionUnit& primaryUnit = GetPrimaryVisionUnit();
    /*long nSize = (long)primaryUnit.m_visionInspectionMarks.size();

    const auto& scale = primaryUnit.getScale();

    // Host에 Mark 이미지 전송
    if (nSize)
    {
        CFileFind ff;
        VisionAlignResult sEdgeAlignResult;
        long nDataNum = 0;
        void* pData = primaryUnit.GetVisionDebugInfo(_T(""), _T("EDGE Align Result"), nDataNum);
        if (pData != nullptr && nDataNum > 0)
            sEdgeAlignResult = *(VisionAlignResult*)pData;

        // 영훈 20140807_MarkTeachingImage : Mark Teaching Image를 c:\intekplus\Mark image에 저장하고 호스트로 전송하도록 한다.
        // 영훈 20150324_Mark_Image_Path : Mark Image를 전송하고 Vision에는 별도로 날짜별 Job별 Lot별로 남기도록 한다.
        VisionReusableMemory& VisionreuseableMemory = *primaryUnit.m_VisionReusableMemory;
        std::vector<CString> vecstrFilePath(2);
        CString strTemp;
        CString strFilePath;
        CString strVisionSavePath;

        // 최종 폴더 생성
        strFilePath = DynamicSystemPath::get(DefineFolder::MarkImage) + _T("Mark Image\\");
        ::CreateDirectory(strFilePath, NULL);

        // 날짜 폴더 생성
        strTemp = CTime::GetCurrentTime().Format("%Y.%m.%d\\"); // 오늘 날짜를 알아온다.;
        strFilePath += strTemp + _T("\\");
        ::CreateDirectory(strFilePath, NULL);

        // Job name 폴더 생성
        strFilePath += m_systemConfig.m_strJobFileName + _T("\\");
        ::CreateDirectory(strFilePath, NULL);

        // Lot Name 폴더 생성
        if (m_lotID == _T(""))
            strTemp.Format(_T("Manual\\")); // Host에서 입력한 LotID를 받아온다.
        else
            strTemp.Format(_T("%s\\"), (LPCTSTR)m_lotID); // Host에서 입력한 LotID를 받아온다.
        strFilePath += strTemp;
        ::CreateDirectory(strFilePath, NULL);

        float fAngle_rad = sEdgeAlignResult.m_angle_rad;
        auto center = sEdgeAlignResult.m_center;

        long nImageSizeX = VisionreuseableMemory.GetInspImageSizeX();
        long nImageSizeY = VisionreuseableMemory.GetInspImageSizeY();
        Ipvm::Point32r2 imageCenter(nImageSizeX * 0.5f, nImageSizeY * 0.5f);

        Ipvm::Image8u imageRotate;
        Ipvm::Image8u imageThreshold;
        Ipvm::Image8u imageTeachResult;

        VisionreuseableMemory.GetInspByteImage(imageRotate);
        VisionreuseableMemory.GetInspByteImage(imageThreshold);
        VisionreuseableMemory.GetInspByteImage(imageTeachResult);

        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageRotate), 0, imageRotate);
        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageThreshold), 0, imageThreshold);
        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageTeachResult), 0, imageTeachResult);

        auto* markInsp = primaryUnit.m_visionInspectionMarks[0];
        Ipvm::Rect32s rtTotalTeachROI
            = scale.convert_BCUToPixel(markInsp->m_VisionSpec[0]->m_rtMarkTeachROI_BCU, imageCenter);

        std::vector<Ipvm::Rect32s> vecrtCharROI(0);
        Ipvm::Rect32s rtCharROI;
        Ipvm::Rect32s rtTeachResultViewROI(nImageSizeX, nImageSizeY, 0, 0);
        long nCharNum;
        //}}

        for (long n = 0; n < nSize; n++)
        {
            VisionInspectionMark* pMarkInspection = primaryUnit.m_visionInspectionMarks[n];

            for (long nMultiMarkID = 0; nMultiMarkID < NUM_OF_MARKMULTI; nMultiMarkID++)
            {
                if (pMarkInspection->m_VisionSpec[nMultiMarkID]->m_bSaveTeachImage)
                {
                    Ipvm::Rect32s rtThreshROI = scale.convert_BCUToPixel(
                        pMarkInspection->m_VisionSpec[nMultiMarkID]->m_rtMarkTeachROI_BCU, imageCenter);
                    rtTotalTeachROI.UnionRect(rtTotalTeachROI, rtThreshROI);
                }
            }

            //{{ //kircheis_201611XX //TeachResultView
            for (long nMultiMarkID = 0; nMultiMarkID < NUM_OF_MARKMULTI; nMultiMarkID++)
            {
                if (pMarkInspection->m_VisionSpec[nMultiMarkID]->m_bTeachDone)
                {
                    Ipvm::Image8u imageSpec = pMarkInspection->m_VisionSpec[nMultiMarkID]->m_specImage;

                    Ipvm::ImageProcessing::BitwiseAnd(imageSpec, Ipvm::Rect32s(imageSpec), imageTeachResult);

                    nCharNum = pMarkInspection->m_VisionSpec[nMultiMarkID]->m_nCharNum;

                    for (long nChar = 0; nChar < nCharNum; nChar++)
                    {
                        rtCharROI = Ipvm::Rect32s(
                            pMarkInspection->m_VisionSpec[nMultiMarkID]->m_plTeachCharROI[nChar] + imageCenter);
                        vecrtCharROI.push_back(rtCharROI);
                        rtTeachResultViewROI.m_left = (long)min(rtTeachResultViewROI.m_left, rtCharROI.m_left);
                        rtTeachResultViewROI.m_top = (long)min(rtTeachResultViewROI.m_top, rtCharROI.m_top);
                        rtTeachResultViewROI.m_right = (long)max(rtTeachResultViewROI.m_right, rtCharROI.m_right);
                        rtTeachResultViewROI.m_bottom = (long)max(rtTeachResultViewROI.m_bottom, rtCharROI.m_bottom);
                    }
                    break;
                }
            }
            //}}
        }

        //{{//kircheis_201611XX //TeachResultView
        if (bSendToHostJobMode && m_systemConfig.m_bUseMarkTeachResultView)
        {
            nCharNum = (long)vecrtCharROI.size();
            long nWidthOffset = (long)((float)rtTeachResultViewROI.Width() * 0.1f + 0.5f);
            long nHeightOffset = (long)((float)rtTeachResultViewROI.Height() * 0.1f + 0.5f);
            rtTeachResultViewROI.InflateRect(nWidthOffset, nHeightOffset);
            m_pVisionPrimaryUI->SetMarkTeachTotalResultView(
                imageTeachResult, rtTeachResultViewROI, nCharNum, &vecrtCharROI[0]);
        }
        //}}

        BOOL bCreateImageFile = FALSE;

        for (long n = 0; n < nSize; n++)
        {
            VisionInspectionMark* pMarkInspection = primaryUnit.m_visionInspectionMarks[n];

            // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
            Ipvm::Image8u imageMark = pMarkInspection->GetInspectionFrameImage();
            if (imageMark.GetMem() == nullptr)
            {
                primaryUnit
                    .CloseProcessingTeachingUI(); //SDY 이미지가 없는 상태에서 Mark가 포함된 Job을 수정하면 Job Teaching UI가 닫히지 않는 문제 수정
                return;
            }

            Ipvm::ImageProcessing::RotateLinearInterpolation(imageMark, rtTotalTeachROI, center, -fAngle_rad * ITP_RAD_TO_DEG,
                Ipvm::Point32r2(0.f, 0.f), imageRotate);

            // Multi Teach 폴더 생성
            strVisionSavePath = strFilePath + pMarkInspection->m_strModuleName + _T("\\");
            ::CreateDirectory(strVisionSavePath, NULL);

            for (long nMultiMarkID = 0; nMultiMarkID < NUM_OF_MARKMULTI; nMultiMarkID++)
            {
                if (pMarkInspection->m_VisionSpec[nMultiMarkID]->m_bSaveTeachImage)
                {
                    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageThreshold), 0, imageThreshold);

                    // Multi Teach 폴더 생성
                    strTemp.Format(_T("%sMulti_Teach_%d\\"), (LPCTSTR)strVisionSavePath, nMultiMarkID);
                    ::CreateDirectory(strTemp, NULL);

                    // 이미지 생성
                    vecstrFilePath[0].Format(_T("%sMarkImage_Gray.bmp"), (LPCTSTR)strTemp);
                    vecstrFilePath[1].Format(_T("%sMarkImage_Thresh.bmp"), (LPCTSTR)strTemp);

                    // 영훈 [DebugImageSave] 20130808 : 중복되는 파일이 있으면 뒤에 숫자로 카운팅한다.
                    long nFildChecker = 0;
                    while (1)
                    {
                        if (ff.FindFile(vecstrFilePath[0]))
                        {
                            vecstrFilePath[0].Format(_T("%s%d_MarkImage_Gray.bmp"), (LPCTSTR)strTemp, nFildChecker);
                            vecstrFilePath[1].Format(_T("%s%d_MarkImage_Thresh.bmp"), (LPCTSTR)strTemp, nFildChecker);

                            if (nFildChecker > 100)
                            {
                                break;
                            }
                            nFildChecker++;
                        }
                        else
                        {
                            break;
                        }
                    }

                    BYTE threshValue
                        = (BYTE)min(255, max(0, pMarkInspection->m_VisionSpec[nMultiMarkID]->m_nMarkThreshold));
                    Ipvm::Rect32s rtThreshROI = scale.convert_BCUToPixel(
                        pMarkInspection->m_VisionSpec[nMultiMarkID]->m_rtMarkTeachROI_BCU, imageCenter);

                    Ipvm::ImageProcessing::BinarizeGreaterEqual(imageRotate, rtThreshROI, threshValue, imageThreshold);

                    Ipvm::Image8u(imageRotate, rtTotalTeachROI).SaveBmp(vecstrFilePath[0]);
                    Ipvm::Image8u(imageThreshold, rtTotalTeachROI).SaveBmp(vecstrFilePath[1]);

                    bCreateImageFile = TRUE;
                }
            }
        }

        if (m_pMessageSocket->IsConnected())
        {
            // Multi Teach 폴더 생성
            strVisionSavePath = strFilePath + _T("Send_Host_Image\\");
            ::CreateDirectory(strVisionSavePath, NULL);

            Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageRotate), 0, imageRotate);
            Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageThreshold), 0, imageThreshold);

            Ipvm::Image8u imageMarkSpec;
            VisionreuseableMemory.GetInspByteImage(imageMarkSpec);

            Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageMarkSpec), 0, imageMarkSpec);

            // 영훈 20150409: image가 정상 저장되었는지 확인하는 변수
            BOOL bMakeSuccessImage = FALSE;
            for (long n = 0; n < nSize; n++)
            {
                VisionInspectionMark* pMarkInspection = primaryUnit.m_visionInspectionMarks[n];

                Ipvm::Image8u imageSpec = pMarkInspection->m_VisionSpec[0]->m_specImage;

                Ipvm::ImageProcessing::BitwiseAnd(imageSpec, Ipvm::Rect32s(imageSpec), imageMarkSpec);

                // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
                Ipvm::Image8u imageMark = pMarkInspection->GetInspectionFrameImage();
                if (imageMark.GetMem() == nullptr)
                {
                    primaryUnit
                        .CloseProcessingTeachingUI(); //SDY 이미지가 없는 상태에서 Mark가 포함된 Job을 수정하면 Job Teaching UI가 닫히지 않는 문제 수정
                    return;
                }

                Ipvm::ImageProcessing::RotateLinearInterpolation(imageMark, rtTotalTeachROI, center, -fAngle_rad * ITP_RAD_TO_DEG,
                    Ipvm::Point32r2(0.f, 0.f), imageRotate);

                if (pMarkInspection->m_VisionSpec[0]->m_bSaveTeachImage)
                {
                    // 이미지 생성
                    vecstrFilePath[0] = strVisionSavePath + _T("MarkImage_Gray.bmp");
                    vecstrFilePath[1] = strVisionSavePath + _T("MarkImage_Thresh.bmp");

                    BYTE threshValue = (BYTE)min(255, max(0, pMarkInspection->m_VisionSpec[0]->m_nMarkThreshold));
                    Ipvm::Rect32s rtThreshROI
                        = scale.convert_BCUToPixel(pMarkInspection->m_VisionSpec[0]->m_rtMarkTeachROI_BCU, imageCenter);

                    Ipvm::ImageProcessing::BinarizeGreaterEqual(imageRotate, rtThreshROI, threshValue, imageThreshold);

                    // 영훈 20150409 : image가 저장됐으면 호스트에 보낼 준비를 한다.
                    bMakeSuccessImage = TRUE;
                }

                // 영훈 : 저장이 끝났으면 다음 티칭을 할 때까진 저장하지 않도록 한다.
                for (long nMultiMarkID = 0; nMultiMarkID < NUM_OF_MARKMULTI; nMultiMarkID++)
                {
                    if (pMarkInspection->m_VisionSpec[nMultiMarkID]->m_bSaveTeachImage)
                    {
                        pMarkInspection->m_VisionSpec[nMultiMarkID]->m_bSaveTeachImage = FALSE;
                    }
                }
            }

            // 영훈 20150409 : Image가 정상 저장 됐으면 호스트로 보낸다.
            if (bMakeSuccessImage)
            {
                Ipvm::Image8u(imageRotate, rtTotalTeachROI).SaveBmp(vecstrFilePath[0]);

                // 원본 이미지 복사
                Ipvm::ImageProcessing::Copy(imageMarkSpec, rtTotalTeachROI, imageThreshold);
                Ipvm::Image8u(imageThreshold, rtTotalTeachROI).SaveBmp(vecstrFilePath[1]);

                SendToHost_MarkImage(vecstrFilePath);
            }
        }

        for (int nIdx = 0; nIdx < vecstrFilePath.size(); nIdx++)
        {
            vecstrFilePath[nIdx].Empty();
        }

        strTemp.Empty();
        strFilePath.Empty();
        strVisionSavePath.Empty();
    }*/

    //////////////////////////////
    primaryUnit.CloseProcessingTeachingUI();
}

bool VisionMainTR::IsShowProcessingTeachingUI()
{
    return GetPrimaryVisionUnit().IsWindowVisible();
}

void VisionMainTR::InitializeVisionInlineUI(HWND hwndParent)
{
    if (m_pVisionInlineUI)
        return;

    auto& inspectionOrderer = m_inline_grab->getInspectionOrderer();
    m_pVisionInlineUI = new CVisionInlineUI[m_nThreadNum];

    for (long nThread = 0; nThread < m_nThreadNum; nThread++)
    {
        inspectionOrderer.setInlineUI(nThread, m_pVisionInlineUI[nThread], hwndParent);
    }
}

bool VisionMainTR::IsShowVisionInlineUI() const
{
    if (m_pVisionInlineUI == nullptr || m_nThreadNum == 0)
        return false;
    return m_pVisionInlineUI[0].IsWindowVisible() == TRUE;
}

void VisionMainTR::SetColorImageToInlineUIforNGRV(Ipvm::Image8u3 image) //kircheis_NGRVINLINE
{
    if (m_pVisionInlineUI == NULL)
        return;

    m_pVisionInlineUI[0].SetColorImageForNGRV(image);
}

void VisionMainTR::SendToHostMarkTeachImage() //kircheis_AutoTeach
{
    if (m_pMessageSocket->IsConnected() == false)
        return;

    VisionUnit& primaryUnit = GetPrimaryVisionUnit();
    const auto& scale = primaryUnit.getScale();
    /*long nSize = (long)primaryUnit.m_visionInspectionMarks.size();

    // Host에 Mark 이미지 전송
    if (nSize > 0)
    {
        CFileFind ff;
        VisionAlignResult sEdgeAlignResult;
        long nDataNum = 0;
        void* pData = primaryUnit.GetVisionDebugInfo(_T(""), _T("EDGE Align Result"), nDataNum);
        if (pData != nullptr && nDataNum > 0)
            sEdgeAlignResult = *(VisionAlignResult*)pData;

        // 영훈 20140807_MarkTeachingImage : Mark Teaching Image를 c:\intekplus\Mark image에 저장하고 호스트로 전송하도록 한다.
        // 영훈 20150324_Mark_Image_Path : Mark Image를 전송하고 Vision에는 별도로 날짜별 Job별 Lot별로 남기도록 한다.
        VisionReusableMemory& VisionreuseableMemory = *primaryUnit.m_VisionReusableMemory;
        std::vector<CString> vecstrFilePath(2);
        CString strTemp;
        CString strFilePath;
        CString strVisionSavePath;

        // 최종 폴더 생성
        strFilePath = DynamicSystemPath::get(DefineFolder::MarkImage) + _T("Mark Image\\");
        ::CreateDirectory(strFilePath, NULL);

        // 날짜 폴더 생성
        strTemp = CTime::GetCurrentTime().Format("%Y.%m.%d\\"); // 오늘 날짜를 알아온다.;
        strFilePath += strTemp + _T("\\");
        ::CreateDirectory(strFilePath, NULL);

        // Job name 폴더 생성
        strFilePath += m_systemConfig.m_strJobFileName + _T("\\");
        ::CreateDirectory(strFilePath, NULL);

        // Lot Name 폴더 생성
        if (m_lotID == _T(""))
            strTemp.Format(_T("Manual\\")); // Host에서 입력한 LotID를 받아온다.
        else
            strTemp.Format(_T("%s\\"), (LPCTSTR)m_lotID); // Host에서 입력한 LotID를 받아온다.
        strFilePath += strTemp;
        ::CreateDirectory(strFilePath, NULL);

        float fAngle_rad = sEdgeAlignResult.m_angle_rad;
        auto center = sEdgeAlignResult.m_center;

        long nImageSizeX = VisionreuseableMemory.GetInspImageSizeX();
        long nImageSizeY = VisionreuseableMemory.GetInspImageSizeY();
        Ipvm::Point32r2 imageCenter(nImageSizeX * 0.5f, nImageSizeY * 0.5f);

        Ipvm::Image8u imageRotate;
        Ipvm::Image8u imageThreshold;
        Ipvm::Image8u imageMarkSpec;

        VisionreuseableMemory.GetInspByteImage(imageRotate);
        VisionreuseableMemory.GetInspByteImage(imageThreshold);
        VisionreuseableMemory.GetInspByteImage(imageMarkSpec);

        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageRotate), 0, imageRotate);
        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageThreshold), 0, imageThreshold);
        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageMarkSpec), 0, imageMarkSpec);

        auto* markInsp = primaryUnit.m_visionInspectionMarks[0];
        Ipvm::Rect32s rtTotalTeachROI
            = scale.convert_BCUToPixel(markInsp->m_VisionSpec[0]->m_rtMarkTeachROI_BCU, imageCenter);

        for (long n = 0; n < nSize; n++)
        {
            VisionInspectionMark* pMarkInspection = primaryUnit.m_visionInspectionMarks[n];

            for (long nMultiMarkID = 0; nMultiMarkID < NUM_OF_MARKMULTI; nMultiMarkID++)
            {
                if (pMarkInspection->m_VisionSpec[nMultiMarkID]->m_bSaveTeachImage)
                {
                    Ipvm::Rect32s rtThreshROI = scale.convert_BCUToPixel(
                        pMarkInspection->m_VisionSpec[nMultiMarkID]->m_rtMarkTeachROI_BCU, imageCenter);
                    rtTotalTeachROI.UnionRect(rtTotalTeachROI, rtThreshROI);
                }
            }
        }

        BOOL bCreateImageFile = FALSE;

        for (long n = 0; n < nSize; n++)
        {
            VisionInspectionMark* pMarkInspection = primaryUnit.m_visionInspectionMarks[n];

            // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
            Ipvm::Image8u imageMark = pMarkInspection->GetInspectionFrameImage();
            if (imageMark.GetMem() == nullptr)
            {
                primaryUnit
                    .CloseProcessingTeachingUI(); //SDY 이미지가 없는 상태에서 Mark가 포함된 Job을 수정하면 Job Teaching UI가 닫히지 않는 문제 수정
                return;
            }

            Ipvm::ImageProcessing::RotateLinearInterpolation(imageMark, rtTotalTeachROI, center, -fAngle_rad * ITP_RAD_TO_DEG,
                Ipvm::Point32r2(0.f, 0.f), imageRotate);

            // Multi Teach 폴더 생성
            strVisionSavePath = strFilePath + pMarkInspection->m_strModuleName + _T("\\");
            ::CreateDirectory(strVisionSavePath, NULL);

            for (long nMultiMarkID = 0; nMultiMarkID < NUM_OF_MARKMULTI; nMultiMarkID++)
            {
                if (pMarkInspection->m_VisionSpec[nMultiMarkID]->m_bSaveTeachImage)
                {
                    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageThreshold), 0, imageThreshold);

                    // Multi Teach 폴더 생성
                    strTemp.Format(_T("%sMulti_Teach_%d\\"), (LPCTSTR)strVisionSavePath, nMultiMarkID);
                    ::CreateDirectory(strTemp, NULL);

                    // 이미지 생성
                    vecstrFilePath[0].Format(_T("%sMarkImage_Gray.bmp"), (LPCTSTR)strTemp);
                    vecstrFilePath[1].Format(_T("%sMarkImage_Thresh.bmp"), (LPCTSTR)strTemp);

                    // 영훈 [DebugImageSave] 20130808 : 중복되는 파일이 있으면 뒤에 숫자로 카운팅한다.
                    long nFildChecker = 0;
                    while (1)
                    {
                        if (ff.FindFile(vecstrFilePath[0]))
                        {
                            vecstrFilePath[0].Format(_T("%s%d_MarkImage_Gray.bmp"), (LPCTSTR)strTemp, nFildChecker);
                            vecstrFilePath[1].Format(_T("%s%d_MarkImage_Thresh.bmp"), (LPCTSTR)strTemp, nFildChecker);

                            if (nFildChecker > 100)
                            {
                                break;
                            }
                            nFildChecker++;
                        }
                        else
                        {
                            break;
                        }
                    }

                    BYTE threshValue
                        = (BYTE)min(255, max(0, pMarkInspection->m_VisionSpec[nMultiMarkID]->m_nMarkThreshold));
                    Ipvm::Rect32s rtThreshROI = scale.convert_BCUToPixel(
                        pMarkInspection->m_VisionSpec[nMultiMarkID]->m_rtMarkTeachROI_BCU, imageCenter);

                    Ipvm::ImageProcessing::BinarizeGreaterEqual(imageRotate, rtThreshROI, threshValue, imageThreshold);

                    Ipvm::Image8u(imageRotate, rtTotalTeachROI).SaveBmp(vecstrFilePath[0]);
                    Ipvm::Image8u(imageThreshold, rtTotalTeachROI).SaveBmp(vecstrFilePath[1]);

                    bCreateImageFile = TRUE;
                }
            }
        }

        if (m_pMessageSocket->IsConnected())
        {
            // Multi Teach 폴더 생성
            strVisionSavePath = strFilePath + _T("Send_Host_Image\\");
            ::CreateDirectory(strVisionSavePath, NULL);

            Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageRotate), 0, imageRotate);
            Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageThreshold), 0, imageThreshold);

            // 영훈 20150409: image가 정상 저장되었는지 확인하는 변수
            BOOL bMakeSuccessImage = FALSE;
            for (long n = 0; n < nSize; n++)
            {
                VisionInspectionMark* pMarkInspection = primaryUnit.m_visionInspectionMarks[n];

                Ipvm::Image8u imageSpec = pMarkInspection->m_VisionSpec[0]->m_specImage;

                Ipvm::ImageProcessing::BitwiseAnd(imageSpec, Ipvm::Rect32s(imageSpec), imageMarkSpec);

                // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
                Ipvm::Image8u imageMark = pMarkInspection->GetInspectionFrameImage();
                if (imageMark.GetMem() == nullptr)
                {
                    primaryUnit
                        .CloseProcessingTeachingUI(); //SDY 이미지가 없는 상태에서 Mark가 포함된 Job을 수정하면 Job Teaching UI가 닫히지 않는 문제 수정
                    return;
                }

                Ipvm::ImageProcessing::RotateLinearInterpolation(imageMark, rtTotalTeachROI, center, -fAngle_rad * ITP_RAD_TO_DEG,
                    Ipvm::Point32r2(0.f, 0.f), imageRotate);

                if (pMarkInspection->m_VisionSpec[0]->m_bSaveTeachImage)
                {
                    // 이미지 생성
                    vecstrFilePath[0] = strVisionSavePath + _T("MarkImage_Gray.bmp");
                    vecstrFilePath[1] = strVisionSavePath + _T("MarkImage_Thresh.bmp");

                    BYTE threshValue = (BYTE)min(255, max(0, pMarkInspection->m_VisionSpec[0]->m_nMarkThreshold));
                    Ipvm::Rect32s rtThreshROI
                        = scale.convert_BCUToPixel(pMarkInspection->m_VisionSpec[0]->m_rtMarkTeachROI_BCU, imageCenter);

                    Ipvm::ImageProcessing::BinarizeGreaterEqual(imageRotate, rtThreshROI, threshValue, imageThreshold);

                    // 영훈 20150409 : image가 저장됐으면 호스트에 보낼 준비를 한다.
                    bMakeSuccessImage = TRUE;
                }

                // 영훈 : 저장이 끝났으면 다음 티칭을 할 때까진 저장하지 않도록 한다.
                for (long nMultiMarkID = 0; nMultiMarkID < NUM_OF_MARKMULTI; nMultiMarkID++)
                {
                    if (pMarkInspection->m_VisionSpec[nMultiMarkID]->m_bSaveTeachImage)
                    {
                        pMarkInspection->m_VisionSpec[nMultiMarkID]->m_bSaveTeachImage = FALSE;
                    }
                }
            }

            // 영훈 20150409 : Image가 정상 저장 됐으면 호스트로 보낸다.
            if (bMakeSuccessImage)
            {
                Ipvm::Image8u(imageRotate, rtTotalTeachROI).SaveBmp(vecstrFilePath[0]);

                // 원본 이미지 복사
                Ipvm::ImageProcessing::Copy(imageMarkSpec, rtTotalTeachROI, imageThreshold);
                Ipvm::Image8u(imageThreshold, rtTotalTeachROI).SaveBmp(vecstrFilePath[1]);

                SendToHost_MarkImage(vecstrFilePath);
            }
        }
        for (int nIdx = 0; nIdx < vecstrFilePath.size(); nIdx++)
        {
            vecstrFilePath[nIdx].Empty();
        }

        strTemp.Empty();
        strFilePath.Empty();
        strVisionSavePath.Empty();
    }*/
}

void VisionMainTR::CloseAccessUI()
{
    if (m_menuAccessUI)
    {
        m_menuAccessUI->callQuit();

        delete m_menuAccessUI;
        m_menuAccessUI = nullptr;
    }
}

void VisionMainTR::ShowProcessingTeachingUI(HWND hwndParent)
{
    m_pVisionPrimaryUI->HideMarkTeachTotalResultView(); //kircheis_201611XX //TeachResultView

    GetPrimaryVisionUnit().ShowProcessingTeachingUI(hwndParent);
}

void VisionMainTR::SaveDumpImageFiles(LPCTSTR dumpPath, LPCTSTR fileVersion)
{
    // Thread 에 있는 영상을 모두 저장한다.
    // ThreadNum + 1 은 Primary UI에 있는 영상이다

    for (long index = 0; index < m_nThreadNum + 1; index++)
    {
        CString strBackupImage;
        strBackupImage.Format(_T("%s\\%s_Image%d.bmp"), dumpPath, fileVersion, index + 1);

        // 이미지가 없는 Thread는 저장하지 말자..
        auto& imageLot = m_unitBuffers[index]->getImageLot();
        if (imageLot.GetImageFrameCount() <= 0)
            continue;
        if (imageLot.GetImageSizeX() <= 0 || imageLot.GetImageSizeY() <= 0)
            continue;

        imageLot.Save(strBackupImage);

        strBackupImage.Empty();
    }
}

BOOL VisionMainTR::SaveJobFile()
{
    // BMP File 을 선택할 File Dialog 팝업
    CFileDialog Dlg(FALSE, _T("vmjob"), NULL, /*OFN_NOCHANGEDIR | OFN_FILEMUSTEXIST | */ OFN_HIDEREADONLY,
        _T("Job File (*.vmjob)|*.vmjob|All File(*.*)|*.*||"));

    CString initDir = PersonalConfig::getInstance().getLastAccessDirectory_Job();
    Dlg.GetOFN().lpstrInitialDir = initDir;

    if (Dlg.DoModal() != IDOK)
        return FALSE;

    CString strFilename = Dlg.GetPathName();
    if (Dlg.GetFileExt() == "")
    {
        strFilename += _T(".vmjob");
    }

    // 영훈 20130912 : SaveAs 했을 경우 바로 Job 이름이 바뀌도록 해주자.
    CString strJobName = Dlg.GetFileName();
    strJobName = REMOVE_EXT_FROM_FILENAME(strJobName);
    SetVisionJobName(strFilename, strJobName);

    if (SaveJobFile(strFilename, SaveJobMode::Normal))
    {
        PersonalConfig::getInstance().setLastAccessDirectory_Job(GET_PATHNAME(Dlg.GetPathName()));
        return TRUE;
    }

    initDir.Empty();
    strFilename.Empty();
    strJobName.Empty();

    return FALSE;
}

BOOL VisionMainTR::SaveJobFile(CString strFilename, SaveJobMode mode, BOOL bSendJob, BOOL bSendPackageSpec)
{
    CString strJobImageName;

    if (strFilename.IsEmpty())
    {
        strFilename = m_strJobFilePath;
    }

    long nFindID = 0;
    nFindID = strFilename.Find(_T(".vmjob"));
    if (nFindID < 0)
    {
        nFindID = strFilename.Find(_T("New Job"));

        if (nFindID >= 0)
        {
            strFilename = DynamicSystemPath::get(DefineFolder::Job) + m_strJobFileName;
        }

        strFilename += _T(".vmjob");
    }

    CString strLeft = strFilename.Left(3);
    if (strLeft.Find(_T("\\")) < 0)
    {
        if (SystemConfig::GetInstance().m_nAutomationMode
            == 1) // Automation이 활성화 될 경우 Automation 폴더로 이동한다. 해당 경우에는 name만 전달됨
        {
            strFilename = DynamicSystemPath::get(DefineFolder::Log) + _T("\\AutomationTemp\\") + strFilename;
        }
        else
        {
            strFilename = DynamicSystemPath::get(DefineFolder::Job) + strFilename;
        }
    }

    CiDataBase jobDB;

    if (!LinkDataBase(TRUE, jobDB))
        return FALSE;

    // Job File에 저장한다.
    if (!jobDB.Save(strFilename))
        return FALSE;

    if (mode == SaveJobMode::SaveOnly)
    {
        // Save만을 위해 Job을 저장한다 (용도 : CrashDump or Raw 폴더에 Job저장)
        // UI에 전달해줄 필요 없으므로 여기서 종료한다

        return TRUE;
    }

    DevelopmentLog::AddLog(DevelopmentLog::Type::Event, _T("SaveJobFile : %s"), LPCTSTR(strFilename));

    // 20140226 영훈 : 이제 그냥 Save 버튼은 호스트와 연결이 되어있든 말든 그냥 Local 저장만하고 화면저장은 하지 않는다. Teach End버튼이 호스트로 잡을 전송한다.
    if (bSendJob)
    {
        iPIS_Send_Job(jobDB, MSG_PROBE_TEACH_END);
        iPIS_Send_NGRV_AF_Info(); //kircheis_NGRVAF

        //{{//kircheis_Comp3DHeightBug
        if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP)
        {
            CheckIntegrityOfCompMapData();
        }
        //}}

        if (GetPrimaryVisionUnit().Is2DMatrixReadingBypass() == FALSE)
        {
            //{{ NGRV SingleRun Recipe를 위한 2D Vision Image 정보
            iPIS_Send_2D_Vision_Image_Info_To_NGRV();

            //{{ NGRV SingleRunInfo 정보 전송
            iPIS_Send_NGRV_SingleRunInfo();
        }
    }

    OpenJobFile(strFilename, CString(_T("")));

    /* TR에서는 PackageSpec을 보내지 않고 알아서 TR VisionMain끼리 Spec을 교환한다.
    long nCurLogonMode = SystemConfig::GetInstance().m_nCurrentAccessMode;
    bool bIsExchangedPackageSpec = SystemConfig::GetInstance().m_bIsExchangedPackageSpec;
    if (bSendJob && bSendPackageSpec && nCurLogonMode > _OPERATOR && bIsExchangedPackageSpec)
    {
        Sleep(200);
        iPIS_Send_PackageSpec();
        SystemConfig::GetInstance().m_bIsExchangedPackageSpec = false;
    }
    */

    strJobImageName.Empty();
    strLeft.Empty();

    return TRUE;
}

BOOL VisionMainTR::SynchronizeJobFile(BOOL bCallLinkDatabase)
{
    if (bCallLinkDatabase)
    {
        CiDataBase jobDB;

        if (!LinkDataBase(TRUE, jobDB))
        {
            return FALSE;
        }

        if (!LinkDataBase(FALSE, jobDB))
        {
            return FALSE;
        }
    }

    OnJobChanged();

    return TRUE;
}

BOOL VisionMainTR::OpenJobFile()
{
    // BMP File 을 선택할 File Dialog 팝업
    CFileDialog Dlg(TRUE, _T("vmjob"), NULL, /*OFN_NOCHANGEDIR|OFN_FILEMUSTEXIST|*/ OFN_HIDEREADONLY,
        _T("Job File (*.vmjob)|*.vmjob|All File(*.*)|*.*|"));

    CString initDir = PersonalConfig::getInstance().getLastAccessDirectory_Job();
    Dlg.GetOFN().lpstrInitialDir = initDir;

    if (Dlg.DoModal() != IDOK)
        return FALSE;

    CString strFilePath = Dlg.GetPathName();
    CString strFileName = Dlg.GetFileName();
    strFileName = REMOVE_EXT_FROM_FILENAME(strFileName); //영훈 20130808 : FileName만 따로 저장한다.

    if (OpenJobFile(strFilePath, strFileName))
    {
        SystemConfig::GetInstance().m_nRecipeOpenType = RECIPE_OPEN_TYPE_VISION_OPEN;
        PersonalConfig::getInstance().setLastAccessDirectory_Job(GET_PATHNAME(Dlg.GetPathName()));
        return TRUE;
    }

    SimpleMessage(GetLastLinkDataBaseErrorMessage());

    initDir.Empty();
    strFilePath.Empty();
    strFileName.Empty();
    return FALSE;
}

BOOL VisionMainTR::OpenJobFile(CString strFilePath, CString strFileName)
{
    if (strFilePath != "" && strFileName == "")
    {
        strFileName = REMOVE_FOLDER_FROM_PATHNAME(strFilePath);
        strFileName = REMOVE_EXT_FROM_FILENAME(strFileName);
    }

    CiDataBase jobDB;

    // Job File Open 한다.
    if (!jobDB.Load(strFilePath))
        return FALSE;


    BOOL bValid = LinkDataBase(FALSE, jobDB);

    if (bValid)
    {
        SetVisionJobName(strFilePath, strFileName);
        DevelopmentLog::AddLog(DevelopmentLog::Type::Event, _T("OpenJobFile : %s"), LPCTSTR(strFileName));
        m_systemConfig.m_tr.SetPrevJob(m_mainType, strFilePath);
        OnJobChanged();
    }
    else if (SimpleFunction::GetAsyncHiddenKeyDown(true, true) == TRUE)
    {
        SetVisionJobName(strFilePath, strFileName);
        OnJobChanged();
    }

    return bValid;
}

void VisionMainTR::SetVisionJobName(CString strFilePath, CString strFileName)
{
    m_strJobFileName = strFileName;
    m_strJobFilePath = strFilePath;
}

//----------------------------------------------------------------------------------------
// JobSave Version이 아직 개발버전인데
// 정식 Release시 Version을 정식 Release 버전으로 바꾸지 않았으면 컴파일 오류를 발생시킨다.
//----------------------------------------------------------------------------------------

static bool jobSaveVersionCheck[(IPIS500I_BUILD_NUMBER != IPIS500I_DEVELOPER_VERSION_BUILD_NUMBER
                                    && LAST_JOB_VERSION == IPIS500I_DEVELOPER_VERSION_BUILD_NUMBER)
        ? 0
        : 1];

BOOL VisionMainTR::LinkDataBase(BOOL bSave, CiDataBase& jobDB)
{
    m_lastLinkDataBaseErrorMessage.Empty();
    m_lastLinkDataBaseJobVersionChanged = FALSE;

    CString errorMessageMatchedVisionType;
    if (CheckVisionTypeInRecipe(bSave, jobDB, errorMessageMatchedVisionType) == false)
    {
        m_lastLinkDataBaseErrorMessage = errorMessageMatchedVisionType;
        return FALSE;
    }

    auto& mainDB = jobDB[_T("VisionMain")];

    // Job 이 수정되면 변경 당시의 새로운 Build Number을 Job Version으로 넣어주자
    // 이를 통해 현재 Job Version이 현재 Build Number 보다 낮으면 로드가 가능하다고 보자

    long version = LAST_JOB_VERSION;

    CString strAppVersion(m_strDetailFileVersion);
    if (!mainDB[_T("AppVersionNumber")].Link(bSave, strAppVersion))
    {
        if (!bSave)
            strAppVersion.Format(_T("Unknown : Under MED#3 Version"));
    }
    m_strRecipeFileVersion = strAppVersion;
    m_systemConfig.m_strRecipeVersion = strAppVersion;

    //TR Vision일 경우 Open한 Recipe가 InPocket용인지 OTI용인지 확인하고 현재 Vision과 맞지 않으면 경고를  띄운다.
    static const bool isTRV = SystemConfig::GetInstance().IsVisionTypeTR() == 1 ? true : false;

    if (isTRV)
    {
        CString strTRVisionTypeInRecipe;
        strTRVisionTypeInRecipe.Format(_T("{56FEC1B2-679D-4B5D-881A-8EC24D96E44C}"));

        if (bSave == FALSE)
        {
            if (!mainDB[strTRVisionTypeInRecipe].Link(bSave, m_trVisionTypeInRecipe))
                m_trVisionTypeInRecipe = -1;

            if (m_trVisionTypeInRecipe == -1) //해당 정보가 없다는 의미로 이 경우 현재 Type을 넣어 준다
            {
                m_trVisionTypeInRecipe = m_mainType;
            }
            else if (m_trVisionTypeInRecipe
                != m_mainType) //현재 Vision Type과 Recipe Type이 다를 때 경고 메시지 출력하고 Recipe는 열지 않는다.
            {
                if (m_mainType == TR_EXECUTE_IN_POCKET)
                    m_lastLinkDataBaseErrorMessage = (_T("This vision is In-Pocket, But opend recipe is not In-Pocket ")
                                                      _T("Type.\nPlease check the recipe file"));
                else
                    m_lastLinkDataBaseErrorMessage
                        = (_T("This vision is OTI, But opend recipe is not OTI Type.\nPlease check the recipe file"));

                return FALSE;
            }
        }
        else
        {
            if (m_trVisionTypeInRecipe == m_mainType) //현재 Vision Type과 Recipe Type이 같을 때만 저장
            {
                if (!mainDB[strTRVisionTypeInRecipe].Link(bSave, m_trVisionTypeInRecipe))
                    m_trVisionTypeInRecipe = -1;
            }
        }
    }
    else //이 함수 자체는 TR Vision일 때만 들어와야 하는데 isTRV가 false 이면 이 함수에서의 동작 자체가 이루어지면 안된다. 경고 Message를 띄우고 함수를 Rerurm FALSE한다.
    {
        m_lastLinkDataBaseErrorMessage = (_T("Vision Type is not TR Vision Type. Please check the vision type."));
        return FALSE;
    }


    /* Recipe의 Type을 저장 및 Open, 0 : Normal, 1 : ByPass, 2 : Review
	 BTM2D 인 것을 확인, 현재 Recipe를 어떤 MSG로 받았는지 확인 Host Connection check 조건 필요
	 If문으로 구성, else 필요 없음
	 1이 아니면서 ByPass일 때(ByPass MSG로 확인해야함), nType != 1 && ByPass_MSG == TRUE, 함수 재귀호출(FileName으로 Recipe 여는 함수 호출의 개념) :: 하지만 무한루프의 가능성이 있으므로 예외처리 필수
	 NGRV_Vision인데 2가 아니면 Default review recipe open, 재귀 호출 개념 :: 하지만 무한루프의 가능성이 있으므로 예외처리 필수*/

    long nRecipeOpenType = SystemConfig::GetInstance().m_nRecipeOpenType;
    if (!mainDB[_T("{63A72351-6467-4112-AAB1-A2744F663160}")].Link(bSave, m_nRecipeType))
        m_nRecipeType = RECIPE_TYPE_NORMAL;

    static BOOL bIsReOpen = FALSE;
    static BOOL bIsDefaultRecipeSend = FALSE;

    BOOL bIsNeedDefaultOpenNGRV = FALSE;
    BOOL bIsNeedDefaultOpenByPass = FALSE;

    if ((nRecipeOpenType == RECIPE_OPEN_TYPE_NORMAL || nRecipeOpenType == RECIPE_OPEN_TYPE_BYPASS) && bIsReOpen == FALSE
        && !bSave) // Host와 연결 되어 있을 때만 진입한다 - JHB_NGRV. bIsReOpen이 True면 이 구간을 태우면 안된다. 그냥 열어야한다.
    {
        if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP)
        {
            if (nRecipeOpenType == RECIPE_OPEN_TYPE_NORMAL && m_nRecipeType != RECIPE_TYPE_NGRV)
            {
                DevelopmentLog::AddLog(DevelopmentLog::Type::Notice,
                    _T("The recieved NGRV recipe is wrong. Recipe will be replaced by default recipe."));
                bIsNeedDefaultOpenNGRV = TRUE;
                bIsReOpen = TRUE;
            }
        }
        else if (SystemConfig::GetInstance().IsBtm2DVision())
        {
            if (nRecipeOpenType == RECIPE_OPEN_TYPE_BYPASS && m_nRecipeType != RECIPE_TYPE_BYPASS)
            {
                bIsNeedDefaultOpenByPass = TRUE;
                bIsReOpen = TRUE;
            }
        }

        if (bIsNeedDefaultOpenNGRV
            && bIsReOpen) //NGRV Default Recipe가 필요하면서 Recipe ReOpen이 TRUE이면 Default Recipe로 Open
        {
            //Recipe Open with default one

            CString strNGRVDefaultRecipeFileNameTarget = _T("Default_NGRV.vmjob");

            SystemConfig::GetInstance().m_nRecipeOpenType = RECIPE_OPEN_TYPE_VISION_OPEN;
            strNGRVDefaultRecipeFileNameTarget = REMOVE_EXT_FROM_FILENAME(strNGRVDefaultRecipeFileNameTarget);

            BOOL bValid = OpenJobFile(m_strDefaultRecipePathNameForNGRV, strNGRVDefaultRecipeFileNameTarget);

            if (bValid) //Recipe를 제대로 열었을 경우
            {
                bIsDefaultRecipeSend = TRUE; //Recipe를 Default로 열었으니 해당 Recipe 정보를 Host에 전달해줘야함
                m_nRecipeType = RECIPE_TYPE_NGRV; //Recipe Type을 NGRV로 변경
                bIsReOpen = FALSE; //다시 Recipe를 열 필요가 없으므로 FALSE로 변경
                SystemConfig::GetInstance().m_nRecipeOpenType = RECIPE_OPEN_TYPE_INIT;

                //Send Recipe가 추가되어야함
                if (bIsDefaultRecipeSend)
                {
                    iPIS_Send_Job(jobDB, MSG_PROBE_TEACH_END);
                    bIsDefaultRecipeSend = FALSE;
                }

                return bValid;
            }
            else
            {
                SystemConfig::GetInstance().m_nRecipeOpenType = RECIPE_OPEN_TYPE_INIT;

                return bValid; //어차피 OpenJobFile()에서 이 함수를 다시 부르니까.
            }
        }

        if (bIsNeedDefaultOpenByPass && bIsReOpen)
        {
            //Recipe Open with default one
            CString strByPassDefaultRecipeFileNameTarget = _T("Default_BYPASS.vmjob");

            SystemConfig::GetInstance().m_nRecipeOpenType = RECIPE_OPEN_TYPE_VISION_OPEN;
            strByPassDefaultRecipeFileNameTarget = REMOVE_EXT_FROM_FILENAME(strByPassDefaultRecipeFileNameTarget);

            BOOL bValid = OpenJobFile(m_strDefaultRecipePathNameForByPass, strByPassDefaultRecipeFileNameTarget);

            if (bValid) //Recipe를 제대로 열었을 경우
            {
                bIsDefaultRecipeSend = TRUE; //Recipe를 Default로 열었으니 해당 Recipe 정보를 Host에 전달해줘야함
                m_nRecipeType = RECIPE_TYPE_BYPASS; //Recipe Type을 BYPASS로 변경
                bIsReOpen = FALSE; //다시 Recipe를 열 필요가 없으므로 FALSE로 변경
                SystemConfig::GetInstance().m_nRecipeOpenType = RECIPE_OPEN_TYPE_INIT;

                //Send Recipe가 추가되어야함
                if (bIsDefaultRecipeSend)
                {
                    iPIS_Send_Job(jobDB, MSG_PROBE_TEACH_END);
                    bIsDefaultRecipeSend = FALSE;
                }

                return bValid;
            }
            else
            {
                SystemConfig::GetInstance().m_nRecipeOpenType = RECIPE_OPEN_TYPE_INIT;

                return bValid; //어차피 OpenJobFile()에서 이 함수를 다시 부르니까.
            }
        }
    }

    SystemConfig::GetInstance().m_nRecipeOpenType = RECIPE_OPEN_TYPE_INIT;

    if (!mainDB[_T("Version")].Link(bSave, version))
        version = 0;

    if (version == 0)
    {
        // 예전에는 Version 정보가 이곳에 있었다..
        if (!mainDB.Link(bSave, version))
            version = 0;
    }

    if (version > IPIS500I_BUILD_NUMBER)
    {
        m_lastLinkDataBaseErrorMessage = _T("The version of Job is higher than the program version.\r\n");
        m_lastLinkDataBaseErrorMessage.AppendFormat(
            _T("To load, you must update to a program version with build number %d or higher."), version);
        return FALSE;
    }

    if (version != LAST_JOB_VERSION)
    {
        m_lastLinkDataBaseJobVersionChanged = TRUE;
    }

    //{{//kircheis_NGRV
    if (!mainDB[_T("{D979D726-80C7-4B23-AA47-33AA991414A5}")].Link(bSave, m_bIsRecipeForNGRVVision))
        m_bIsRecipeForNGRVVision = false;

    if (m_bIsRecipeForNGRVVision == false)
    {
        if (!mainDB[_T("{3B274F2A-5D4B-4E12-9D19-07A13A8C387D}")].Link(bSave, m_bIsReadOnly2DMatMode))
            m_bIsReadOnly2DMatMode = false;
    }
    else
    {
        m_bIsReadOnly2DMatMode = false;
        //여기에 지금 Vision이 NGRV 인지 확인하는 부분이 필요하다.
        //일반 Vision이면 핸들러로 MSG_PROBE_NGRV_WARNING_RECIPE_TYPE 메세지를 보내야 한다.
    }
    //}}

    if (!m_pSlitBeam3DCommon->LinkDataBase(bSave, jobDB[_T("{D43325A2-1860-42E3-B9E0-96793CE270C9}")]))
    {
        return FALSE;
    }

    // hardware 먼저 열어야 뒤에 조명 세팅값이 정상 적용된다.
    if (version < 4)
    {
        m_illum2D.LinkDataBase(bSave, version, jobDB);
    }
    else
    {
        m_illum2D.LinkDataBase(bSave, version, jobDB[_T("Illum2D")]);
    }

    //{{//kircheis_NGRV
    if (SystemConfig::GetInstance().IsVisionTypeNGRV() == TRUE)
    {
        long nItemNum = (long)m_vecGrabItemFrameInfo.size();
        if (!mainDB[_T("{DEE44B12-C9D6-4AD2-8A60-FD3A26898BCE}")].Link(bSave, nItemNum))
            nItemNum = 0;

        if (!bSave)
        {
            m_vecGrabItemFrameInfo.clear();
            if (nItemNum > 0)
                m_vecGrabItemFrameInfo.resize(nItemNum);
        }

        for (long nIndex = 0; nIndex < nItemNum; nIndex++)
            NGRV_FrameInfoLinkDataBase(bSave, nIndex, jobDB, m_vecGrabItemFrameInfo[nIndex]);

        ClassifyInspItemInfo(m_vecGrabItemFrameInfo);

        if (!bSave)
        {
            NgrvFrameInfo ngrvFrameInfo;
            GetNGRVFrameInfo(ngrvFrameInfo);
            iPIS_Send_NGRV_GrabFrameInfo(ngrvFrameInfo);

            GetNGRVGrabPositionInfo(m_vecGrabPositionInfo);
            iPIS_Send_NGRV_GrabZposInfo(m_vecGrabPositionInfo);
        }

        NGRV_AFInfoLinkDataBase(bSave, jobDB);

        // VisionUnit에서 가져와야하는 NGRV 2D ImageInfo.. 왜 이렇게 밖에 못할까.. 차후에 방법이 있으면 해보자...MED#6 NGRV SingleRun_JHB
        if (bSave == TRUE)
        {
            m_ngrv2DImageInfo_BTM = GetPrimaryVisionUnit().Get2DImageInfo_NGRV(VISION_BTM_2D);
            m_ngrv2DImageInfo_TOP = GetPrimaryVisionUnit().Get2DImageInfo_NGRV(VISION_TOP_2D);
        }

        NGRV_2DImageInfoLinkDataBase(bSave, jobDB); // MED#6 NGRV SingleRun - JHB

        // Recipe Load할 경우에는 VisionUnit에서 가져와야하는 NGRV 2D ImageInfo 넣어줘야 한다...
        if (bSave == FALSE)
        {
            GetPrimaryVisionUnit().Set2DImageInfo_NGRV(VISION_BTM_2D, m_ngrv2DImageInfo_BTM);
            GetPrimaryVisionUnit().Set2DImageInfo_NGRV(VISION_TOP_2D, m_ngrv2DImageInfo_TOP);
        }
    }

    if (!GetPrimaryVisionUnit().LinkDataBase(version, bSave, jobDB))
    {
        return FALSE;
    }

    //{{//kircheis_NGRV Bypass
    auto& visionUnit = GetPrimaryVisionUnit();
    //long nPreProcessNum = 2; //_VISION_INSP_GUID_PACKAGE_SPEC, _VISION_INSP_GUID_FOV
    long nUseProcessNumForBypassMode = 0;
    BOOL bUse2DMatrixBypassMode = false;
    long nVisionModuleNum = CAST_LONG(visionUnit.m_vecVisionModules.size());
    if (nVisionModuleNum <= 3)
    {
        for (long i = 0; i < nVisionModuleNum; i++)
        {
            //auto* inspectModule = dynamic_cast<VisionInspection*>(visionUnit.m_vecVisionModules[i]);
            CString strModuleGUID = visionUnit.m_vecVisionModules[i]->m_moduleGuid;
            if (strModuleGUID.Find(_VISION_INSP_GUID_PACKAGE_SPEC) == 0)
                nUseProcessNumForBypassMode++;
            else if (strModuleGUID.Find(_VISION_INSP_GUID_FOV) == 0)
                nUseProcessNumForBypassMode++;
            else if (strModuleGUID.Find(_VISION_INSP_GUID_2D_MATRIX) == 0)
                nUseProcessNumForBypassMode++;
        }
        if (nUseProcessNumForBypassMode == 3)
            bUse2DMatrixBypassMode = true;
    }
    visionUnit.Set2DMatrixBypassMode(bUse2DMatrixBypassMode);
    //}}

    BOOL bUseIntensityCheckerBypassMode = false;
    nUseProcessNumForBypassMode = 0;
    if (nVisionModuleNum <= 3)
    {
        for (long i = 0; i < nVisionModuleNum; i++)
        {
            //auto* inspectModule = dynamic_cast<VisionInspection*>(visionUnit.m_vecVisionModules[i]);
            CString strModuleGUID = visionUnit.m_vecVisionModules[i]->m_moduleGuid;
            if (strModuleGUID.Find(_VISION_INSP_GUID_PACKAGE_SPEC) == 0)
                nUseProcessNumForBypassMode++;
            else if (strModuleGUID.Find(_VISION_INSP_GUID_FOV) == 0)
                nUseProcessNumForBypassMode++;
            else if (strModuleGUID.Find(_VISION_INSP_GUID_INTENSITYCHECKER_2D) == 0)
                nUseProcessNumForBypassMode++;
        }
        if (nUseProcessNumForBypassMode == 3)
            bUseIntensityCheckerBypassMode = true;
    }

    visionUnit.SetIntensityCheckerBypassMode(bUseIntensityCheckerBypassMode);

    if (!bSave)
    {
        for (long nThread = 0; nThread < m_nThreadNum; nThread++)
        {
            if (!m_unitBuffers[nThread]->LinkDataBase(version, bSave, jobDB))
            {
                return FALSE;
            }
            m_unitBuffers[nThread]->Set2DMatrixBypassMode(bUse2DMatrixBypassMode); //kircheis_NGRV Bypass
            m_unitBuffers[nThread]->SetIntensityCheckerBypassMode(bUseIntensityCheckerBypassMode);
        }
    }
    strAppVersion.Empty();

    return TRUE;
}

bool VisionMainTR::CheckVisionTypeInRecipe(const BOOL& bSave, CiDataBase& jobDB, CString& errorText)
{
    errorText.Empty();
    static const long currentVisionType = SystemConfig::GetInstance().GetVisionType();
    static const CString currentVisionTypeText = SystemConfig::GetInstance().GetVisionTypeText();
    long inRecipeVisionType = currentVisionType;

    const long currentSideVisionNumber = static_cast<long>(SystemConfig::GetInstance().GetSideVisionNumber());
    long inRecipeSideVisionNumber = currentSideVisionNumber;

    auto& mainDB = jobDB[_T("VisionMain")];

    if (bSave == false)
    {
        if (!mainDB[RECIPE_KEY_VALUE_VISION_TYPE].Link(bSave, inRecipeVisionType))
        {
            inRecipeVisionType = VISIONTYPE_UNKNOWN;
            //Recipe에 Vision Type에 대한 정보가 없으면, true 반환
            return true;
        }

        bool isHiddenKeyDown = SimpleFunction::GetAsyncHiddenKeyDown(true, true);
        if (currentVisionType == inRecipeVisionType || isHiddenKeyDown)
        {
            return true;
        }
        else
        {
            CString inRecipeVisionTypeText = SystemConfig::GetInstance().GetVisionTypeText(inRecipeVisionType);
            errorText.Format(_T("The vision type in the recipe (%ls) is different from the current vision type (%ls)."),
                inRecipeVisionTypeText, currentVisionTypeText);
            return false;
        }
    }
    else
    {
        mainDB[RECIPE_KEY_VALUE_VISION_TYPE].Link(bSave, inRecipeVisionType);
    }

    return true;
}

BOOL VisionMainTR::NGRV_FrameInfoLinkDataBase(
    BOOL bSave, long nIndex, CiDataBase& JobDB, CGrabItemFrameInfoNGRV& io_grabItemFrameInfoNGRV) //kircheis_NGRV
{
    auto& mainDB = JobDB[_T("VisionMain")];

    CString m_strGrabItemName;
    std::vector<long> m_vecnFrameID;

    CString strGrabItemNameID;
    CString strIsSaveImageID;
    CString strFrameID_Cnt;
    CString strFrameID;
    CString strAFTypeID;
    CString strPlaneOffsetUMID;

    strGrabItemNameID.Format(_T("{56C462C5-0BA5-48AC-AD89-D61AA209CC9A}_%04d"), nIndex);
    strIsSaveImageID.Format(_T("{8646DF8F-F6E6-4980-8BA1-7F53B6991FA2}_%04d"), nIndex);
    strFrameID_Cnt.Format(_T("{504A1172-4FC5-4EA8-A06F-F27F48B089F6}_%04d"), nIndex);
    strAFTypeID.Format(_T("{B2535165-C071-4B22-B582-A64BDC6D44A2}_%04d"), nIndex);
    strPlaneOffsetUMID.Format(_T("{A895A4EE-788A-4C83-81FA-BE8BC1948ED9}_%04d"), nIndex);

    long nFrameSize = (long)io_grabItemFrameInfoNGRV.m_vecnFrameID.size();

    if (!mainDB[strGrabItemNameID].Link(bSave, io_grabItemFrameInfoNGRV.m_strGrabItemName))
        io_grabItemFrameInfoNGRV.m_strGrabItemName.Format(_T("NoName_%04d"), nIndex);
    if (!mainDB[strIsSaveImageID].Link(bSave, io_grabItemFrameInfoNGRV.m_bIsSaveImage))
        io_grabItemFrameInfoNGRV.m_bIsSaveImage = false;
    if (!mainDB[strFrameID_Cnt].Link(bSave, nFrameSize))
        nFrameSize = 1;
    if (!mainDB[strAFTypeID].Link(bSave, io_grabItemFrameInfoNGRV.m_nAFType))
        io_grabItemFrameInfoNGRV.m_nAFType = NgrvGrabPositionInfo::enumNGRVAFMode::NGRV_AF_MODE_EACH_POINT;
    if (!mainDB[strPlaneOffsetUMID].Link(bSave, io_grabItemFrameInfoNGRV.m_nPlaneOffsetUM))
        io_grabItemFrameInfoNGRV.m_nPlaneOffsetUM = 0;

    io_grabItemFrameInfoNGRV.m_nPlaneOffsetUM = (long)max(io_grabItemFrameInfoNGRV.m_nPlaneOffsetUM, -10000);
    io_grabItemFrameInfoNGRV.m_nPlaneOffsetUM = (long)min(io_grabItemFrameInfoNGRV.m_nPlaneOffsetUM, 10000);

    if (!bSave)
    {
        io_grabItemFrameInfoNGRV.m_vecnFrameID.clear();
        io_grabItemFrameInfoNGRV.m_vecnFrameID.resize(nFrameSize);
    }

    for (long nFrame = 0; nFrame < nFrameSize; nFrame++)
    {
        strFrameID_Cnt.Format(_T("{443292DF-BD64-4B53-9395-8FE8C7207ECE}_%04d_%02d"), nIndex, nFrame);
        if (!mainDB[strFrameID_Cnt].Link(bSave, io_grabItemFrameInfoNGRV.m_vecnFrameID[nFrame]))
            io_grabItemFrameInfoNGRV.m_vecnFrameID[nFrame] = 0;
    }

    return true;
}

BOOL VisionMainTR::NGRV_2DImageInfoLinkDataBase(BOOL bSave, CiDataBase& JobDB)
{
    auto& mainDB = JobDB[_T("VisionMain")];
    CString strKeyName;

    if (!mainDB[_T("{BCABB1D8-2FF9-4A14-BF15-F97FBE27E7DF_BTM}")].Link(bSave, m_ngrv2DImageInfo_BTM.m_visionID))
    {
        m_ngrv2DImageInfo_BTM.m_visionID = VISION_BTM_2D;
    }

    if (!mainDB[_T("{236F25A9-6E24-43DC-AA28-535DD0AAD8AC_BTM}")].Link(bSave, m_ngrv2DImageInfo_BTM.m_imageSizeX))
    {
        m_ngrv2DImageInfo_BTM.m_imageSizeX = FrameGrabber::GetInstance().get_grab_image_width();
    }

    if (!mainDB[_T("{C25E86CF-0721-42EC-AE83-7D26ECA1E938_BTM}")].Link(bSave, m_ngrv2DImageInfo_BTM.m_imageSizeY))
    {
        m_ngrv2DImageInfo_BTM.m_imageSizeY = FrameGrabber::GetInstance().get_grab_image_height();
        ;
    }

    if (!mainDB[_T("{34E58EE7-646A-44E3-8981-8C8DE4BD0340_BTM}")].Link(bSave, m_ngrv2DImageInfo_BTM.m_totalImageSize))
    {
        m_ngrv2DImageInfo_BTM.m_totalImageSize
            = m_ngrv2DImageInfo_BTM.m_imageSizeX * m_ngrv2DImageInfo_BTM.m_imageSizeY;
    }

    if (!mainDB[_T("{888574C0-9359-4844-95DC-387D08EDCD3B_BTM}")].Link(bSave, m_ngrv2DImageInfo_BTM.m_scaleX))
    {
        m_ngrv2DImageInfo_BTM.m_scaleX = 0;
    }

    if (!mainDB[_T("{DEE925A1-0FF0-4298-B012-D17A5A6089F9_BTM}")].Link(bSave, m_ngrv2DImageInfo_BTM.m_scaleY))
    {
        m_ngrv2DImageInfo_BTM.m_scaleY = 0;
    }

    if (!mainDB[_T("{C2519055-2E2A-4FF1-88A8-FE96614D4695_BTM}")].Link(bSave, m_ngrv2DImageInfo_BTM.m_isImageColor))
    {
        m_ngrv2DImageInfo_BTM.m_isImageColor = FALSE;
    }

    long totalImageSize = m_ngrv2DImageInfo_BTM.m_totalImageSize;
    if (!bSave)
    {
        //m_ngrv2DImageInfo_BTM.m_vecbyImage.clear();
        m_ngrv2DImageInfo_BTM.m_vecbyImage.resize(totalImageSize);
    }
    if (!mainDB[_T("{24372BFC-F923-4441-8CC5-C7CD8DF10380_BTM}")].LinkArray(
            bSave, &m_ngrv2DImageInfo_BTM.m_vecbyImage[0], totalImageSize))
    {
        memset(&m_ngrv2DImageInfo_BTM.m_vecbyImage[0], BYTE(0), totalImageSize);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if (!mainDB[_T("{BCABB1D8-2FF9-4A14-BF15-F97FBE27E7DF_TOP}")].Link(bSave, m_ngrv2DImageInfo_TOP.m_visionID))
    {
        m_ngrv2DImageInfo_TOP.m_visionID = VISION_TOP_2D;
    }

    if (!mainDB[_T("{236F25A9-6E24-43DC-AA28-535DD0AAD8AC_TOP}")].Link(bSave, m_ngrv2DImageInfo_TOP.m_imageSizeX))
    {
        m_ngrv2DImageInfo_TOP.m_imageSizeX = FrameGrabber::GetInstance().get_grab_image_width();
    }

    if (!mainDB[_T("{C25E86CF-0721-42EC-AE83-7D26ECA1E938_TOP}")].Link(bSave, m_ngrv2DImageInfo_TOP.m_imageSizeY))
    {
        m_ngrv2DImageInfo_TOP.m_imageSizeY = FrameGrabber::GetInstance().get_grab_image_height();
    }

    if (!mainDB[_T("{34E58EE7-646A-44E3-8981-8C8DE4BD0340_TOP}")].Link(bSave, m_ngrv2DImageInfo_TOP.m_totalImageSize))
    {
        m_ngrv2DImageInfo_TOP.m_totalImageSize
            = m_ngrv2DImageInfo_TOP.m_imageSizeX * m_ngrv2DImageInfo_TOP.m_imageSizeY;
    }

    if (!mainDB[_T("{888574C0-9359-4844-95DC-387D08EDCD3B_TOP}")].Link(bSave, m_ngrv2DImageInfo_TOP.m_scaleX))
    {
        m_ngrv2DImageInfo_TOP.m_scaleX = 0;
    }

    if (!mainDB[_T("{DEE925A1-0FF0-4298-B012-D17A5A6089F9_TOP}")].Link(bSave, m_ngrv2DImageInfo_TOP.m_scaleY))
    {
        m_ngrv2DImageInfo_TOP.m_scaleY = 0;
    }

    if (!mainDB[_T("{C2519055-2E2A-4FF1-88A8-FE96614D4695_TOP}")].Link(bSave, m_ngrv2DImageInfo_TOP.m_isImageColor))
    {
        m_ngrv2DImageInfo_TOP.m_isImageColor = FALSE;
    }

    long imageSize = m_ngrv2DImageInfo_TOP.m_totalImageSize;
    if (!bSave)
    {
        //m_ngrv2DImageInfo_TOP.m_vecbyImage.clear();
        m_ngrv2DImageInfo_TOP.m_vecbyImage.resize(imageSize);
    }
    if (!mainDB[_T("{24372BFC-F923-4441-8CC5-C7CD8DF10380_TOP}")].LinkArray(
            bSave, &m_ngrv2DImageInfo_TOP.m_vecbyImage[0], imageSize))
    {
        memset(&m_ngrv2DImageInfo_TOP.m_vecbyImage[0], BYTE(0), imageSize);
    }

    return TRUE;
}

BOOL VisionMainTR::NGRV_AFInfoLinkDataBase(BOOL bSave, CiDataBase& JobDB)
{
    auto& mainDB = JobDB[_T("VisionMain")];
    CString strKeyName;

    if (!mainDB[_T("{3124C2A1-7C73-4D34-ABB9-5F95D3C8E6CE_BTM}")].Link(bSave, m_ngrvAFRefInfor_BTM.m_nVisionID))
    {
        m_ngrvAFRefInfor_BTM.m_nVisionID = VISION_BTM_2D;
    }

    if (!mainDB[_T("{3A3A765E-D3AC-471D-8037-DD253F271914_BTM}")].Link(
            bSave, m_ngrvAFRefInfor_BTM.m_bIsValidPlaneRefInfo))
    {
        m_ngrvAFRefInfor_BTM.m_bIsValidPlaneRefInfo = false;
    }

    for (long nIndex = 0; nIndex < (long)m_ngrvAFRefInfor_BTM.m_vecptRefPos_UM.size(); nIndex++)
    {
        strKeyName.Format(_T("{B78A44C7-2D2D-4EB8-9AD7-BEFDFE869638_BTM_%d}"), nIndex);
        Ipvm::Point32s2 temp(
            m_ngrvAFRefInfor_BTM.m_vecptRefPos_UM[nIndex].x, m_ngrvAFRefInfor_BTM.m_vecptRefPos_UM[nIndex].y);
        if (!mainDB[strKeyName].Link(bSave, temp))
        {
            m_ngrvAFRefInfor_BTM.m_vecptRefPos_UM[nIndex].SetPoint(0, 0);
        }
    }

    if (!mainDB[_T("{7CA61EF8-A689-4B15-A89C-6FBE1FB9E18C_BTM}")].Link(bSave, m_ngrvAFRefInfor_BTM.m_nImageSizeX))
    {
        m_ngrvAFRefInfor_BTM.m_nImageSizeX = FrameGrabber::GetInstance().get_grab_image_width();
    }

    if (!mainDB[_T("{AF2D37EA-DD5D-485A-BED9-3E40BAEB7941_BTM}")].Link(bSave, m_ngrvAFRefInfor_BTM.m_nImageSizeY))
    {
        m_ngrvAFRefInfor_BTM.m_nImageSizeY = FrameGrabber::GetInstance().get_grab_image_height();
    }

    long nImageSizeBTM = m_ngrvAFRefInfor_BTM.m_nImageSizeX * m_ngrvAFRefInfor_BTM.m_nImageSizeY * 3;
    if (!bSave)
    {
        m_ngrvAFRefInfor_BTM.m_vecbyImage.clear();
        m_ngrvAFRefInfor_BTM.m_vecbyImage.resize(nImageSizeBTM);
    }
    if (!mainDB[_T("{590CA2CE-9834-48D4-AA1B-7BC6D8A41684_BTM}")].LinkArray(
            bSave, &m_ngrvAFRefInfor_BTM.m_vecbyImage[0], nImageSizeBTM))
    {
        memset(&m_ngrvAFRefInfor_BTM.m_vecbyImage[0], BYTE(0), nImageSizeBTM);
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    if (!mainDB[_T("{3124C2A1-7C73-4D34-ABB9-5F95D3C8E6CE_TOP}")].Link(bSave, m_ngrvAFRefInfor_TOP.m_nVisionID))
    {
        m_ngrvAFRefInfor_TOP.m_nVisionID = VISION_TOP_2D;
    }

    if (!mainDB[_T("{3A3A765E-D3AC-471D-8037-DD253F271914_TOP}")].Link(
            bSave, m_ngrvAFRefInfor_TOP.m_bIsValidPlaneRefInfo))
    {
        m_ngrvAFRefInfor_TOP.m_bIsValidPlaneRefInfo = false;
    }

    for (long nIndex = 0; nIndex < (long)m_ngrvAFRefInfor_TOP.m_vecptRefPos_UM.size(); nIndex++)
    {
        strKeyName.Format(_T("{B78A44C7-2D2D-4EB8-9AD7-BEFDFE869638_TOP_%d}"), nIndex);
        Ipvm::Point32s2 temp(
            m_ngrvAFRefInfor_TOP.m_vecptRefPos_UM[nIndex].x, m_ngrvAFRefInfor_TOP.m_vecptRefPos_UM[nIndex].y);
        if (!mainDB[strKeyName].Link(bSave, temp))
        {
            m_ngrvAFRefInfor_TOP.m_vecptRefPos_UM[nIndex].SetPoint(0, 0);
        }
    }

    if (!mainDB[_T("{7CA61EF8-A689-4B15-A89C-6FBE1FB9E18C_TOP}")].Link(bSave, m_ngrvAFRefInfor_TOP.m_nImageSizeX))
    {
        m_ngrvAFRefInfor_TOP.m_nImageSizeX = FrameGrabber::GetInstance().get_grab_image_width();
    }

    if (!mainDB[_T("{AF2D37EA-DD5D-485A-BED9-3E40BAEB7941_TOP}")].Link(bSave, m_ngrvAFRefInfor_TOP.m_nImageSizeY))
    {
        m_ngrvAFRefInfor_TOP.m_nImageSizeY = FrameGrabber::GetInstance().get_grab_image_height();
    }

    long nImageSizeTOP = m_ngrvAFRefInfor_TOP.m_nImageSizeX * m_ngrvAFRefInfor_TOP.m_nImageSizeY * 3;
    if (!bSave)
    {
        m_ngrvAFRefInfor_TOP.m_vecbyImage.clear();
        m_ngrvAFRefInfor_TOP.m_vecbyImage.resize(nImageSizeTOP);
    }

    if (!mainDB[_T("{590CA2CE-9834-48D4-AA1B-7BC6D8A41684_TOP}")].LinkArray(
            bSave, &m_ngrvAFRefInfor_TOP.m_vecbyImage[0], nImageSizeTOP))
    {
        memset(&m_ngrvAFRefInfor_TOP.m_vecbyImage[0], BYTE(0), nImageSizeTOP);
    }

    return TRUE;
}

BOOL VisionMainTR::LinkDataBase_PackageSpec(BOOL bSave, CiDataBase& jobDB)
{
    return GetPrimaryVisionUnit().LinkDataBase_PackageSpec(bSave, jobDB);
}

BOOL VisionMainTR::OpenJobFileAgent()
{
    return OpenJobFile();
}

BOOL VisionMainTR::OpenImageFile()
{
    return GetPrimaryVisionUnit().OpenImageFile();
}

void VisionMainTR::SendDLResult(VisionDeviceResult& DeviceResult, BOOL& bSendSuccess)
{
    iPIS_Send_DLInspResult(DeviceResult, bSendSuccess);
}

void VisionMainTR::SendResult(VisionUnit& visionUnit, VisionDeviceResult& DeviceResult, BOOL& bSendSuccess)
{
    iPIS_Send_InspResult(visionUnit, DeviceResult, bSendSuccess);

    SystemConfig::GetInstance().Save_DeviceSendResultTimeLog(DeviceResult.m_nTrayID, DeviceResult.m_nPocketID,
        DeviceResult.m_nPane, DeviceResult.m_inspectionStartTime, bSendSuccess,
        SystemConfig::GetInstance().Get_SaveDeviceSendResultTimeLog());
}

void VisionMainTR::SendResult(VisionUnit& visionUnit, SideVisionDeviceResult& DeviceResult, BOOL& bSendSuccess)
{
    iPIS_Send_InspResult(visionUnit, DeviceResult, bSendSuccess);
}

void VisionMainTR::SendImage(const Ipvm::Image8u& image) //kircheis_UPH_up
{
    iPIS_Send_Image(image);
}

void VisionMainTR::SendImage(
    const Ipvm::Image8u& imageFront, const Ipvm::Image8u& imageRear) //Side Vision Inline 전송용
{
    iPIS_Send_Image(imageFront, imageRear);
}

void VisionMainTR::Send2DAllFrame() //mc_MED#3.5 AllFrameSave
{
    iPIS_Send_2DAllFrame();
}

bool VisionMainTR::isConnect() const //kircheis_UPH_up
{
    return m_pMessageSocket->IsConnected();
}

long VisionMainTR::GetInlineRunCnt() const
{
    return m_inline_grab->GetInlineRunCnt();
}

void VisionMainTR::SetInlineRunCnt(long inlineRunCnt)
{
    m_inline_grab->SetInlineRunCnt(inlineRunCnt);
}

void VisionMainTR::SetScanTrayIndex(long trayIndex)
{
    m_inline_grab->SetScanTrayIndex(trayIndex);
}

void VisionMainTR::SetInspectionRepeatIndex(long repeatIndex)
{
    m_nInspectionRepeatIndex = repeatIndex;
}

long VisionMainTR::GetInspectionRepeatIndex()
{
    return m_nInspectionRepeatIndex;
}

void VisionMainTR::SetScanSpec(float scanLength_mm)
{
    m_pSlitBeam3DCommon->m_scanLength_mm = scanLength_mm;
}

void VisionMainTR::SetTrayScanSpec(const VisionTrayScanSpec& spec)
{
    for (long nThread = 0; nThread <= m_nThreadNum; nThread++)
    {
        m_unitBuffers[nThread]->SetTrayScanSpec(spec);
    }

    OnImagingConditionChanged();
}

void VisionMainTR::Iocheck()
{
    SyncController::GetInstance().PopupDioCheckDialog();
}

BOOL VisionMainTR::VisionPrimaryUI_SaveImageFile()
{
    if (m_menuAccessUI)
    {
        return m_menuAccessUI->callImageSave();
    }

    return GetPrimaryVisionUnit().SaveImageFile();
}

//  현식 마크티치관련 수정
void VisionMainTR::OnMarkTeachDone()
{
    if (m_bMarkTeach)
    {
        if (SaveJobFile(CString(_T("")), SaveJobMode::Normal, TRUE, TRUE))
        {
            iPIS_MarkTeach_Done();
            iPIS_ChangeDisplay(0);
            iPIS_Send_Start_Handler(MSG_PROBE_START_HANDLER);
        }
    }
    else
    {
        SaveJobFile(CString(_T("")), SaveJobMode::Normal, TRUE, TRUE);
    }
}

void VisionMainTR::OnMarkTeachSkip()
{
    iPIS_MarkTeach_Skip();
}

void VisionMainTR::OnImageGrab()
{
    //mc_TestCode
    bool isOn = true;
    SetInlineStartFlag(isOn);
    SetSyncStartFlag();

    return;
    //

    if (!SystemConfig::GetInstance().m_bHardwareExist)
        return;

    if (m_menuAccessUI)
    {
        m_menuAccessUI->callGrab();
        return;
    }

    auto& imageLot = GetPrimaryVisionUnit().getImageLot();

    //if (!m_systemConfig.m_bHardwareExist) return;

    FrameGrabber::GetInstance().live_off();
    SyncController::GetInstance().TurnOffLight(enTRVisionmodule(m_mainType));

    ManualGrab2D(imageLot, enTRVisionmodule(m_mainType));

    m_pVisionPrimaryUI->ShowCurrentImage(true);
}

bool VisionMainTR::ManualGrab2D(VisionImageLot& imageLot, const enTRVisionmodule& TR_Visionmodule)
{
    auto& visionReusableMemory = *GetPrimaryVisionUnit().m_VisionReusableMemory;

    std::vector<BYTE*> vecBufferMem;

    for (long imageIndex = 0; imageIndex < imageLot.GetImageFrameCount(enSideVisionModule(TR_Visionmodule)); imageIndex++)
    {
        vecBufferMem.push_back(visionReusableMemory.GetByteMemory());
    }

    long scanAreaIndex = 0;
    long stitchGrabIndex = 0;

    imageLot.SetTrayIndex(m_inline_grab->GetTrayIndex(), enSideVisionModule(TR_Visionmodule));
    imageLot.SetRunTimeIndex(m_inline_grab->GetInlineRunCnt(), enSideVisionModule(TR_Visionmodule));
    imageLot.SetScales(m_systemConfig.GetScale(enSideVisionModule(TR_Visionmodule)));

    imageLot.SetScanAreaIndex(scanAreaIndex, enSideVisionModule(TR_Visionmodule));

    static const bool bIs380Bridge = SystemConfig::GetInstance().GetHandlerType() == HANDLER_TYPE_380BRIDGE;
    short nImageFrameCount = (short)vecBufferMem.size();
    short nHighPosGrabCount = bIs380Bridge ? 0 : (short)(m_illum2D.getHighFrameCount()); //추후 제대로된 값을 넣자
    short nNormalGrabCount = nImageFrameCount - nHighPosGrabCount;

    FrameGrabber::GetInstance().StartGrab2D((int)enSideVisionModule(TR_Visionmodule), 0, &vecBufferMem[0], (short)nNormalGrabCount, nullptr);

    SyncController::GetInstance().StartSyncBoard(FALSE, false, TR_Visionmodule);

    FrameGrabber::GetInstance().wait_grab_end((int)enSideVisionModule(TR_Visionmodule), 0, nullptr);

    SyncController::GetInstance().TurnOffLight(TR_Visionmodule);

    imageLot.Paste2D(stitchGrabIndex, &vecBufferMem[0], (long)vecBufferMem.size(), 1, 1, enSideVisionModule(TR_Visionmodule));

    GetPrimaryVisionUnit().getImageLotInsp().Set(imageLot, enSideVisionModule(TR_Visionmodule));

    // 버퍼 해제
    for (auto* mem : vecBufferMem)
    {
        visionReusableMemory.Release_ByteMemory(mem);
    }

    return true;
}

void VisionMainTR::ResetLiveMain()
{
    m_pVisionPrimaryUI->ResetLiveMain();
}

void VisionMainTR::OnGotoHost()
{
    iPIS_ChangeDisplay(0);
}

void VisionMainTR::SetLiveMain()
{
    m_pVisionPrimaryUI->SetLiveMain();
}

void VisionMainTR::OnLogin()
{
    m_pVisionPrimaryUI->OnLogin();
}

BOOL VisionMainTR::VisionPrimaryUI_OpenJobFile()
{
    if (m_menuAccessUI)
    {
        return m_menuAccessUI->callJobOpen();
    }

    return OpenJobFile();
}

BOOL VisionMainTR::VisionPrimaryUI_SaveJobFile()
{
    if (m_menuAccessUI)
    {
        return m_menuAccessUI->callJobSave();
    }

    return SaveJobFile(CString(_T("")), SaveJobMode::Normal);
}

BOOL VisionMainTR::VisionPrimaryUI_OpenImageFile()
{
    if (m_menuAccessUI)
    {
        return m_menuAccessUI->callImageOpen();
    }

    return m_pVisionPrimaryUI->OnFileImageOpen();
}

BOOL VisionMainTR::OpenImageFile(CString strFileName)
{
    return GetPrimaryVisionUnit().OpenImageFile(strFileName, false);
}

BOOL VisionMainTR::OpenImageFileAgent()
{
    return OpenImageFile();
}

BOOL VisionMainTR::OpenImageFileAgent(CString strFileName)
{
    return OpenImageFile(strFileName);
}

BOOL VisionMainTR::SetPackageType()
{
    // 20141021 영훈 : 여기에 초기화 코드 넣기
    // JOb 초기화
    // Debuginfo 초기화
    return GetPrimaryVisionUnit().SetPackageType();
}

CString VisionMainTR::GetPrevJob()
{
    return m_systemConfig.m_tr.GetPrevJob(m_mainType);
}

void VisionMainTR::ReviewImageViewer()
{
    m_pVisionPrimaryUI->ReviewImageViewer();
}

void VisionMainTR::SystemSetup()
{
    //iPIS_Send_TextRecipe();//kircheis_TxtRecipeDebug
    m_pVisionPrimaryUI->SystemSetup();
}

CWnd* VisionMainTR::GetVisionPrimaryDialog()
{
    if (m_pVisionPrimaryUI == nullptr)
        return nullptr;
    return m_pVisionPrimaryUI->m_pDlg;
}

CWnd* VisionMainTR::GetVisionInlineDialog(long nSelect)
{
    if (m_pVisionInlineUI == nullptr || m_nThreadNum == 0)
        return (CWnd*)false;
    return (CWnd*)m_pVisionInlineUI[nSelect].GetDlg();
}

BOOL VisionMainTR::OnJobChanged()
{
    m_pVisionPrimaryUI->OnJobChanged();

    HardwareSetup();
    OnImagingConditionChanged();

    static const auto& systemConfig = SystemConfig::GetInstance();
    static const BOOL b2DIllum
        = ((systemConfig.GetVisionType() == VISIONTYPE_2D_INSP || systemConfig.GetVisionType() == VISIONTYPE_NGRV_INSP
               || systemConfig.GetVisionType() == VISIONTYPE_SIDE_INSP || systemConfig.GetVisionType() == VISIONTYPE_TR)
            && systemConfig.IsHardwareExist());
    static BOOL bFirstExecute = FALSE;
    if (b2DIllum && !bFirstExecute)
    {
        m_pVisionPrimaryUI->SetLiveMain();
        Sleep(200);
        m_pVisionPrimaryUI->ResetLiveMain();
        if (systemConfig.IsVisionTypeSide() == TRUE)
        {
            Sleep(200);
            SetSideVisionSection(enSideVisionModule::SIDE_VISIONMODULE_REAR);
            m_pVisionPrimaryUI->SetLiveMain();
            Sleep(200);
            m_pVisionPrimaryUI->ResetLiveMain();
            SetSideVisionSection(enSideVisionModule::SIDE_VISIONMODULE_FRONT);
        }
        bFirstExecute = TRUE;
    }

    return TRUE;
}

BOOL VisionMainTR::HardwareSetup()
{
    const auto& systemConfig = SystemConfig::GetInstance();

    static const bool bIs380Bridge = systemConfig.GetHandlerType() == HANDLER_TYPE_380BRIDGE;
    static const long nVisionType = m_systemConfig.GetVisionType();

    switch (nVisionType)
    {
        case VISIONTYPE_TR:
        {
            SyncController::GetInstance().SetTriggerEdgeMode(TRUE, TRUE, TRUE, enTRVisionmodule(m_mainType));

            //SyncController::GetInstance().SetFramePeriod(m_systemConfig.GetGrabDuration());//kircheis_CamTrans

            long nTotalFrameCount = m_illum2D.getTotalFrameCount();
            long nHighPosGrabCount = bIs380Bridge
                ? 0
                : (short)(m_illum2D
                          .getHighFrameCount()); //kircheis_MultiGrab //추후 Job에 있는 Hig Pos Grab Count를 쓰자
            SyncController::GetInstance().SetFrameCount(
                nTotalFrameCount - nHighPosGrabCount, nHighPosGrabCount, enTRVisionmodule(m_mainType));

            //{{//kircheis_CamTrans
            float fFrameExposureSum = 0.f;
            float fFrameExposureMax = 0.f;
            //}}

            // Set 2D Illumination
            for (int i = 0; i < nTotalFrameCount; i++)
            {
                SyncController::GetInstance().SetIllumiParameter(i, m_illum2D.getIllum(i), true, false, FALSE, enTRVisionmodule(m_mainType));

                //{{//kircheis_CamTrans
                auto& illum = m_illum2D.getIllum(i);
                fFrameExposureSum = 0.f;
                for (long nCh = 0; nCh < 16; nCh++)
                    fFrameExposureSum += illum[nCh];

                fFrameExposureMax = (float)max(fFrameExposureMax, fFrameExposureSum);
                //}}
            }

            //{{//kircheis_CamTrans//두영아
            static const int nDuration = m_systemConfig.GetGrabDuration();
            int nMaxExposure = (int)(fFrameExposureMax + .5f) + m_systemConfig.m_nCameraTransferTimeMS;
            m_systemConfig.m_nLastDownloadDuration = nMaxExposure;
            SyncController::GetInstance().SetFramePeriod(CAST_FLOAT(max(nDuration, nMaxExposure)), enTRVisionmodule(m_mainType));
            //SyncController::GetInstance().SetCameraTransferTime(m_systemConfig.m_nCameraTransferTimeMS);
            //}}
        }
        break;
    }

    return TRUE;
}

BOOL VisionMainTR::HardwareSetupForNGRV(std::vector<long> vecnFrameID, BOOL bUseIR, BOOL bInline)
{
    return TRUE;
}

BOOL VisionMainTR::OnImagingConditionChanged()
{
    static const long nVisionType = m_systemConfig.GetVisionType();

    switch (nVisionType)
    {
        case VISIONTYPE_2D_INSP:
            m_inline_grab->initialize2D(m_illum2D.getTotalFrameCount(), m_illum2D.getHighFrameCount());
            break;

        case VISIONTYPE_3D_INSP:
            m_inline_grab->initialize3D(m_pSlitBeam3DCommon->GetLogicalScanLength_px());
            break;

        case VISIONTYPE_NGRV_INSP:
            m_inline_grab->initializeColorVision(m_illum2D.getTotalFrameCount(), m_illum2D.getHighFrameCount());
            break;

        case VISIONTYPE_SIDE_INSP:
            m_inline_grab->initializeSide(m_illum2D.getTotalFrameCount(), m_illum2D.getHighFrameCount());
            break;

        case VISIONTYPE_TR:
            m_inline_grab->initializeTR(m_illum2D.getTotalFrameCount(), m_illum2D.getHighFrameCount());
            break;
    }

    auto maxImageSize = m_inline_grab->GetImageBufferSize();

    for (long nThread = 0; nThread <= m_nThreadNum; nThread++)
    {
        m_unitBuffers[nThread]->OnImagingConditionChanged(nVisionType, maxImageSize.m_w, maxImageSize.m_h);
    }

    return TRUE;
}

Ipvm::Rect32s VisionMainTR::GetReviewRegion(bool isManualInsp, VisionUnit& visionUnit)
{
    Ipvm::Rect32s totalRegion(
        0, 0, visionUnit.getImageLotInsp().GetImageSizeX(), visionUnit.getImageLotInsp().GetImageSizeY());
    Ipvm::Rect32s reviewRegion = totalRegion;
    //Body Align 정보 가져오기

    long nDataNum = 0;
    auto* pData = visionUnit.GetVisionDebugInfo(
        visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);

    if (pData != nullptr && nDataNum > 0)
    {
        auto* edgeAlignResult = (VisionAlignResult*)pData;
        auto margin = max(0, GetImageOption(isManualInsp).m_nReviewImageMargin);

        reviewRegion = edgeAlignResult->getBodyRect32s();
        reviewRegion.InflateRect(margin, margin);
        reviewRegion &= Ipvm::Rect32s(totalRegion);
    }

    return reviewRegion;
}

const ImageSaveOptionData& VisionMainTR::GetImageOption(bool isManualInsp) const
{
    if (isManualInsp)
    {
        return m_manualImageSaveOption;
    }

    return m_saveOption;
}

void VisionMainTR::PostExecutionProcessing(bool isManualInsp, VisionUnit& visionUnit, VisionDeviceResult& deviceResult,
    const enSideVisionModule i_eSideVisionModule)
{
    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);
    auto& option = GetImageOption(isManualInsp);
    CString strLogSum = visionUnit.GetLastInspection_Text(TRUE);
    visionUnit.m_reviewColorImageName = _T("");

    const auto& imageInfo = visionUnit.getImageLot().GetInfo();
    const long nInspectionRepeatIndex = visionUnit.getImageLot().GetInspectionRepeatIndex();
    const long nTrayIndex = imageInfo.m_trayIndex;
    const long nSideScanID = visionUnit.getImageLot().GetSideScanID();
    const long nScanID = (bIsSideVision == true) ? nSideScanID : imageInfo.m_scanAreaIndex;
    const long paneIndex = visionUnit.GetCurrentPaneID();

    deviceResult.m_reviewRoi = GetReviewRegion(isManualInsp, visionUnit);

    SetImageSave(
        visionUnit, option, deviceResult.m_reviewRoi, nTrayIndex, nScanID, paneIndex, strLogSum, i_eSideVisionModule);

    SystemConfig::GetInstance().Save_VisionLog_InspectionTimeLog(
        visionUnit.m_threadIndex, nTrayIndex, nScanID, paneIndex, _T("Image Save End"));

    strLogSum.Empty();

    auto visionInspectionResults = visionUnit.GetVisionInspectionResults();

    CString strMajorID = _T("NULL");
    if (visionUnit.IsEnable2DMatrix() == TRUE) //kircheis_20230126
        strMajorID = _T("NOREAD");

    std::vector<CString> vecStrSub2DID(0);
    std::vector<CString> vecStrBaby2DID(0);

    long nDataNum = 0;
    void* pData = visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_2D_MATRIX, _T("2D Matrix Major Result"), nDataNum);
    if (nDataNum > 0 && pData != nullptr)
    {
        CString* pStr2DID = (CString*)pData;
        if (!pStr2DID[0].IsEmpty())
            strMajorID = pStr2DID[0];
    }

    nDataNum = 0;
    pData = visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_2D_MATRIX, _T("2D Matrix Sub Result"), nDataNum);
    if (nDataNum > 0 && pData != nullptr)
    {
        vecStrSub2DID.resize(nDataNum);
        CString* pStr2DID = (CString*)pData;
        for (long nID = 0; nID < nDataNum; nID++)
            vecStrSub2DID[nID] = pStr2DID[nID];
    }

    nDataNum = 0;
    pData = visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_2D_MATRIX, _T("2D Matrix Baby Result"), nDataNum);
    if (nDataNum > 0 && pData != nullptr)
    {
        vecStrBaby2DID.resize(nDataNum);
        CString* pStr2DID = (CString*)pData;
        for (long nID = 0; nID < nDataNum; nID++)
            vecStrBaby2DID[nID] = pStr2DID[nID];
    }

    VisionWarpageShapeResult visionWarpageShapeResult;
    nDataNum = 0;
    pData = visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_BGA_BALL_3D, _T("Warpage Shape Info"), nDataNum);
    if (nDataNum > 0 && pData != nullptr)
    {
        visionWarpageShapeResult.SetResult(*(VisionWarpageShapeResult*)pData);
    }
    else
    {
        pData = visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_LGA_LAND_3D, _T("Warpage Shape Info"), nDataNum);
        if (nDataNum > 0 && pData != nullptr)
        {
            visionWarpageShapeResult.SetResult(*(VisionWarpageShapeResult*)pData);
        }
    }

    //{{//kircheis_InspLid
    std::vector<VisionWarpageShapeResult> vecLidWarpageShapeResult;
    nDataNum = 0;
    pData = visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_LID_3D, _T("Lid Warpage Shape Info"), nDataNum);
    if (nDataNum > 0 && pData != nullptr)
    {
        vecLidWarpageShapeResult.resize(nDataNum);
        VisionWarpageShapeResult* pWarpageShapeInfo = (VisionWarpageShapeResult*)pData;
        for (long nLid = 0; nLid < nDataNum; nLid++)
            vecLidWarpageShapeResult[nLid].SetResult(pWarpageShapeInfo[nLid]);
    }
    //}}

    // Host에 넘겨줄 Pane ID를 계산한다...
    long pocketID = -1;
    if (paneIndex >= 0 && paneIndex < (long)visionUnit.getInspectionAreaInfo().m_unitIndexList.size())
    {
        pocketID = visionUnit.getInspectionAreaInfo().m_unitIndexList[paneIndex];
    }

    VisionAlignResult* alignResult = nullptr;
    pData = visionUnit.GetVisionDebugInfo(
        visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);

    if (pData != nullptr && nDataNum > 0)
    {
        alignResult = (VisionAlignResult*)pData;
    }

    if (alignResult && SystemConfig::GetInstance().m_bIsAnalysisResult)
    {
        deviceResult.m_analysis->SetUse(true);
        deviceResult.m_analysis->m_packageOffsetX = alignResult->m_analysis_packageOffsetX_um;
        deviceResult.m_analysis->m_packageOffsetY = alignResult->m_analysis_packageOffsetY_um;

        long h_count = (long)alignResult->m_analysis_histogramZ.size();
        long h_c = h_count / 2;

        for (long index = 0; index < h_count; index++)
        {
            float z = (float)(index - h_c) * 10.f; // 10um 단위로 저장하고 있음
            deviceResult.m_analysis->AccumulateZ(z, alignResult->m_analysis_histogramZ[index]);
        }
    }

    if (m_nPassDeviceSaveCount > 0)
    {
        if (visionUnit.GetInspTotalResult() == PASS && m_nDevicePassCount == 0
            || m_nDevicePassCount % m_nPassDeviceSaveCount == 0)
        {
            //TestSetup();
            CString strDummyFilePath("");
            InlineImageSave_AllFrame(visionUnit, option //m_saveOption
                ,
                m_vecstr2DReview_AllFrameFilePath, strDummyFilePath, m_vecstr2DRaw_AllFrameFilePath);

            m_nDevicePassCount++;

            m_bSend2DAllFrame = true;

            strDummyFilePath.Empty();
        }
        else if (visionUnit.GetInspTotalResult() == PASS)
            m_nDevicePassCount++;
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////// TR Pocket Center Position ////////////////////////////////////////////////////////////
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    float fPocketCenterShifhtY = -999.f;
    if (SystemConfig::GetInstance().IsVisionTypeTR() == TRUE)
    {
        pData = visionUnit.GetVisionDebugInfo(
            _VISION_INSP_GUID_CARRIER_TAPE_ALIGN, _DEBUG_INFO_CTA_POCKET_ROUGH_ALIGN_CENTER_GAP_Y, nDataNum);
        if (nDataNum > 0 && pData != nullptr)
        {
            fPocketCenterShifhtY = *(float*)pData;
        }
    }
    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////////////////////////////////

    deviceResult.SetInspectResults(visionUnit.m_threadIndex, nTrayIndex, nInspectionRepeatIndex, pocketID, nScanID,
        paneIndex, visionUnit.GetInspTotalResult(), visionUnit.m_reviewColorImageName, visionInspectionResults,
        strMajorID, vecStrSub2DID, vecStrBaby2DID, visionWarpageShapeResult, vecLidWarpageShapeResult,
        fPocketCenterShifhtY); //kircheis_InspLid

    if (deviceResult.m_nTotalResult == NOT_MEASURED)
    {
        deviceResult.m_nTotalResult = INVALID;
    }

    DevelopmentLog::AddLog(DevelopmentLog::Type::Thread,
        _T("VisionMainTR::PostExecutionProcessing ScanIndex %d, PaneIndex %d Pocket %d, Result %d"), nScanID, paneIndex,
        pocketID, deviceResult.m_nTotalResult);

    strMajorID.Empty();

    for (int nIdx = 0; nIdx < vecStrSub2DID.size(); nIdx++)
    {
        vecStrSub2DID[nIdx].Empty();
    }
    for (int nIdx = 0; nIdx < vecStrBaby2DID.size(); nIdx++)
    {
        vecStrBaby2DID[nIdx].Empty();
    }
}

void VisionMainTR::SaveSystemInfo()
{
    CString strZipPath; //	압축 파일이 저장될 Path
    CString strZipFileName; //	압축 파일명
    CString strRootPathName; //	압축할 파일의 Path
    CString strExt; //	.zip을 찾을 문자

    long nIndex;

    strZipPath.Format(_T("D:\\Review Image\\SystemInfo\\"));
    strZipFileName.Format(_T("%sSystemInfo.zip"), LPCTSTR(strZipPath));
    strRootPathName.Format(_T("%s"), LPCTSTR(DynamicSystemPath::get(DefineFolder::System)));

    Ipvm::CreateDirectories(strZipPath);

    nIndex = strZipFileName.ReverseFind(_T('.'));
    strExt = strZipFileName.Right(strZipFileName.GetLength() - nIndex - 1);

    if (strExt.CompareNoCase(_T("zip")) == 0)
    {
        Ipvm::Zipper zipFile(strZipFileName, strRootPathName, Ipvm::ZipperMethod::e_deflate);

        if (zipFile.GetStatus() == Ipvm::ZipperStatus::e_noError)
        {
            zipFile.AddFiles(strRootPathName, L"*csv", false, false, -1);
            zipFile.AddFile(strRootPathName + _T("System.ini"), false, -1);
        }
    }

    strZipPath.Empty();
    strZipFileName.Empty();
    strRootPathName.Empty();
    strExt.Empty();
}

const InspectionAreaInfo& VisionMainTR::getInspectionAreaInfo()
{
    return GetPrimaryVisionUnit().getInspectionAreaInfo();
}

bool VisionMainTR::IsShowVisionPrimaryUI() const
{
    return m_pVisionPrimaryUI && m_pVisionPrimaryUI->IsWindowVisible();
}

void VisionMainTR::TestSetup()
{
    m_saveOption.m_bisSaveReviewImage = TRUE;
    m_saveOption.m_bisSaveCroppingImage = TRUE;
    m_saveOption.m_vecRawImageSaveOption.clear();
    m_saveOption.m_vecRawImageSaveOption.push_back(ImageSaveOption::RawImage_AllPackages);
    m_saveOption.m_bReviewImageOption_Pass = TRUE;
    m_saveOption.m_bReviewImageOption_Reject = TRUE;
    m_saveOption.m_bReviewImageOption_Marginal = FALSE;
    m_saveOption.m_nReviewImageROI = ImageSaveOption::ReviewROI_Insert;
    m_saveOption.m_bReviewImageCombined = TRUE;
    m_saveOption.m_nReviewImageQuality = 30;
    m_saveOption.m_nReviewImageMargin = 100;
    m_saveOption.m_nCroppingImageSaveOption = ImageSaveOption::CroppingImage_NotPass;
    m_saveOption.m_nCroppingImageROI = ImageSaveOption::CroppingROI_Insert;
    m_saveOption.m_nCroppingImageFrame_Ball = ImageSaveOption::CroppingFrame_NotSave;
    m_saveOption.m_nCroppingImageFrame_Land = ImageSaveOption::CroppingFrame_Inspection;
    m_saveOption.m_nCroppingImageFrame_Comp = ImageSaveOption::CroppingFrame_Frame01;
    m_saveOption.m_nCroppingImageFrame_Surface = ImageSaveOption::CroppingFrame_Color;
    m_saveOption.m_nCroppingImageQuality = 30;
}

void VisionMainTR::SetImageSaveOption(ImageSaveOptionData i_save)
{
    m_saveOption.m_bisSaveReviewImage = i_save.m_bisSaveReviewImage;
    ;
    m_saveOption.m_bisSaveCroppingImage = i_save.m_bisSaveCroppingImage;
    m_saveOption.m_vecRawImageSaveOption = i_save.m_vecRawImageSaveOption;
    m_saveOption.m_bReviewImageOption_Pass = i_save.m_bReviewImageOption_Pass;
    m_saveOption.m_bReviewImageOption_Reject = i_save.m_bReviewImageOption_Reject;
    m_saveOption.m_bReviewImageOption_Marginal = i_save.m_bReviewImageOption_Marginal;
    m_saveOption.m_nReviewImageROI = i_save.m_nReviewImageROI;
    m_saveOption.m_bReviewImageCombined = i_save.m_bReviewImageCombined;
    m_saveOption.m_nReviewImageQuality = i_save.m_nReviewImageQuality;
    m_saveOption.m_nReviewImageMargin = i_save.m_nReviewImageMargin;
    m_saveOption.m_nCroppingImageSaveOption = i_save.m_nCroppingImageSaveOption;
    m_saveOption.m_nCroppingImageROI = i_save.m_nCroppingImageROI;
    m_saveOption.m_nCroppingImageFrame_Ball = i_save.m_nCroppingImageFrame_Ball;
    m_saveOption.m_nCroppingImageFrame_Land = i_save.m_nCroppingImageFrame_Land;
    m_saveOption.m_nCroppingImageFrame_Comp = i_save.m_nCroppingImageFrame_Comp;
    m_saveOption.m_nCroppingImageFrame_Surface = i_save.m_nCroppingImageFrame_Surface;
    m_saveOption.m_nCroppingImageQuality = i_save.m_nCroppingImageQuality;
}

void VisionMainTR::SetImageSave(VisionUnit& visionUnit, const ImageSaveOptionData& option,
    const Ipvm::Rect32s& reviewRegion, const long i_nTrayIndex, const long i_scanID, const long i_paneID,
    const CString i_strLogData, const enSideVisionModule i_eSideVisionModule)
{
    //TestSetup();

    CString strRealRawImagePath;
    CString strRealRawJobPath;
    CString strRealReviewImagePath;
    CString strRealCroppingImagePath;

    //이미지 저장 폴더 생성

    //strRealRawImagePath = CString(RAW_IMAGE_DIRECTORY);
    //strRealRawJobPath = CString(RAW_IMAGE_DIRECTORY);
    //strRealReviewImagePath = CString(REVIEW_IMAGE_DIRECTORY);
    //strRealCroppingImagePath = CString(CROPPING_IMAGE_DIRECTORY);

    //::CreateDirectory(strRealRawImagePath, NULL);
    //::CreateDirectory(strRealReviewImagePath, NULL);
    //::CreateDirectory(strRealCroppingImagePath, NULL);

    //if (!CheckFreeDiskForImageSave(true))
    //{
    //	// 저장공간 부족
    //	return;
    //}

    //PathToSaveImage(visionUnit, i_nTrayIndex, i_scanID, i_paneID, &strRealRawImagePath, &strRealRawJobPath, &strRealReviewImagePath, &strRealCroppingImagePath);

    if (CreateImageSave_Directory(visionUnit, i_nTrayIndex, i_scanID, i_paneID, strRealRawImagePath,
            strRealReviewImagePath, strRealCroppingImagePath)
        != ImageSaveStatus::ImageSaveStatus_CreateDirectory_Success)
    {
        SaveReviewImage_DeubgLOG(visionUnit, _T(""), _T(""), _T(""),
            eReviewimageSaveLOGCategory::eReviewimageSaveLOGCatrgory_InsufficientDiskCapacity_D, false);
        return;
    }

    InlineImageSave_Review(visionUnit, option, reviewRegion, strRealReviewImagePath, i_strLogData, i_eSideVisionModule);
    InlineImageSave_Cropping(visionUnit, option, strRealCroppingImagePath, i_eSideVisionModule);

    BOOL isRawSave = RawImageSaveResult(option, visionUnit.GetInspTotalResult());

    if (isRawSave && visionUnit.getImageLot().InlineRawSaveCheck())
    {
        // Raw Image Save은 Raw Image 저장 조건에 한번만이라도 부합된다면
        // 한번만 저장한다. (같은 파일을 중복 저장하는 것 방지)
        CString strDummyRawImageFilePath(_T(""));
        Ipvm::CreateDirectories(LPCTSTR(strRealRawImagePath));
        RawImageSave(visionUnit, strRealRawImagePath, strDummyRawImageFilePath);
    }

    strRealRawImagePath.Empty();
    strRealRawJobPath.Empty();
    strRealReviewImagePath.Empty();
    strRealCroppingImagePath.Empty();
}

void VisionMainTR::InlineImageSave_Review(VisionUnit& visionUnit, const ImageSaveOptionData& option,
    const Ipvm::Rect32s& reviewRegion, LPCTSTR realReviewImagePath, LPCTSTR logData,
    const enSideVisionModule i_eSideVisionModule)
{
    if (!option.m_bisSaveReviewImage)
    {
        SaveReviewImage_DeubgLOG(visionUnit, _T(""), _T(""), _T(""),
            eReviewimageSaveLOGCategory::eReviewimageSaveLOGCatrgory_SaveOptionDisable, false);
        return;
    }

    const auto& imageInfo = visionUnit.getImageLot().GetInfo();

    const long nTrayIndex = imageInfo.m_trayIndex;
    const long nScanID = imageInfo.m_scanAreaIndex;
    const long paneIndex = visionUnit.GetCurrentPaneID();

    bool isReportSave = false;
    auto& imageLotInsp = visionUnit.getImageLotInsp();

    long frameNum = long(imageLotInsp.m_vecImages[i_eSideVisionModule].size());
    if (frameNum == 0)
    {
        // 저장할 Frame이 없다
        SaveReviewImage_DeubgLOG(visionUnit, _T(""), _T(""), _T(""),
            eReviewimageSaveLOGCategory::eReviewimageSaveLOGCatrgory_SaveFrameEmpty, false);
        return;
    }

    struct SPassFrame
    {
        std::vector<VisionInspectionResult*> m_items;
    };

    DevelopmentLog::AddLog(DevelopmentLog::Type::Thread, _T("Review Save Mode - Start"));

    std::vector<SPassFrame> vecPassFrame;
    vecPassFrame.resize(frameNum);

    long totalResult = visionUnit.GetInspTotalResult();

    // vision Module 별로 체크함
    for (auto* visionModule : visionUnit.m_vecVisionModules)
    {
        auto* inspectModule = dynamic_cast<VisionInspection*>(visionModule);

        if (inspectModule == nullptr)
        {
            // 검사항목이 아니다
            continue;
        }

        CString moduleGuid = inspectModule->m_moduleGuid;
        CString moduleName = inspectModule->m_strModuleName;
        if (moduleName.Find(_T("MSK_")) == 0)
        {
            // Package Size와 Surface Mask는 수집대상이 아니다
            // Package Size는 검사항목인데 왜?
            continue;
        }

        long frameIndex = inspectModule->GetImageFrameIndex(0);

        if (frameIndex < 0)
            frameIndex = 0;
        if (frameIndex >= frameNum)
            frameIndex = 0;

        std::vector<Ipvm::Rect32s> vecrtRejectROI;
        std::vector<Ipvm::Rect32s> vecrtMarginalROI;
        std::map<CString, BOOL> rejectNameMap;
        long moduleResult = NOT_MEASURED;

        for (auto& inspResult : inspectModule->m_resultGroup.m_vecResult)
        {
            long result = inspResult.m_totalResult;
            moduleResult = max(moduleResult, result);

            CString name = inspResult.m_inspName;

            if (result == PASS)
            {
                vecPassFrame[frameIndex].m_items.push_back(&inspResult);
            }
            else
            {
                rejectNameMap[name] = TRUE;
            }

            /* Reject ROI 수집 */
            vecrtRejectROI.insert(
                vecrtRejectROI.end(), inspResult.vecrtRejectROI.begin(), inspResult.vecrtRejectROI.end());

            /* Marginal ROI 수집 */
            vecrtMarginalROI.insert(
                vecrtMarginalROI.end(), inspResult.vecrtMarginalROI.begin(), inspResult.vecrtMarginalROI.end());

            name.Empty();
        }

        if (moduleResult == NOT_MEASURED)
            continue;

        CString rejectNames;
        for (auto& itRejectName : rejectNameMap)
        {
            rejectNames += _T("_") + itRejectName.first;
        }

        CString strSideHeader("");
        if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP)
        {
            switch (i_eSideVisionModule)
            {
                case enSideVisionModule::SIDE_VISIONMODULE_FRONT:
                    strSideHeader = _T("F_");
                    break;
                case enSideVisionModule::SIDE_VISIONMODULE_REAR:
                    strSideHeader = _T("R_");
                    break;
                default:
                    break;
            }
        }

        CString reviewFilePath;
        reviewFilePath.Format(_T("%s%s_T%d_S%d_P%d_%sFrame%d%s.jpg"), realReviewImagePath,
            LPCTSTR(m_lotReviewFileHeader), nTrayIndex, nScanID, paneIndex, (LPCTSTR)strSideHeader, frameIndex + 1,
            LPCTSTR(rejectNames));
        //reviewFilePath.Format(_T("%s%s_T%d_S%d_P%d_Frame%d%s.jpg"),
        //	realReviewImagePath,
        //	LPCTSTR(m_lotReviewFileHeader),
        //	nTrayIndex, nScanID, paneIndex,
        //	frameIndex + 1,
        //	LPCTSTR(rejectNames));

        bool isReviewSave = false;

        if (moduleResult == MARGINAL && option.m_bReviewImageOption_Marginal)
            isReviewSave = true;
        if (moduleResult != MARGINAL && moduleResult != PASS && option.m_bReviewImageOption_Reject)
            isReviewSave = true;

        if (isReviewSave)
        {
            Ipvm::CreateDirectories(realReviewImagePath);

            ImageSave_Review(visionUnit, option, reviewRegion, moduleGuid, moduleName, frameIndex, vecrtRejectROI,
                vecrtMarginalROI, reviewFilePath, rejectNames, i_eSideVisionModule);

            // PASS가 아닌놈은 모두 이 Review이미지를 가르킨다
            for (auto& inspResult : inspectModule->m_resultGroup.m_vecResult)
            {
                long result = inspResult.m_totalResult;
                if (result != PASS)
                {
                    inspResult.m_reviewImageFileName = reviewFilePath;
                }
            }

            isReportSave = true;

            moduleGuid.Empty();
            moduleName.Empty();

            rejectNames.Empty();
            reviewFilePath.Empty();
        }
    }

    if (option.m_bReviewImageOption_Pass)
    {
        // Side Vision일 경우 Review Image Header 추가 - 2024.06.27_JHB
        CString strSideHeader("");
        if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP)
        {
            switch (i_eSideVisionModule)
            {
                case enSideVisionModule::SIDE_VISIONMODULE_FRONT:
                    strSideHeader = _T("F_");
                    break;
                case enSideVisionModule::SIDE_VISIONMODULE_REAR:
                    strSideHeader = _T("R_");
                    break;
                default:
                    break;
            }
        }
        ////////////////////////////////////////////////////////////

        for (long frameIndex = 0; frameIndex < frameNum; frameIndex++)
        {
            if (totalResult != PASS && vecPassFrame[frameIndex].m_items.size() == 0)
            {
                // Total Result가 PASS 가 아닐 때는
                // PASS 된 검사항목이 사용한 Frame만 저장한다
                continue;
            }

            // Pass를 저장하게 했으면 한놈만 만들어서 다 이것을 가르키게 한다

            // Side Vision일 경우 Review File Path 추가 - 2024.06.27_JHB
            CString reviewFilePath;
            reviewFilePath.Format(_T("%s%s_T%d_S%d_P%d_%sFrame%d.jpg"), realReviewImagePath,
                LPCTSTR(m_lotReviewFileHeader), nTrayIndex, nScanID, paneIndex, (LPCTSTR)strSideHeader, frameIndex + 1);
            //reviewFilePath.Format(_T("%s%s_T%d_S%d_P%d_Frame%d%s.jpg"),
            //	realReviewImagePath,
            //	LPCTSTR(m_lotReviewFileHeader),
            //	nTrayIndex, nScanID, paneIndex,
            //	frameIndex + 1,
            //	LPCTSTR(rejectNames));

            //Ipvm::CreateDirectories(realReviewImagePath);

            ImageSave_Review(visionUnit, option, reviewRegion, _T(""), _T(""), frameIndex, std::vector<Ipvm::Rect32s>(),
                std::vector<Ipvm::Rect32s>(), reviewFilePath, _T(""), i_eSideVisionModule);

            for (auto* inspResult : vecPassFrame[frameIndex].m_items)
            {
                inspResult->m_reviewImageFileName = reviewFilePath;
            }

            isReportSave = true;

            reviewFilePath.Empty();
        }
    }

    if (isReportSave)
    {
        SYSTEMTIME Time;
        Ipvm::TimeCheck().GetLocalTime(Time);

        CString logText;
        logText.Format(_T("%04d/%02d/%02d %02d:%02d:%02d\r\n"), Time.wYear, Time.wMonth, Time.wDay, Time.wHour,
            Time.wMinute, Time.wSecond);

        CString logDataText = logData;
        if (logDataText == _T(""))
        {
            logText += _T("Empty");
        }
        else
        {
            logText += logDataText;
        }

        ResultTextSave(logText, nTrayIndex, nScanID, paneIndex, realReviewImagePath);

        logText.Empty();
        logDataText.Empty();
    }

    DevelopmentLog::AddLog(DevelopmentLog::Type::Thread, _T("Review Save Mode - End"));
}

void VisionMainTR::InlineImageSave_Cropping(VisionUnit& visionUnit, const ImageSaveOptionData& option,
    LPCTSTR croppingImagePath, const enSideVisionModule i_eSideVisionModule)
{
    if (!option.m_bisSaveCroppingImage)
        return;

    auto& imageLotInsp = visionUnit.getImageLotInsp();
    long frameNum = long(imageLotInsp.m_vecImages[i_eSideVisionModule].size());
    if (frameNum == 0)
        return; // 저장할 Frame이 없다

    DevelopmentLog::AddLog(DevelopmentLog::Type::Thread, _T("Cropping Save Mode - Start"));

    // vision Module 별로 체크함
    for (auto* visionModule : visionUnit.m_vecVisionModules)
    {
        auto* inspectModule = dynamic_cast<VisionInspection*>(visionModule);

        if (inspectModule == nullptr)
        {
            // 검사항목이 아니다
            continue;
        }

        CString moduleGuid = inspectModule->m_moduleGuid;
        CString moduleName = inspectModule->m_strModuleName;
        if (moduleName.Find(_T("MSK_")) == 0)
        {
            // Package Size와 Surface Mask는 수집대상이 아니다
            // Package Size는 검사항목인데 왜?
            continue;
        }

        long frameIndex = inspectModule->GetImageFrameIndex(0);

        if (frameIndex < 0)
            frameIndex = 0;
        if (frameIndex >= frameNum)
            frameIndex = 0;

        CroppingImageSave(visionUnit, option, moduleGuid, frameIndex, croppingImagePath, i_eSideVisionModule);

        moduleGuid.Empty();
        moduleName.Empty();
    }

    DevelopmentLog::AddLog(DevelopmentLog::Type::Thread, _T("Cropping Save Mode - End"));
}

BOOL VisionMainTR::RawImageSaveResult(
    const ImageSaveOptionData& option, const long i_nTotalResult, const bool i_bAllFrameSaveMode)
{
    if (i_bAllFrameSaveMode == true)
        return TRUE;

    BOOL bResult = FALSE;

    for (int i = 0; i < (long)(option.m_vecRawImageSaveOption.size()); i++)
    {
        if (option.m_vecRawImageSaveOption[i] == ConvertResult(i_nTotalResult)
            || option.m_vecRawImageSaveOption[i] == ImageSaveOption::RawImage_AllPackages)
        {
            bResult = TRUE;

            break;
        }
    }

    return bResult;
}

long VisionMainTR::ConvertResult(const long i_nTotalResult)
{
    if (i_nTotalResult == PASS)
        return ImageSaveOption::RawImage_Pass;
    else if (i_nTotalResult == MARGINAL)
        return ImageSaveOption::RawImage_Marginal;
    else if (i_nTotalResult == REJECT)
        return ImageSaveOption::RawImage_Reject;
    else if (i_nTotalResult == INVALID)
        return ImageSaveOption::RawImage_Invalid;
    else if (i_nTotalResult == EMPTY)
        return ImageSaveOption::RawImage_Empty;
    else
        return ImageSaveOption::RawImage_End;
}

void VisionMainTR::RawImageSave(
    VisionUnit& visionUnit, const CString i_strImagePath, CString& o_strSaveRawImageFilePath)
{
    visionUnit.getImageLot().SaveTo(i_strImagePath, o_strSaveRawImageFilePath);
}

void VisionMainTR::ResultTextSave(
    const CString i_strLogData, const long i_nTrayIndex, const long i_scanID, const long i_paneID, LPCTSTR i_path)
{
    CFile fp;
    CString strTotalPath;
    strTotalPath.Format(_T("%s%s_T%d_S%d_P%d_Package Report.txt"), i_path, LPCTSTR(m_lotReviewFileHeader), i_nTrayIndex,
        i_scanID, i_paneID);

    if (fp.Open(strTotalPath, CFile::modeCreate | CFile::modeWrite, NULL))
    {
        fp.Write(i_strLogData, i_strLogData.GetLength() * sizeof(TCHAR));
        fp.Close();
    }

    strTotalPath.Empty();
}

BOOL VisionMainTR::ImageSave_Review(VisionUnit& visionUnit, const ImageSaveOptionData& option,
    const Ipvm::Rect32s& reviewRegion, LPCTSTR moduleGuid, LPCTSTR moduleName, const long frameIndex,
    const std::vector<Ipvm::Rect32s>& i_vecrtRejectROI, const std::vector<Ipvm::Rect32s>& i_vecrtMarginalROI,
    LPCTSTR imageFilePath, CString i_strRejectNames, const enSideVisionModule i_eSideVisionModule)
{
    auto& imageLotInsp = visionUnit.getImageLotInsp();
    long nImageSizeX = (imageLotInsp.m_vecImages[i_eSideVisionModule])[frameIndex].GetSizeX();
    long nImageSizeY = (imageLotInsp.m_vecImages[i_eSideVisionModule])[frameIndex].GetSizeY();
    Ipvm::Image8u OriginalImage(nImageSizeX, nImageSizeY);

    Ipvm::Image8u3 Image(nImageSizeX, nImageSizeY);
    Ipvm::ImageProcessing::Copy(
        imageLotInsp.m_vecImages[i_eSideVisionModule][frameIndex], Ipvm::Rect32s(OriginalImage), OriginalImage);

    //BOOL isColor = FALSE;

    Ipvm::Image8u3 Image_Color_ReviewBoth(nImageSizeX, nImageSizeY);
    BOOL isColor_ReviewBoth = FALSE;
    CString strColorImageName_ReviewBoth;

    if (option.m_bReviewImageCombined && !CString(moduleName).IsEmpty())
    {
        if (auto* processing = visionUnit.GetVisionProcessingByGuid(moduleGuid))
        //if (auto* processing = visionUnit.GetVisionProcessingByGuid(moduleName))
        {
            ImageProcPara ImageCombinedPara(*processing);

            if (visionUnit.GetParaData(moduleName, ImageCombinedPara))
            {
                Ipvm::Image8u image = (imageLotInsp.m_vecImages[i_eSideVisionModule])[frameIndex];
                if (image.GetMem() == nullptr)
                {
                    SaveReviewImage_DeubgLOG(visionUnit, i_strRejectNames, moduleName, _T(""),
                        eReviewimageSaveLOGCategory::eReviewimageSaveLOGCatrgory_Combine_Insp_image_Empty, true);
                    return FALSE;
                }

                Ipvm::Rect32s rtProcessingROI(0, 0, OriginalImage.GetSizeX(), OriginalImage.GetSizeY());

                if (!CippModules::GrayImageProcessingManage(*(visionUnit.m_VisionReusableMemory), &image, false,
                        rtProcessingROI, ImageCombinedPara, OriginalImage))
                {
                    SaveReviewImage_DeubgLOG(visionUnit, i_strRejectNames, moduleName, _T(""),
                        eReviewimageSaveLOGCategory::eReviewimageSaveLOGCatrgory_Combine_ImageCombine_Fail, true);
                    return FALSE;
                }
            }
        }
    }

    // Gray -> Color
    Ipvm::ImageProcessing::ConvertGrayToBGR(OriginalImage, Ipvm::Rect32s(OriginalImage), Image);

    if (option.m_nReviewImageROI == ImageSaveOption::ReviewROI_Insert)
    {
        for (long n = 0; n < (long)i_vecrtRejectROI.size(); n++)
        {
            DrawRectangle(Image, i_vecrtRejectROI[n], RGB(255, 0, 0), 3);
            /*Color일땐 반전으로*/
            DrawRectangle(Image, i_vecrtRejectROI[n], RGB(255, 0, 0), 4);
            DrawRectangle(Image, i_vecrtRejectROI[n], RGB(255, 0, 0), 5);
        }

        for (long n = 0; n < (long)i_vecrtMarginalROI.size(); n++)
        {
            DrawRectangle(Image, i_vecrtMarginalROI[n], RGB(243, 157, 58), 3);
            /*Color일땐 반전으로*/
            DrawRectangle(Image, i_vecrtMarginalROI[n], RGB(243, 157, 58), 4);
            DrawRectangle(Image, i_vecrtMarginalROI[n], RGB(243, 157, 58), 5);
        }
    }

    //Ipvm::Image8u3(Image, reviewRegion).SaveJpeg(imageFilePath, option.m_nReviewImageQuality);
    Ipvm::Status ReviewimageStatus
        = Ipvm::ImageFile::SaveAsJpeg(Ipvm::Image8u3(Image, reviewRegion), imageFilePath, option.m_nReviewImageQuality);

    if (ReviewimageStatus != Ipvm::Status::e_ok)
        SaveReviewImage_DeubgLOG(visionUnit, i_strRejectNames, moduleName, Ipvm::ToString(ReviewimageStatus),
            eReviewimageSaveLOGCategory::eReviewimageSaveLOGCatrgory_Review_ImageSave_NOT_SUCCESS, true);

    if (m_pMessageSocket->IsConnected())
    {
        //if ((SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_2D_INSP || SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP) && m_illum2D.m_2D_colorFrame)
        if (SystemConfig::GetInstance().IsVisionType2DorSide() == true && m_illum2D.m_2D_colorFrame)
        {
            auto& RedImage = imageLotInsp.m_vecImages[i_eSideVisionModule][m_illum2D.m_2D_colorFrameIndex_red];
            auto& GreenImage = imageLotInsp.m_vecImages[i_eSideVisionModule][m_illum2D.m_2D_colorFrameIndex_green];
            auto& BlueImage = imageLotInsp.m_vecImages[i_eSideVisionModule][m_illum2D.m_2D_colorFrameIndex_blue];

            Ipvm::ImageProcessing::CombineRGB(
                RedImage, GreenImage, BlueImage, Ipvm::Rect32s(RedImage), Image_Color_ReviewBoth);
            isColor_ReviewBoth = TRUE;
        }
        else if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP)
        {
            const long tableLength = 256;

            std::vector<Ipvm::Point8u3> tableColor(tableLength);

            Ipvm::ColorMap::Make(Ipvm::ColorMapIndex::e_rainbow, tableLength - 1, &tableColor[1]);
            tableColor[0] = {0, 0, 0};

            for (int y = 0; y < nImageSizeY; y++)
            {
                const auto* gray = OriginalImage.GetMem(0, y);
                auto* color = Image_Color_ReviewBoth.GetMem(0, y);

                for (int x = 0; x < nImageSizeX; x++) // Gray -> Color
                {
                    color[x] = tableColor[gray[x]];
                }
            }
        }
        else
        {
            return TRUE;
        }

        CString imageName;
        imageName.Format(_T("%s"), imageFilePath);
        int findIdx = imageName.Find(_T("Frame"));
        strColorImageName_ReviewBoth.Format(_T("%sCOLOR.jpg"), LPCTSTR(imageName.Left(findIdx)));

        if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(strColorImageName_ReviewBoth)
            && GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            if (SystemConfig::GetInstance().IsVisionType2DorSide() == true)
            {
                if (isColor_ReviewBoth)
                    Ipvm::ImageFile::SaveAsJpeg(Ipvm::Image8u3(Image_Color_ReviewBoth, reviewRegion),
                        strColorImageName_ReviewBoth, option.m_nReviewImageQuality);
            }
            else
                Ipvm::ImageFile::SaveAsJpeg(Ipvm::Image8u3(Image_Color_ReviewBoth, reviewRegion),
                    strColorImageName_ReviewBoth, option.m_nReviewImageQuality);

            visionUnit.m_reviewColorImageName = strColorImageName_ReviewBoth;
        }
        imageName.Empty();
    }

    strColorImageName_ReviewBoth.Empty();

    return TRUE;
}

BOOL VisionMainTR::ImageSave_Review_AllFrame_2D(VisionUnit& visionUnit, const ImageSaveOptionData& option,
    LPCTSTR moduleGuid, LPCTSTR moduleName, LPCTSTR imageFilePath,
    std::vector<CString>& o_vecstr2DReviewAll_FrameFilePath, CString& o_str2DReview_AllFrameColorImageFilePath,
    const enSideVisionModule i_eSideVisionModule)
{
    UNREFERENCED_PARAMETER(moduleGuid);
    UNREFERENCED_PARAMETER(moduleName);

    auto& imageLotInsp = visionUnit.getImageLotInsp();

    CString strCurrentVisionType = SystemConfig::GetInstance().m_strVisionInfo;

    for (long nAllFrameidx = 0; nAllFrameidx < imageLotInsp.m_vecImages[i_eSideVisionModule].size(); nAllFrameidx++)
    {
        long nImageSizeX = (imageLotInsp.m_vecImages[i_eSideVisionModule])[nAllFrameidx].GetSizeX();
        long nImageSizeY = (imageLotInsp.m_vecImages[i_eSideVisionModule])[nAllFrameidx].GetSizeY();
        Ipvm::Image8u OriginalImage(nImageSizeX, nImageSizeY);
        Ipvm::ImageProcessing::Copy(
            imageLotInsp.m_vecImages[i_eSideVisionModule][nAllFrameidx], Ipvm::Rect32s(OriginalImage), OriginalImage);

        CString strAllFrameFilePath("");
        strAllFrameFilePath.Format(
            _T("%s_%s_Pass_Frame%d.jpg"), imageFilePath, (LPCTSTR)strCurrentVisionType, nAllFrameidx + 1);

        Ipvm::ImageFile::SaveAsJpeg(OriginalImage, strAllFrameFilePath, option.m_nReviewImageQuality);

        o_vecstr2DReviewAll_FrameFilePath.push_back(strAllFrameFilePath);

        strAllFrameFilePath.Empty();

        //Combine과 Color는 별도의 함수로 처리하자[요청사항이 있을수도 있으니] 협의이후 구현예정
        //if (option.m_bReviewImageCombined && !CString(moduleName).IsEmpty() || !CString(moduleGuid).IsEmpty())
        //{
        //	SaveCombinedImage_2D(visionUnit
        //						,moduleGuid ,moduleName
        //						,imageFilePath
        //						,option.m_nReviewImageQuality);
        //}
    }

    if (m_pMessageSocket->IsConnected())
    {
        CString strColorImageFilePath("");
        strColorImageFilePath.Format(_T("%s_%s_Pass_Frame"), imageFilePath, (LPCTSTR)strCurrentVisionType);
        if (SystemConfig::GetInstance().IsVisionType2DorSide() == true && m_illum2D.m_2D_colorFrame)
            SaveColorImage_2D(visionUnit, m_illum2D.m_2D_colorFrameIndex_red, m_illum2D.m_2D_colorFrameIndex_green,
                m_illum2D.m_2D_colorFrameIndex_blue, strColorImageFilePath, option.m_nReviewImageQuality,
                o_str2DReview_AllFrameColorImageFilePath);
        else if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP)
        {
        }
        else
            return TRUE;

        strColorImageFilePath.Empty();
    }

    strCurrentVisionType.Empty();

    return TRUE;
}

bool VisionMainTR::SaveCombinedImage_2D(VisionUnit& visionUnit, const LPCTSTR moduleName, const LPCTSTR moduleGuid,
    LPCTSTR imageFilePath, const long i_nReviewImageQuality, const enSideVisionModule i_eSideVisionModule)
{
    auto& imageLotInsp = visionUnit.getImageLotInsp();

    if (!CString(moduleName).IsEmpty() || !CString(moduleGuid).IsEmpty())
    {
        if (auto* processing = visionUnit.GetVisionProcessingByGuid(moduleGuid))
        {
            ImageProcPara ImageCombinedPara(*processing);

            if (visionUnit.GetParaData(moduleName, ImageCombinedPara))
            {
                long nInspectionframeIndex(-1);
                for (auto* visionModule : visionUnit.m_vecVisionModules)
                {
                    auto* inspectModule = dynamic_cast<VisionInspection*>(visionModule);

                    if (inspectModule->m_moduleGuid == moduleGuid)
                    {
                        nInspectionframeIndex = inspectModule->GetImageFrameIndex(0);
                        break;
                    }
                }

                if (nInspectionframeIndex < 0)
                    return false;

                Ipvm::Image8u Inspectionimage = (imageLotInsp.m_vecImages[i_eSideVisionModule])[nInspectionframeIndex];
                Ipvm::Image8u CombineImage(Inspectionimage.GetSizeX(), Inspectionimage.GetSizeY());

                if (Inspectionimage.GetMem() == nullptr)
                    return false;

                Ipvm::Rect32s rtProcessingROI(0, 0, Inspectionimage.GetSizeX(), Inspectionimage.GetSizeY());

                if (!CippModules::GrayImageProcessingManage(*(visionUnit.m_VisionReusableMemory), &Inspectionimage,
                        false, rtProcessingROI, ImageCombinedPara, CombineImage))
                {
                    return false;
                }

                Ipvm::ImageFile::SaveAsJpeg(CombineImage, imageFilePath,
                    i_nReviewImageQuality); //mc_2020.07.13 Combine은 고려해야한다 FilePath는 Path만 들어오지 나머지는 들어오지 않는다 별도작업 필요
            }
        }
    }

    return true;
}

bool VisionMainTR::SaveColorImage_2D(VisionUnit& visionUnit, const long i_nColorFrameidx_Red,
    const long i_nColorFrameidx_Green, const long i_nColorFrameidx_Blue, LPCTSTR imageFilePath,
    const long i_nReviewImageQuality, CString& o_str2DReview_AllFrameColorImageFilePath,
    const enSideVisionModule i_eSideVisionModule)
{
    auto& imageLotInsp = visionUnit.getImageLotInsp();

    auto& RedImage = imageLotInsp.m_vecImages[i_eSideVisionModule][i_nColorFrameidx_Red];
    auto& GreenImage = imageLotInsp.m_vecImages[i_eSideVisionModule][i_nColorFrameidx_Green];
    auto& BlueImage = imageLotInsp.m_vecImages[i_eSideVisionModule][i_nColorFrameidx_Blue];

    if (RedImage.GetMem() == nullptr || GreenImage.GetMem() == nullptr
        || BlueImage.GetMem() == nullptr) //3개중하나라도 데이터가 없다면 저장하지 않는다
        return false;

    Ipvm::Image8u3 Image_Color_Review(RedImage.GetSizeX(), RedImage.GetSizeY());

    Ipvm::ImageProcessing::CombineRGB(RedImage, GreenImage, BlueImage, Ipvm::Rect32s(RedImage), Image_Color_Review);

    CString strimageName(""), strColorImageName_Review("");
    strimageName.Format(_T("%s"), imageFilePath);
    int findIdx = strimageName.Find(_T("Frame"));
    strColorImageName_Review.Format(_T("%s_COLOR.jpg"), LPCTSTR(strimageName.Left(findIdx)));

    if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(strColorImageName_Review)
        && GetLastError() == ERROR_FILE_NOT_FOUND)
    {
        Ipvm::ImageFile::SaveAsJpeg(Image_Color_Review, strColorImageName_Review, i_nReviewImageQuality);

        o_str2DReview_AllFrameColorImageFilePath = strColorImageName_Review;
    }

    strimageName.Empty();
    strColorImageName_Review.Empty();

    return true;
}

void VisionMainTR::CroppingImageSave(VisionUnit& visionUnit, const ImageSaveOptionData& option, LPCTSTR moduleGuid,
    const long i_nFrameID, const CString i_strImagePath, const enSideVisionModule i_eSideVisionModule)
{
    BOOL b_ColorImage = FALSE;
    long ImageSaveOption;
    long nFrameID = i_nFrameID;

    auto& imageLotInsp = visionUnit.getImageLotInsp();
    long nImageSizeX = imageLotInsp.GetImageSizeX();
    long nImageSizeY = imageLotInsp.GetImageSizeY();
    Ipvm::Image8u3 image(nImageSizeX, nImageSizeY);

    CroppingSetImageFrame(visionUnit, option, moduleGuid, image, nFrameID, &ImageSaveOption);
    if (ImageSaveOption == ImageSaveOption::CroppingFrame_NotSave)
        return;

    if (ImageSaveOption == ImageSaveOption::CroppingFrame_Color)
        b_ColorImage = TRUE;

    auto* inspection = (VisionInspection*)visionUnit.GetVisionProcessing(moduleGuid);
    auto croppingOption = option.m_nCroppingImageSaveOption;

    for (auto& inspResult : inspection->m_resultGroup.m_vecResult)
    {
        for (long n = 0; n < inspResult.m_objectResults.size(); n++)
        {
            auto roi = inspResult.m_objectRects[n];
            if ((roi & Ipvm::Rect32s(image)) != roi)
                continue;

            if ((croppingOption == ImageSaveOption::CroppingImage_All)
                || (croppingOption == ImageSaveOption::CroppingImage_NotPass && inspResult.m_objectResults[n] != PASS)
                || (croppingOption == ImageSaveOption::CroppingImage_Reject && inspResult.m_objectResults[n] == REJECT)
                || (croppingOption == ImageSaveOption::CroppingImage_Marginal
                    && inspResult.m_objectResults[n] == MARGINAL))
            {
                Ipvm::CreateDirectories(LPCTSTR(i_strImagePath));

                CString filePath;
                NameToSaveImageforCropping(inspResult.m_inspName, inspResult.m_resultName, i_strImagePath,
                    inspResult.m_objectNames[n], i_nFrameID, b_ColorImage, &filePath, i_eSideVisionModule);

                roi.InflateRect(FloppingImageInflate, FloppingImageInflate);
                roi &= Ipvm::Rect32s(image);
                Ipvm::Image8u3 CropImage(image, roi);
                Ipvm::ImageFile::SaveAsJpeg(CropImage, filePath, option.m_nCroppingImageQuality);

                filePath.Empty();
            }
        }
    }
}

void VisionMainTR::CroppingSetImageFrame(VisionUnit& visionUnit, const ImageSaveOptionData& option, LPCTSTR moduleGuid,
    Ipvm::Image8u3& io_Image, long i_FrameID, long* ImageSaveOption, const enSideVisionModule i_eSideVisionModule)
{
    long InspSaveOption = ImageSaveOption::CroppingFrame_Inspection;
    long nFrameID = i_FrameID;

    auto& imageLotInsp = visionUnit.getImageLotInsp();
    auto* visionProcessing = visionUnit.GetVisionProcessing(moduleGuid);
    CString inspName = visionProcessing->m_strModuleName;

    if (dynamic_cast<VisionInspectionSurface*>(visionProcessing))
    {
        InspSaveOption = option.m_nCroppingImageFrame_Surface;
    }
    else if (inspName == _VISION_INSP_NAME_BGA_BALL_2D || inspName == _VISION_INSP_NAME_BGA_BALL_3D)
    {
        InspSaveOption = option.m_nCroppingImageFrame_Ball;
    }
    else if (inspName == _VISION_INSP_NAME_LGA_LAND_2D || inspName == _VISION_INSP_NAME_LGA_LAND_3D)
    {
        InspSaveOption = option.m_nCroppingImageFrame_Land;
    }
    else if (inspName == _VISION_INSP_NAME_PASSIVE_2D || inspName == _VISION_INSP_NAME_PASSIVE_3D)
    {
        InspSaveOption = option.m_nCroppingImageFrame_Comp;
    }

    *ImageSaveOption = InspSaveOption;
    long nFrameNum = imageLotInsp.GetImageFrameCount();

    if (InspSaveOption >= ImageSaveOption::CroppingFrame_Frame01)
    {
        nFrameID = InspSaveOption - ImageSaveOption::CroppingFrame_Frame01;

        if (nFrameNum <= nFrameID)
        {
            nFrameID = i_FrameID;
        }
    }

    nFrameID = min(max(0, nFrameID), nFrameNum - 1);
    if (nFrameID < 0)
    {
        DevelopmentLog::AddLog(DevelopmentLog::Type::Warning, _T("Cropping failed"));
        return;
    }

    BOOL isColor = FALSE;

    if (InspSaveOption == ImageSaveOption::CroppingFrame_Color)
    {
        isColor = TRUE;

        if (SystemConfig::GetInstance().IsVisionType2DorSide() == true && m_illum2D.m_2D_colorFrame)
        {
            auto& RedImage = imageLotInsp.m_vecImages[i_eSideVisionModule][m_illum2D.m_2D_colorFrameIndex_red];
            auto& GreenImage = imageLotInsp.m_vecImages[i_eSideVisionModule][m_illum2D.m_2D_colorFrameIndex_green];
            auto& BlueImage = imageLotInsp.m_vecImages[i_eSideVisionModule][m_illum2D.m_2D_colorFrameIndex_blue];

            Ipvm::ImageProcessing::CombineRGB(RedImage, GreenImage, BlueImage, Ipvm::Rect32s(RedImage), io_Image);
        }
        else if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP)
        {
            const long tableLength = 256;

            std::vector<Ipvm::Point8u3> tableColor(tableLength);

            Ipvm::ColorMap::Make(Ipvm::ColorMapIndex::e_rainbow, tableLength - 1, &tableColor[1]);
            tableColor[0] = {0, 0, 0};

            long sizeX = io_Image.GetSizeX();
            long sizeY = io_Image.GetSizeY();

            for (int y = 0; y < sizeY; y++)
            {
                const auto* gray = imageLotInsp.m_vecImages[i_eSideVisionModule][nFrameID].GetMem(0, y);

                auto* color = io_Image.GetMem(0, y);

                for (int x = 0; x < sizeX; x++) // Gray -> Color
                {
                    color[x] = tableColor[gray[x]];
                }
            }
        }
        else
        {
            isColor = FALSE;
        }
    }

    if (!isColor)
    {
        // Gray -> Color
        auto& grayImage = imageLotInsp.m_vecImages[i_eSideVisionModule][nFrameID];
        Ipvm::ImageProcessing::CombineRGB(grayImage, grayImage, grayImage, Ipvm::Rect32s(grayImage), io_Image);
    }

    if (option.m_nCroppingImageROI == ImageSaveOption::CroppingROI_Insert)
        CroppingSetRejectROI(dynamic_cast<VisionInspection*>(visionProcessing), option, io_Image);

    inspName.Empty();
}

void VisionMainTR::CroppingSetRejectROI(
    VisionInspection* inspection, const ImageSaveOptionData& option, Ipvm::Image8u3& io_Image)
{
    if (option.m_nCroppingImageROI == ImageSaveOption::CroppingROI_Not)
        return;

    for (auto& inspResult : inspection->m_resultGroup.m_vecResult)
    {
        const auto& vecrtRejectROI = inspResult.vecrtRejectROI;
        const auto& vecrtMarginalROI = inspResult.vecrtMarginalROI;

        if (option.m_nCroppingImageSaveOption == ImageSaveOption::CroppingImage_NotPass
            || option.m_nCroppingImageSaveOption == ImageSaveOption::CroppingImage_Reject)
        {
            for (auto& defectRoi : vecrtRejectROI)
            {
                DrawRectangle(io_Image, defectRoi, RGB(255, 0, 0), 3);
            }
        }

        if (option.m_nCroppingImageSaveOption == ImageSaveOption::CroppingImage_NotPass
            || option.m_nCroppingImageSaveOption == ImageSaveOption::CroppingImage_Marginal)
        {
            for (auto& defectRoi : vecrtMarginalROI)
            {
                DrawRectangle(io_Image, defectRoi, RGB(255, 0, 0), 3);
            }
        }
    }
}

BOOL VisionMainTR::InspectionResult(const long i_nThreadNum, const long i_nInspMin, const long i_nInspMax,
    const std::vector<VisionInspectionResult*> i_pvecVisonInspResult, long* o_nFrameID, long* o_nResult)
{
    auto& inspectionOrderer = m_inline_grab->getInspectionOrderer();
    auto& visionUnit = (i_nThreadNum >= 0) ? inspectionOrderer.getVisionUnit(i_nThreadNum) : *m_primaryVisionUnit;

    VisionReusableMemory* pVisionUsableMemory;
    BOOL bSaveImage = FALSE;
    CString strInspName;
    CString strModuleName;

    for (long nSize = i_nInspMin; nSize <= i_nInspMax; nSize++)
    {
        *o_nResult = max(*o_nResult, (*(i_pvecVisonInspResult)[nSize]).m_totalResult);

        if ((*(i_pvecVisonInspResult)[nSize]).m_totalResult == EMPTY)
        {
            bSaveImage = TRUE;

            long nVisionProcSize = visionUnit.GetVisionProcessingCount();
            for (long n = 0; n < nVisionProcSize; n++)
            {
                strInspName = (*(i_pvecVisonInspResult)[nSize]).m_inspName;
                strModuleName = visionUnit.GetVisionProcessing(n)->m_strModuleName;
                if (strInspName == strModuleName)
                {
                    *o_nFrameID = visionUnit.GetVisionProcessing(n)->GetImageFrameIndex(0);
                    break;
                }
            }
            break;
        }
        else if ((*(i_pvecVisonInspResult)[nSize]).m_totalResult == INVALID)
        {
            bSaveImage = TRUE;
            *o_nFrameID = 0;

            break;
        }
        else if ((*(i_pvecVisonInspResult)[nSize]).m_totalResult == REJECT
            || (*(i_pvecVisonInspResult)[nSize]).m_totalResult == MARGINAL)
        {
            bSaveImage = TRUE;

            long nVisionProcSize = visionUnit.GetVisionProcessingCount();
            for (long n = 0; n < nVisionProcSize; n++)
            {
                strInspName = (*(i_pvecVisonInspResult)[nSize]).m_inspName;
                strModuleName = visionUnit.GetVisionProcessing(n)->m_strModuleName;
                if (strModuleName == strInspName)
                {
                    pVisionUsableMemory = visionUnit.m_VisionReusableMemory;
                    *o_nFrameID = visionUnit.GetVisionProcessing(n)->GetImageFrameIndex(0);
                }
            }
        }
    }

    if (*o_nResult == PASS)
    {
        bSaveImage = TRUE;

        long nVisionProcSize = visionUnit.GetVisionProcessingCount();
        for (long n = 0; n < nVisionProcSize; n++)
        {
            strInspName = (*(i_pvecVisonInspResult)[i_nInspMin]).m_inspName;
            strModuleName = visionUnit.GetVisionProcessing(n)->m_strModuleName;
            if (strInspName == strModuleName)
            {
                *o_nFrameID = visionUnit.GetVisionProcessing(n)->GetImageFrameIndex(0);
                break;
            }
        }
    }

    strInspName.Empty();
    strModuleName.Empty();

    return bSaveImage;
}

BOOL VisionMainTR::NameToSaveImageforReview(ImageSaveOptionData& option, const long i_nFrameID, const long i_nInspMin,
    const long i_nInspMax, const std::vector<VisionInspectionResult*> i_pvecVisonInspResult,
    CString* o_strSaveImageName)
{
    CString strFileName;
    strFileName.Format(_T("Frame%2d"), i_nFrameID + 1);

    BOOL isPass = TRUE;
    BOOL isInvalid = FALSE;

    for (long nSize = i_nInspMin; nSize <= i_nInspMax; nSize++)
    {
        if (option.m_bReviewImageOption_Reject && (*(i_pvecVisonInspResult)[nSize]).m_totalResult == REJECT)
        {
            strFileName += _T("_");
            if ((*(i_pvecVisonInspResult)[nSize]).m_hostReportCategory == HostReportCategory::SURFACE)
                strFileName += (*(i_pvecVisonInspResult)[nSize]).m_inspName;
            else
                strFileName += (*(i_pvecVisonInspResult)[nSize]).m_resultName;

            isPass &= FALSE;
        }
        else if (option.m_bReviewImageOption_Marginal && (*(i_pvecVisonInspResult)[nSize]).m_totalResult == MARGINAL)
        {
            strFileName += _T("_");
            if ((*(i_pvecVisonInspResult)[nSize]).m_hostReportCategory == HostReportCategory::SURFACE)
                strFileName += (*(i_pvecVisonInspResult)[nSize]).m_inspName;
            else
                strFileName += (*(i_pvecVisonInspResult)[nSize]).m_resultName;

            isPass &= FALSE;
        }
        else if (option.m_bReviewImageOption_Pass && (*(i_pvecVisonInspResult)[nSize]).m_totalResult == PASS)
        {
            isPass &= TRUE;
        }
        else if (option.m_bReviewImageOption_Reject && (*(i_pvecVisonInspResult)[nSize]).m_totalResult == INVALID)
        {
            if (strFileName.Find(_T("INVALID")) == -1)
                strFileName += _T("_") + (*(i_pvecVisonInspResult)[nSize]).m_inspName + _T("_INVALID");

            isInvalid = TRUE;
        }
        else if (option.m_bReviewImageOption_Reject && (*(i_pvecVisonInspResult)[nSize]).m_totalResult == EMPTY)
        {
            if (strFileName.Find(_T("EMPTY")) == -1)
                strFileName += _T("_EMPTY");

            isPass = FALSE;
            isInvalid = FALSE;
        }
    }

    if (isPass || isInvalid)
    {
        if (isPass && !isInvalid)
            strFileName += _T("_PASS.jpg");

        *o_strSaveImageName += strFileName;

        return TRUE;
    }

    *o_strSaveImageName += strFileName + _T(".jpg");

    strFileName.Empty();

    return FALSE;
}

void VisionMainTR::NameToSaveImageforCropping(const CString i_strInspName, const CString i_strResultName,
    const CString i_fileRealPath, const CString i_strObjType, const long i_nFrameID, const BOOL i_bIsColor,
    CString* o_strSaveImageName, const enSideVisionModule i_eSideVisionModule)
{
    CString strFileName;
    CString strCroppingImagePath = i_fileRealPath;
    CString inspName = i_strInspName;

    // 쓸수 없는 문자열이 이름에 포함되어 있다면 바꿔주자
    inspName.Replace(_T("\\"), _T("_"));
    inspName.Replace(_T("/"), _T("_"));

    CString strSideHeader("");
    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP)
    {
        switch (i_eSideVisionModule)
        {
            case enSideVisionModule::SIDE_VISIONMODULE_FRONT:
                strSideHeader = _T("F_");
                break;
            case enSideVisionModule::SIDE_VISIONMODULE_REAR:
                strSideHeader = _T("R_");
                break;
            default:
                break;
        }
    }

    if (i_bIsColor)
    {
        if (i_strResultName == _T("Surface"))
        {
            strCroppingImagePath += i_strResultName + _T("\\");
            if (!PathFileExists(strCroppingImagePath))
                Ipvm::CreateDirectories(LPCTSTR(strCroppingImagePath));

            //strFileName.Format(_T("[%s_%s]_[Color]"), (LPCTSTR)inspName, (LPCTSTR)i_strObjType);
            strFileName.Format(
                _T("[%s%s_%s]_[Color]"), (LPCTSTR)strSideHeader, (LPCTSTR)inspName, (LPCTSTR)i_strObjType);
        }
        else
        {
            strCroppingImagePath += inspName + _T("\\");
            if (!PathFileExists(strCroppingImagePath))
                Ipvm::CreateDirectories(LPCTSTR(strCroppingImagePath));

            //strFileName.Format(_T("[%s]_[Color]"), (LPCTSTR)i_strObjType);
            strFileName.Format(_T("[%s%s]_[Color]"), (LPCTSTR)strSideHeader, (LPCTSTR)i_strObjType);
        }
    }
    else
    {
        if (i_strResultName == _T("Surface"))
        {
            strCroppingImagePath += i_strResultName + _T("\\");
            if (!PathFileExists(strCroppingImagePath))
                Ipvm::CreateDirectories(LPCTSTR(strCroppingImagePath));

            //strFileName.Format(_T("[%s]_[Frame%02d]"), (LPCTSTR)inspName/*, (LPCTSTR)i_strObjType*/, i_nFrameID);
            strFileName.Format(_T("[%s%s]_[Frame%02d]"), (LPCTSTR)strSideHeader,
                (LPCTSTR)inspName /*, (LPCTSTR)i_strObjType*/, i_nFrameID);
        }
        else
        {
            strCroppingImagePath += inspName + _T("\\");
            if (!PathFileExists(strCroppingImagePath))
                Ipvm::CreateDirectories(LPCTSTR(strCroppingImagePath));

            //strFileName.Format(_T("[%s]_[Frame%02d]"), (LPCTSTR)i_strObjType, i_nFrameID);
            strFileName.Format(_T("[%s%s]_[Frame%02d]"), (LPCTSTR)strSideHeader, (LPCTSTR)i_strObjType, i_nFrameID);
        }
    }

    strFileName += _T(".JPG");

    *o_strSaveImageName = strCroppingImagePath + strFileName;

    strFileName.Empty();
    strCroppingImagePath.Empty();
    inspName.Empty();
}

void VisionMainTR::PathToSaveImage(VisionUnit& visionUnit, const long i_nTrayIndex, const long i_nScanIndex,
    long i_nPaneIndex, CString* o_strRawImagePath, CString* o_strRawJabPath, CString* o_strReviewImagePath,
    CString* o_strCroppingImagePath)
{
    auto& imageLot = visionUnit.getImageLot();

    //CString strInlinePath = strRealReviewImagePath; // Image Save 경로를 알아온다.
    CString strRawFolderPath;
    CString strRawJobPath;
    CString strReviewFolderPath;
    CString strCroppingFolderPath;

    CString strFileName;
    CString strFileName_Text;
    CString strJobFileName;
    CString strLotID;
    CString strJobName;
    CString strLotID_RunCount;
    CString strTrayID;
    CString strObjType;
    CString strTimeName; // 오늘 날짜를 알아온다.;

    CString strPackage;

    strTimeName.Format(_T("%04d.%02d.%02d"), CTime::GetCurrentTime().GetYear(), CTime::GetCurrentTime().GetMonth(),
        CTime::GetCurrentTime().GetDay());

    CString strJobSavePath;
    CString strTotalPath;

    strJobName.Format(_T("\\%s\\"), LPCTSTR(m_strJobFileName)); // Job Name을 받아온다.

    if (m_lotID.IsEmpty())
    {
        CString temp;
        strLotID.Format(_T("Manual_Inpsection_")); // Host에서 입력한 LotID를 받아온다.
        strLotID += strTimeName;
        temp.Format(_T("_%02d%02d"), CTime::GetCurrentTime().GetHour(), CTime::GetCurrentTime().GetMinute());
        strLotID += temp;

        temp.Empty();
    }
    else
    {
        strLotID.Format(_T("%s"), LPCTSTR(m_lotID)); // Host에서 입력한 LotID를 받아온다.
    }

    strLotID_RunCount.Format(_T("%s_%d\\"), (LPCTSTR)strLotID, imageLot.GetInfo().m_runTimeIndex);

    strLotID += _T("\\");

    strTrayID.Format(_T("Tray_%02d\\"), i_nTrayIndex);
    strPackage.Format(_T("S%02d_P%02d\\"), i_nScanIndex, i_nPaneIndex);

    strRawFolderPath += strTimeName + strJobName + strLotID;
    strRawJobPath += strTimeName + strJobName;

    // 2018.8.6. Review 및 Cropping 이미지를 각각 한폴더에 모두 담기로 했다
    // Host에서 전송된 이미지와 전송안된 이미지를 관리하는데 그것이 편리해서
    // 그렇게 하기로 했다
    // 2018.8.28.
    // + LotID 뒤에 RunTimeIndex도 빼기로 했다
    // + Cropping Folder는 내부 파일이 구분이 안되서 폴더로 구분하기로 했다

    strReviewFolderPath += strLotID;
    strCroppingFolderPath += strLotID_RunCount + strTrayID + strPackage;

    *o_strRawJabPath += strRawJobPath;
    *o_strRawImagePath += strRawFolderPath;
    *o_strReviewImagePath += strReviewFolderPath;
    *o_strCroppingImagePath += strCroppingFolderPath;

    strRawFolderPath.Empty();
    strRawJobPath.Empty();
    strReviewFolderPath.Empty();
    strCroppingFolderPath.Empty();
    strFileName.Empty();
    strFileName_Text.Empty();
    strJobFileName.Empty();
    strLotID.Empty();
    strJobName.Empty();
    strLotID_RunCount.Empty();
    strTrayID.Empty();
    strObjType.Empty();
    strTimeName.Empty();

    strPackage.Empty();

    strJobSavePath.Empty();
    strTotalPath.Empty();
}

void VisionMainTR::DrawRectangle(Ipvm::Image8u3& image, const Ipvm::Rect32s& roi, COLORREF rgbColor, long inflate)
{
    Ipvm::Rect32s modify = roi;
    modify.InflateRect(inflate, inflate);

    Ipvm::Point8u3 lineValue = {GetBValue(rgbColor), GetGValue(rgbColor), GetRValue(rgbColor)};

    long x1 = max(0, modify.m_left);
    long x2 = min(image.GetSizeX(), modify.m_right);
    long y1 = max(0, modify.m_top);
    long y2 = min(image.GetSizeY(), modify.m_bottom);

    if (modify.m_top >= 0 && modify.m_top < image.GetSizeY())
    {
        // Top Line
        auto* image_y = image.GetMem(0, modify.m_top);
        for (long x = x1; x < x2; x++)
            image_y[x] = lineValue;
    }

    if (modify.m_bottom >= 0 && modify.m_bottom < image.GetSizeY())
    {
        // Bottom Line
        auto* image_y = image.GetMem(0, modify.m_bottom);
        for (long x = x1; x < x2; x++)
            image_y[x] = lineValue;
    }

    if (modify.m_left >= 0 && modify.m_left < image.GetSizeX())
    {
        // Bottom Line
        auto imageWidthBytes = image.GetWidthBytes();
        auto* image_y = image.GetMem(modify.m_left, y1);
        for (long y = y1; y < y2; y++)
        {
            *image_y = lineValue;
            image_y = (Ipvm::Point8u3*)(((BYTE*)image_y) + imageWidthBytes);
        }
    }

    if (modify.m_right >= 0 && modify.m_right < image.GetSizeX())
    {
        // Bottom Line
        auto imageWidthBytes = image.GetWidthBytes();
        auto* image_y = image.GetMem(modify.m_right - 1, y1);
        for (long y = y1; y < y2; y++)
        {
            *image_y = lineValue;
            image_y = (Ipvm::Point8u3*)(((BYTE*)image_y) + imageWidthBytes);
        }
    }
}

BOOL VisionMainTR::GetFreeSpaceCheck(const long i_nFreeSpaceMinGB, LPCTSTR checkDiskPath)
{
    ULARGE_INTEGER avail, total, free;
    avail.QuadPart = 0L;
    total.QuadPart = 0L;
    free.QuadPart = 0L;

    // 해당 Drvie의 디스크 용량 정보를 받아 옴
    GetDiskFreeSpaceEx(checkDiskPath, &avail, &total, &free);

    // GByte 로 표현을 하기 위한 부분
    int freeGB = (int)(free.QuadPart >> 30);

    return freeGB >= i_nFreeSpaceMinGB;
}

void VisionMainTR::CopyImage_From_OtherVisionUnit_Into_PrimaryVisionUnit(long oterhVisionUnitIndex)
{
    if (oterhVisionUnitIndex < 0)
    {
        return;
    }

    if (oterhVisionUnitIndex >= m_nThreadNum)
    {
        ASSERT(!_T("Thread Index 가 이상하다"));
        return;
    }

    auto& inspectionOrderer = m_inline_grab->getInspectionOrderer();
    auto& srcRaw = inspectionOrderer.getVisionUnit(oterhVisionUnitIndex).getImageLot();
    auto& srcInsp = inspectionOrderer.getVisionUnit(oterhVisionUnitIndex).getImageLotInsp();

    GetPrimaryVisionUnit().getImageLot().CopyFrom(srcRaw);
    GetPrimaryVisionUnit().getImageLotInsp().CopyFrom(srcInsp);
}

bool VisionMainTR::CheckFreeDiskForImageSave(bool sendErrorMessageToHost)
{
    if (!GetFreeSpaceCheck(50, _TEXT("D:\\")))
    {
        DevelopmentLog::AddLog(
            DevelopmentLog::Type::Notice, _T("There is not enough capacity to save the image.(Less than 50GB)"));

        if (sendErrorMessageToHost)
        {
            //디스크 D 저장 공간 부족으로 Host에게 전송하기
            CString strMessage = _T("The disk capacity is less than 50 GB. Check Out D Drive Capacity");

            iPIS_Send_ErrorMessageForLog(_T("Storage warning"), strMessage);
            iPIS_Send_ErrorMessageForPopup(strMessage);
        }
    }
    else if (!GetFreeSpaceCheck(20, _TEXT("D:\\")))
    {
        DevelopmentLog::AddLog(
            DevelopmentLog::Type::Warning, _T("There is not enough capacity to save the image.(Less than 20GB)"));

        if (sendErrorMessageToHost)
        {
            //디스크 D 저장 공간 부족으로 Host에게 전송하기
            CString strMessage = _T("Vision did not save the image as the disk capacity was less than 20 GB. Check ")
                                 _T("Out D Drive Capacity");

            iPIS_Send_ErrorMessageForLog(_T("Storage warning"), strMessage);
            iPIS_Send_ErrorMessageForPopup(strMessage); //<- 예전방식!
            //Message 보내는 것에 대하여 확인이 필요합니다...
            strMessage.Empty();
        }

        return false;
    }

    return true;
}

bool VisionMainTR::IsJobTeachEnabled()
{
    return m_pVisionPrimaryUI->IsJobTeachEnabled();
}

bool VisionMainTR::IsAllBatchInspectionThreadsIdle()
{
    return m_inline_grab->isIdleForBatch();
}

long VisionMainTR::PushBatchImage(long batchKey, long paneIndex, LPCTSTR imageFilePath, LPCTSTR batchBarcode)
{
    if (m_nThreadNum < 1)
        return -2;

    if (!m_inline_grab->PushBatchImage(batchKey, imageFilePath, paneIndex, batchBarcode))
    {
        return -1;
    }

    return 0;
}

void VisionMainTR::RegisterCallBack_InspectionEnd(void* userData, FUNC_INSPECTION_END_CALLBACK* callback)
{
    m_inline_grab->getInspectionOrderer().registerCallBack_InspectionEnd(userData, callback);
}

void VisionMainTR::UnRegisterCallBack_InspectionEnd(FUNC_INSPECTION_END_CALLBACK* callback)
{
    m_inline_grab->getInspectionOrderer().unRegisterCallBack_InspectionEnd(callback);
}

VisionUnit& VisionMainTR::GetPrimaryVisionUnit()
{
    return *m_primaryVisionUnit;
}

bool VisionMainTR::CheckIntegrityOfCompMapData() //kircheis_Comp3DHeightBug
{
    auto& primaryUnit = GetPrimaryVisionUnit();
    CPackageSpec* pPackageSpec = primaryUnit.m_pPackageSpec;

    long nTotalCompNum(0), nErrorCompNum(0);
    CString strErrorMSG;
    if (pPackageSpec->m_OriginCompData->CheckIntegrityOfCompMapData(nErrorCompNum, nTotalCompNum) == false)
    {
        strErrorMSG.Format(
            _T("%d data of the component map data (width, length, height) has a problem.\nPlease check the map data."),
            nErrorCompNum);
        iPIS_Send_ErrorMessageForPopup(strErrorMSG);
        return false;
    }

    strErrorMSG.Empty();
    return true;
}

bool VisionMainTR::Init_NGRV_SingleRunInfo()
{
    if (m_primaryVisionUnit == nullptr)
        return false;

    auto& primaryUnit = GetPrimaryVisionUnit();

    long nInspectionNum = (long)primaryUnit.m_vecVisionModules.size();
    for (long nid = 0; nid < nInspectionNum; nid++)
    {
        auto* proc = primaryUnit.m_vecVisionModules[nid];

        if (proc == nullptr)
            return false;

        if (dynamic_cast<VisionProcessingNGRV*>(proc))
        {
            if (((VisionProcessingNGRV*)proc)->m_VisionParaBTM == nullptr)
                return false;
            else
            {
                auto& Para_BTM = ((VisionProcessingNGRV*)proc)->m_VisionParaBTM;
                if (Copy_NGRV_SingleRunInfo(VISION_BTM_2D, primaryUnit.getScale(), Para_BTM->m_defectNumber,
                        primaryUnit.getImageLot().GetImageSizeX(), primaryUnit.getImageLot().GetImageSizeY(),
                        Para_BTM->m_vecDefectName, Para_BTM->m_vecStitchCount, Para_BTM->m_vecAFtype,
                        Para_BTM->m_vecGrabHeightOffset_um, Para_BTM->m_vec2IllumUsage, Para_BTM->m_vecGrabPosROI)
                    == false)
                {
                    return false;
                }
            }

            if (((VisionProcessingNGRV*)proc)->m_VisionParaTOP == nullptr)
                return false;
            else
            {
                auto& Para_TOP = ((VisionProcessingNGRV*)proc)->m_VisionParaTOP;
                if (Copy_NGRV_SingleRunInfo(VISION_TOP_2D, primaryUnit.getScale(), Para_TOP->m_defectNumber,
                        primaryUnit.getImageLot().GetImageSizeX(), primaryUnit.getImageLot().GetImageSizeY(),
                        Para_TOP->m_vecDefectName, Para_TOP->m_vecStitchCount, Para_TOP->m_vecAFtype,
                        Para_TOP->m_vecGrabHeightOffset_um, Para_TOP->m_vec2IllumUsage, Para_TOP->m_vecGrabPosROI)
                    == false)
                {
                    return false;
                }
            }
        }
    }

    return true;
}

bool VisionMainTR::Copy_NGRV_SingleRunInfo(const long& VisionType, const VisionScale& scale, const long& defectNumber,
    const long& ImageSizeX, const long& ImageSizeY, const std::vector<CString>& m_vecDefectName,
    const std::vector<Ipvm::Point32s2>& m_vecStitchCount, const std::vector<long>& m_vecAFtype,
    const std::vector<float>& m_vecGrabHeightOffset_um, const std::vector<std::vector<BOOL>>& m_vec2IllumUsage,
    const std::vector<Ipvm::Rect32s>& m_vecGrabPosROI)
{
    if (ImageSizeX < 0 || ImageSizeY < 0)
        return false;

    auto& primaryUnit = GetPrimaryVisionUnit();

    // 현재까지 편집된 item들을 저장
    // 1. NgrvSingleRunInfo 생성
    std::vector<NgrvOnePointGrabInfo> onePointGrabInfo;
    onePointGrabInfo.clear();
    onePointGrabInfo.resize(defectNumber);

    std::vector<Ipvm::Point32s2> eachCenterPoints;

    // 2. 현재 Setup된 item 정보 복사
    for (long index = 0; index < defectNumber; index++)
    {
        eachCenterPoints.clear();

        // a. Single Run Info 초기화
        onePointGrabInfo[index].Init();

        // b. Defect Name, Image center position, Z-offset
        onePointGrabInfo[index].m_defectPositionName = m_vecDefectName[index];

        // Stitching X/Y 둘 중 하나라도 1 이상이면 각 ROI의 center와 전체 ROI의 center 좌표를 보내준다.
        if (m_vecStitchCount[index].m_x > 1 || m_vecStitchCount[index].m_y > 1)
        {
            long centerX(0), centerY(0);
            long stitchX = m_vecStitchCount[index].m_x;
            long stitchY = m_vecStitchCount[index].m_y;

            for (long nY = 0; nY < stitchY; nY++)
            {
                centerY
                    = m_vecGrabPosROI[index].m_top + (m_vecGrabPosROI[index].Height() / (stitchY * 2) * (nY * 2 + 1));

                for (long nX = 0; nX < stitchX; nX++)
                {
                    long roiIndex = stitchX * nY + nX;
                    centerX = m_vecGrabPosROI[index].m_left
                        + (m_vecGrabPosROI[index].Width() / (stitchX * 2) * (nX * 2 + 1));
                    eachCenterPoints.push_back(Ipvm::Point32s2(centerX, centerY));
                }
            }
            //eachCenterPoints.push_back(m_vecGrabPosROI[index].CenterPoint());
        }
        else // Stitching 안할 경우, 해당 ROI의 center 좌표를 보내준다.
        {
            eachCenterPoints.push_back(
                Ipvm::Point32s2(m_vecGrabPosROI[index].CenterPoint().m_x, m_vecGrabPosROI[index].CenterPoint().m_y));
        }

        for (long nSize = 0; nSize < eachCenterPoints.size(); nSize++)
        {
            // Real Image Center Position in pixel
            Ipvm::Point32r2 imageCenterPoint(ImageSizeX / 2, ImageSizeY / 2);
            // Push back with real value with convert pixel to BCU function
            onePointGrabInfo[index].m_vecPointGrabPosition_um.push_back(
                scale.convert_PixelToBCU(eachCenterPoints[nSize], imageCenterPoint));

            // CenterY 좌표 기준이기 때문에, Y좌표의 최종 결과에 (-)부호를 곱해줘서 극 좌표계로 변경시킨 후 전달 필요.
            onePointGrabInfo[index].m_vecPointGrabPosition_um[nSize].m_y
                = onePointGrabInfo[index].m_vecPointGrabPosition_um[nSize].m_y * (-1);
        }

        onePointGrabInfo[index].m_grabHeightOffset_um = m_vecGrabHeightOffset_um[index];
        onePointGrabInfo[index].m_afType = m_vecAFtype[index];
        onePointGrabInfo[index].m_stitchX = m_vecStitchCount[index].m_x;
        onePointGrabInfo[index].m_stitchY = m_vecStitchCount[index].m_y;

        long illumSize = (long)m_vec2IllumUsage[index].size();
        for (long illumIndex = 0; illumIndex < illumSize; illumIndex++)
        {
            onePointGrabInfo[index].m_vecIllumUse.push_back(m_vec2IllumUsage[index][illumIndex]);
        }
    }

    // 3. parameter에 저장
    NgrvSingleRunInfo singleRunInfo;
    singleRunInfo.Init();

    singleRunInfo.m_vecOnePointGrabInfo.resize((long)onePointGrabInfo.size());

    for (long infoIndex = 0; infoIndex < (long)onePointGrabInfo.size(); infoIndex++)
    {
        singleRunInfo.m_vecOnePointGrabInfo[infoIndex] = onePointGrabInfo[infoIndex];
    }

    // 4. visionUnit을 거쳐 visionmain에 NGRV parameter에 저장
    primaryUnit.SetNGRVSingleRunInfo(VisionType, singleRunInfo);

    return true;
}

CString VisionMainTR::GetRecipeFileVersion()
{
    return m_strRecipeFileVersion;
}

void VisionMainTR::InlineImageSave_AllFrame(VisionUnit& visionUnit, const ImageSaveOptionData& option,
    std::vector<CString>& o_vecstr2DReviewAll_FrameFilePath, CString& o_str2DReview_AllFrameColorImageFilePath,
    std::vector<CString>& o_vecstr2DRaw_AllFrameColorImageFilePath, const enSideVisionModule i_eSideVisionModule)
{
    o_vecstr2DReviewAll_FrameFilePath.clear();
    o_vecstr2DRaw_AllFrameColorImageFilePath.clear();

    const auto& imageInfo = visionUnit.getImageLot().GetInfo();

    const long nTrayIndex = imageInfo.m_trayIndex;
    const long nScanID = imageInfo.m_scanAreaIndex;
    const long paneIndex = visionUnit.GetCurrentPaneID();

    CString strRealRawImagePath(""), strRealReviewImagePath(""), strRealCroppingImagePath("");
    if (CreateImageSave_Directory(visionUnit, nTrayIndex, nScanID, paneIndex, strRealRawImagePath,
            strRealReviewImagePath, strRealCroppingImagePath)
        != ImageSaveStatus::ImageSaveStatus_CreateDirectory_Success)
    {
        return;
    }

    CString reviewImageAllFilePath("");
    reviewImageAllFilePath.Format(_T("%s%s_T%d_S%d_P%d"), (LPCTSTR)strRealReviewImagePath,
        LPCTSTR(m_lotReviewFileHeader), nTrayIndex, nScanID, paneIndex);

    Ipvm::CreateDirectories(LPCTSTR(strRealReviewImagePath));
    ImageSave_Review_AllFrame_2D(visionUnit, option, _T(""), _T(""), reviewImageAllFilePath,
        o_vecstr2DReviewAll_FrameFilePath, o_str2DReview_AllFrameColorImageFilePath, i_eSideVisionModule);

    BOOL isRawSave = RawImageSaveResult(option, visionUnit.GetInspTotalResult(), true);
    if (isRawSave && visionUnit.getImageLot().InlineRawSaveCheck())
    {
        // Raw Image Save은 Raw Image 저장 조건에 한번만이라도 부합된다면
        // 한번만 저장한다. (같은 파일을 중복 저장하는 것 방지)

        CString strRawImageSaveFilePath(_T(""));
        Ipvm::CreateDirectories(LPCTSTR(strRealRawImagePath));
        RawImageSave(visionUnit, strRealRawImagePath, strRawImageSaveFilePath);

        o_vecstr2DRaw_AllFrameColorImageFilePath.push_back(strRawImageSaveFilePath);

        strRawImageSaveFilePath.Empty();
    }

    strRealRawImagePath.Empty();
    strRealReviewImagePath.Empty();
    strRealCroppingImagePath.Empty();
    reviewImageAllFilePath.Empty();
}

ImageSaveStatus VisionMainTR::CreateImageSave_Directory(VisionUnit& visionUnit, const long i_nTrayIndex,
    const long i_scanID, const long i_paneID, CString& o_strRaw_Image_FilePath, CString& o_strReview_Image_FilePath,
    CString& o_strCropping_Image_FilePath)
{
    CString strRealRawJobPath; //??

    //이미지 저장 폴더 생성
    o_strRaw_Image_FilePath = CString(RAW_IMAGE_DIRECTORY);
    strRealRawJobPath = CString(RAW_IMAGE_DIRECTORY);
    o_strReview_Image_FilePath = CString(REVIEW_IMAGE_DIRECTORY);
    o_strCropping_Image_FilePath = CString(CROPPING_IMAGE_DIRECTORY);

    Ipvm::CreateDirectories(o_strRaw_Image_FilePath);
    Ipvm::CreateDirectories(o_strReview_Image_FilePath);
    Ipvm::CreateDirectories(o_strCropping_Image_FilePath);

    if (!CheckFreeDiskForImageSave(true))
    {
        // 저장공간 부족
        return ImageSaveStatus::ImageSaveStatus_CreateDirectory_Fail;
    }

    PathToSaveImage(visionUnit, i_nTrayIndex, i_scanID, i_paneID, &o_strRaw_Image_FilePath, &strRealRawJobPath,
        &o_strReview_Image_FilePath, &o_strCropping_Image_FilePath);

    strRealRawJobPath.Empty();

    return ImageSaveStatus::ImageSaveStatus_CreateDirectory_Success;
}

void VisionMainTR::SetSampleImageOption(const long i_nSampleImageOption)
{
    m_nPassDeviceSaveCount = i_nSampleImageOption;
}

void VisionMainTR::ResetDevicePassCount()
{
    m_nDevicePassCount = 0;
}

bool VisionMainTR::GetSend2DAllFrame()
{
    bool bCurrentSend2DAllFrame = m_bSend2DAllFrame;

    m_bSend2DAllFrame = false; //mc_초기화 할곳이 마땅하지 않아서 이곳에..

    return bCurrentSend2DAllFrame;
}

std::vector<CString> VisionMainTR::getAllFrame_2D_FilePath(const bool i_bisRawFilePath)
{
    if (i_bisRawFilePath == true)
        return m_vecstr2DRaw_AllFrameFilePath;
    else
        return m_vecstr2DReview_AllFrameFilePath;
}

void VisionMainTR::ResetInlineGrabTime()
{
    m_inline_grab->ResetInline_GrabTimeLOG();
}

void VisionMainTR::SaveInlineGrabTime()
{
    m_inline_grab->SaveInline_GrabTimeLOG();
}

void VisionMainTR::ConvertDeviceResultToBuffer(BYTE* pbyBuffer, VisionDeviceResult& deviceResult, BOOL bSave)
{
    if (pbyBuffer == NULL)
        return;

    if (bSave)
    {
        memset(m_pbySendResultBuffer, 0, MAX_SEND_RESULT_SIZE);
        CMemFile memFile(m_pbySendResultBuffer, MAX_SEND_RESULT_SIZE, MAX_SEND_RESULT_SIZE);
        ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);
        deviceResult.Serialize(ar);
        ar.Close();

        memFile.SeekToBegin();
        memcpy(pbyBuffer, memFile.Detach(), MAX_SEND_RESULT_SIZE);
    }
    else
    {
        CMemFile memFile;
        memFile.Attach((byte*)pbyBuffer, MAX_SEND_RESULT_SIZE);
        ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);
        deviceResult.Serialize(ar);
    }
}

static CCriticalSection g_ReviewimageSaveCS;

void VisionMainTR::SaveReviewImage_DeubgLOG(VisionUnit& visionUnit, const CString i_strRejectNames,
    const CString i_strModuleName, const CString i_strSKD_NotSuccessMSG,
    const eReviewimageSaveLOGCategory i_eReviewimageSaveLogCatrgory, const bool i_bNeedRawimageSave)
{
    g_ReviewimageSaveCS.Lock();

    CFile File;
    CString strFileName("");
    CString strinspidxinfo(""); //Trayindex, ScanID, PaneID
    CString str2DID(""); //2DID를 기록 할 수있으면 기록한다

    const auto& imageInfo = visionUnit.getImageLot().GetInfo();

    const long nTrayIndex = imageInfo.m_trayIndex;
    const long nScanID = imageInfo.m_scanAreaIndex;
    const long paneIndex = visionUnit.GetCurrentPaneID();

    long nDataNum(0);
    void* p2DMatrixData
        = visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_2D_MATRIX, _T("2D Matrix Major Result"), nDataNum);
    if (nDataNum > 0 && p2DMatrixData != nullptr)
    {
        CString* pStr2DID = (CString*)p2DMatrixData;
        if (!pStr2DID[0].IsEmpty())
            str2DID = pStr2DID[0];
    }

    strinspidxinfo.Format(_T("TrayIndex:[%d],ScanID[%d],Pane[%d]"), nTrayIndex, nScanID, paneIndex);

    // 파일을 생성한다. 혹은 기존에 있으면 추가 한다.
    strFileName.Format(_T("%sVisionDebugLog(%s)_%s.txt"), (LPCTSTR)DynamicSystemPath::get(DefineFolder::Log),
        (LPCTSTR)SystemConfig::GetInstance().m_strVisionInfo, (LPCTSTR)(CTime::GetCurrentTime().Format(_T("%Y%m%d"))));

    if (!File.Open(strFileName, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite))
    {
        g_ReviewimageSaveCS.Unlock();
        return;
    }
    else
    {
        // 파일의 맨 마지막을 찾는다.
        File.SeekToEnd();

        CString strWriteTime("");
        strWriteTime.Format(_T("[%s]\t"), (LPCTSTR)(CTime::GetCurrentTime().Format(_T("%H:%M:%S"))));

        CArchive ar(&File, CArchive::store);
        ar.WriteString(strWriteTime);
        ar.WriteString(g_szReviewimageSaveLOGCategoryNames[i_eReviewimageSaveLogCatrgory]);
        ar.WriteString(_T("\t"));

        if (i_strModuleName.IsEmpty() == false)
        {
            ar.WriteString(i_strModuleName);
        }
        ar.WriteString(_T("\t"));

        if (i_strRejectNames.IsEmpty() == false)
        {
            ar.WriteString(i_strRejectNames);
        }
        ar.WriteString(_T("\t"));

        if (i_strSKD_NotSuccessMSG.IsEmpty() == false)
        {
            ar.WriteString(i_strSKD_NotSuccessMSG);
        }
        ar.WriteString(_T("\t"));

        ar.WriteString(strinspidxinfo);

        ar.WriteString(_T("\t"));
        if (str2DID.IsEmpty() == false && p2DMatrixData != nullptr)
        {
            ar.WriteString(str2DID);
        }

        ar.WriteString(_T("\r\n"));
        ar.Close();
        File.Close();
        strWriteTime.Empty();
    }

    g_ReviewimageSaveCS.Unlock();

    if (i_bNeedRawimageSave == true)
    {
        if (!GetFreeSpaceCheck(20, _TEXT("C:\\")))
        {
            SaveReviewImage_DeubgLOG(visionUnit, _T(""), _T(""), _T(""),
                eReviewimageSaveLOGCategory::eReviewimageSaveLOGCatrgory_InsufficientDiskCapacity_C, false);
            return;
        }

        if (visionUnit.getImageLot().InlineRawSaveCheck())
        {
            // Raw Image Save은 Raw Image 저장 조건에 한번만이라도 부합된다면
            // 한번만 저장한다. (같은 파일을 중복 저장하는 것 방지)
            CString strLOGRawimagePath("");
            CString strTimeName("");
            CString strDummy(_T(""));
            strTimeName.Format(_T("%04d.%02d.%02d"), CTime::GetCurrentTime().GetYear(),
                CTime::GetCurrentTime().GetMonth(), CTime::GetCurrentTime().GetDay());
            strLOGRawimagePath.Format(_T("%s%s\\%s\\%s\\"), (LPCTSTR)DynamicSystemPath::get(DefineFolder::Log),
                LPCTSTR(m_strJobFileName), (LPCTSTR)strTimeName, LPCTSTR(m_lotID));

            Ipvm::CreateDirectories(LPCTSTR(strLOGRawimagePath));
            RawImageSave(visionUnit, strLOGRawimagePath, strDummy);

            strLOGRawimagePath.Empty();
            strTimeName.Empty();
            strDummy.Empty();
        }
    }
    strFileName.Empty();
}

BOOL VisionMainTR::ClassifyInspItemInfo(std::vector<CString> vecstrReceivedInspItemName) //kircheis_NGRV
{
    long nVisionSize = (long)m_vec2InspItemNGRV.size();
    for (long nVision = 0; nVision < nVisionSize; nVision++)
    {
        m_vec2InspItemNGRV[nVision].clear();
    }
    m_vec2InspItemNGRV.clear();
    m_vec2InspItemNGRV.resize(4);

    nVisionSize = (long)m_vecstrEachVisionName.size();

    long nReceivedItemNum = (long)vecstrReceivedInspItemName.size();
    long nInspItemSizeInRecipe = (long)m_vecGrabItemFrameInfo.size();
    long nVisionNumber = 0;
    BOOL bIncludedVisionName = true;
    BOOL bExist = false;
    std::vector<CGrabItemFrameInfoNGRV> vecGrabItemFrameInfo(0);
    CGrabItemFrameInfoNGRV tmpGrabItemFrameInfo;
    CString strCurInspItemName;

    for (long nItem = 0; nItem < nReceivedItemNum; nItem++)
    {
        strCurInspItemName = vecstrReceivedInspItemName[nItem];
        nVisionNumber = -1;
        for (long nVision = 0; nVision < nVisionSize; nVision++) //먼저 Vision Name 이 포함 되어있는지 확인 하고
        {
            if (strCurInspItemName.Find(m_vecstrEachVisionName[nVision]) == 1)
            {
                nVisionNumber = nVision;
                break;
            }
        }

        if (nVisionNumber == -1)
        {
            bIncludedVisionName = false;
            continue;
        }
        m_vec2InspItemNGRV[nVisionNumber].push_back(strCurInspItemName);

        bExist = false;
        for (long nRecipeItem = 0; nRecipeItem < nInspItemSizeInRecipe; nRecipeItem++)
        {
            if (strCurInspItemName == m_vecGrabItemFrameInfo[nRecipeItem].m_strGrabItemName)
            {
                bExist = true;
                vecGrabItemFrameInfo.push_back(m_vecGrabItemFrameInfo[nRecipeItem]);
                break;
            }
        }

        if (!bExist)
        {
            tmpGrabItemFrameInfo.m_strGrabItemName = strCurInspItemName;
            tmpGrabItemFrameInfo.m_bIsSaveImage = false;
            tmpGrabItemFrameInfo.m_vecnFrameID.clear();
            tmpGrabItemFrameInfo.m_vecnFrameID.push_back(0);
            vecGrabItemFrameInfo.push_back(tmpGrabItemFrameInfo);
        }
    }

    m_vecGrabItemFrameInfo.clear();
    long nSize = (long)vecGrabItemFrameInfo.size();
    for (long i = 0; i < nSize; i++)
        m_vecGrabItemFrameInfo.push_back(vecGrabItemFrameInfo[i]);

    return bIncludedVisionName;
}

BOOL VisionMainTR::ClassifyInspItemInfo(std::vector<CGrabItemFrameInfoNGRV> vecGrabItemFrameInfoNGRV) //kircheis_NGRV
{
    long nVisionSize = (long)m_vec2InspItemNGRV.size();
    for (long nVision = 0; nVision < nVisionSize; nVision++)
    {
        m_vec2InspItemNGRV[nVision].clear();
    }
    m_vec2InspItemNGRV.clear();
    m_vec2InspItemNGRV.resize(4);

    long nInspItemSizeInRecipe = (long)vecGrabItemFrameInfoNGRV.size();
    CString strCurInspItemName;
    long nVisionNumber = -1;
    BOOL bIncludedVisionName = true;
    nVisionSize = (long)m_vecstrEachVisionName.size();

    for (long nItem = 0; nItem < nInspItemSizeInRecipe; nItem++)
    {
        strCurInspItemName = vecGrabItemFrameInfoNGRV[nItem].m_strGrabItemName;
        nVisionNumber = -1;
        for (long nVision = 0; nVision < nVisionSize; nVision++) //먼저 Vision Name 이 포함 되어있는지 확인 하고
        {
            if (strCurInspItemName.Find(m_vecstrEachVisionName[nVision]) == 1)
            {
                nVisionNumber = nVision;
                break;
            }
        }

        if (nVisionNumber == -1)
        {
            bIncludedVisionName = false;
            continue;
        }
        m_vec2InspItemNGRV[nVisionNumber].push_back(strCurInspItemName);
    }

    return bIncludedVisionName;
}

void VisionMainTR::SetNGRV_SaveOption(CString strInspModuleName, BOOL bSave) //kircheis_NGRV
{
    long nSize = (long)m_vecGrabItemFrameInfo.size();

    for (long i = 0; i < nSize; i++)
    {
        if (m_vecGrabItemFrameInfo[i].m_strGrabItemName == strInspModuleName)
        {
            m_vecGrabItemFrameInfo[i].m_bIsSaveImage = bSave;
            return;
        }
    }
}

BOOL VisionMainTR::GetNGRV_SaveOption(CString strInspModuleName) //kircheis_NGRV
{
    long nSize = (long)m_vecGrabItemFrameInfo.size();

    for (long i = 0; i < nSize; i++)
    {
        if (m_vecGrabItemFrameInfo[i].m_strGrabItemName == strInspModuleName)
        {
            return m_vecGrabItemFrameInfo[i].m_bIsSaveImage;
        }
    }

    return false;
}

long VisionMainTR::GetVisionID_InspModule(CString strInspModuleName) //kircheis_NGRV
{
    long nSize = (long)m_vecstrEachVisionName.size();
    for (long i = 0; i < nSize; i++)
    {
        //int nIndex = strInspModuleName.Find(m_vecstrEachVisionName[i]);
        if (strInspModuleName.Find(m_vecstrEachVisionName[i]) == 1)
        {
            return i;
        }
    }

    return -1;
}

BOOL VisionMainTR::GetFrameInfoNGRV(
    CString i_strInspModuleName, BOOL& o_bUseIR, std::vector<long>& o_vecFrameID) //kircheis_NGRV
{
    o_bUseIR = false;
    o_vecFrameID.clear();

    long nModuleNum = (long)m_vecGrabItemFrameInfo.size();

    for (long idx = 0; idx < nModuleNum; idx++)
    {
        if (m_vecGrabItemFrameInfo[idx].m_strGrabItemName == i_strInspModuleName)
        {
            //BOOL bUseIR = false;
            long nFrameNum = (long)m_vecGrabItemFrameInfo[idx].m_vecnFrameID.size();
            for (long nFrame = 0; nFrame < nFrameNum; nFrame++)
                o_vecFrameID.push_back(m_vecGrabItemFrameInfo[idx].m_vecnFrameID[nFrame]);

            o_bUseIR = (o_vecFrameID[nFrameNum - 1] == 9);
            return true;
        }
    }

    return false;
}

BOOL VisionMainTR::GetFrameInfoNGRV(CString i_strInspModuleName, BOOL& o_bUseIR, BOOL& o_bUseUV,
    std::vector<long>& o_vecFrameID, std::vector<long>& o_vecUVIRFrameID) //kircheis_NGRVGain
{
    o_bUseIR = false;
    o_bUseUV = false;
    o_vecFrameID.clear();
    o_vecUVIRFrameID.clear();

    long nModuleNum = (long)m_vecGrabItemFrameInfo.size();

    for (long idx = 0; idx < nModuleNum; idx++)
    {
        if (m_vecGrabItemFrameInfo[idx].m_strGrabItemName == i_strInspModuleName)
        {
            //BOOL bUseIR = false;
            long nFrameNum = (long)m_vecGrabItemFrameInfo[idx].m_vecnFrameID.size();
            for (long nFrame = 0; nFrame < nFrameNum; nFrame++) //먼저 UV가 사용되는지 확인한다.
            {
                if (m_vecGrabItemFrameInfo[idx].m_vecnFrameID[nFrame] == NGRV_FrameID_UV)
                    o_bUseUV = TRUE;
                else if (m_vecGrabItemFrameInfo[idx].m_vecnFrameID[nFrame] == NGRV_FrameID_IR)
                    o_bUseIR = TRUE;
            }

            for (long nFrame = 0; nFrame < nFrameNum; nFrame++)
            {
                if (m_vecGrabItemFrameInfo[idx].m_vecnFrameID[nFrame] < NGRV_FrameID_UV)
                    o_vecFrameID.push_back(m_vecGrabItemFrameInfo[idx].m_vecnFrameID[nFrame]);
                else
                    o_vecUVIRFrameID.push_back(m_vecGrabItemFrameInfo[idx].m_vecnFrameID[nFrame]);
            }

            return true;
        }
    }

    return false;
}

void VisionMainTR::GetNGRVFrameInfo(NgrvFrameInfo& o_ngrvFrameInfo) //kircheis_NGRV
{
    o_ngrvFrameInfo.m_vecEachFrameInfo.clear();
    o_ngrvFrameInfo.m_nBeginIRFrame = -1;

    if (SystemConfig::GetInstance().IsVisionTypeNGRV() == false)
        return;

    NgrvEachFrameInfo eachFrameInfo;
    long nGrabItemNum = (long)m_vecGrabItemFrameInfo.size();
    long nFrameNum = m_illum2D.getTotalFrameCount();
    long nItemFrameNum = 0;

    long nIRPosFrameCount = m_illum2D.getHighFrameCount();
    long nBeginIRPosFrameID = nFrameNum - nIRPosFrameCount;
    o_ngrvFrameInfo.m_nBeginIRFrame = nIRPosFrameCount != 0 ? nBeginIRPosFrameID : -1;

    for (long nFrame = 0; nFrame < nFrameNum; nFrame++)
    {
        eachFrameInfo.Init();
        eachFrameInfo.m_bIsIR = (nFrame >= nBeginIRPosFrameID);
        for (long nItem = 0; nItem < nGrabItemNum; nItem++)
        {
            if (m_vecGrabItemFrameInfo[nItem].m_bIsSaveImage == false)
                continue;

            nItemFrameNum = (long)m_vecGrabItemFrameInfo[nItem].m_vecnFrameID.size();
            if (nItemFrameNum == 0)
                continue;

            for (long nItemFrame = 0; nItemFrame < nItemFrameNum; nItemFrame++)
            {
                if (nFrame == m_vecGrabItemFrameInfo[nItem].m_vecnFrameID[nItemFrame])
                {
                    eachFrameInfo.m_vecstrInspModuleName.push_back(m_vecGrabItemFrameInfo[nItem].m_strGrabItemName);
                    break;
                }
            }
        }

        o_ngrvFrameInfo.m_vecEachFrameInfo.push_back(eachFrameInfo);
    }
}

void VisionMainTR::GetNGRVGrabPositionInfo(std::vector<NgrvGrabPositionInfo>& o_vecGrabPositionInfo) //kircheis_NGRV
{
    o_vecGrabPositionInfo.clear();

    if (SystemConfig::GetInstance().IsVisionTypeNGRV() == false)
        return;

    NgrvGrabPositionInfo grabPositionInfo;
    long nGrabItemNum = (long)m_vecGrabItemFrameInfo.size();
    long nFrameNum = m_illum2D.getTotalFrameCount();
    long nIRPosFrameCount = m_illum2D.getHighFrameCount();
    long nBeginIRPosFrameID = nFrameNum - nIRPosFrameCount;
    if (nIRPosFrameCount == 0)
        nBeginIRPosFrameID = 10;

    for (long nItem = 0; nItem < nGrabItemNum; nItem++)
    {
        grabPositionInfo.Init();
        grabPositionInfo.m_strModuleName = m_vecGrabItemFrameInfo[nItem].m_strGrabItemName;
        grabPositionInfo.m_bIsSaveImage = m_vecGrabItemFrameInfo[nItem].m_bIsSaveImage;

        nFrameNum = (long)m_vecGrabItemFrameInfo[nItem].m_vecnFrameID.size();
        nIRPosFrameCount = 0;
        for (long nFrame = 0; nFrame < nFrameNum; nFrame++)
        {
            if (m_vecGrabItemFrameInfo[nItem].m_vecnFrameID[nFrame] >= nBeginIRPosFrameID)
                nIRPosFrameCount++;
        }

        grabPositionInfo.m_nNormalFrameNum = nFrameNum - nIRPosFrameCount;
        grabPositionInfo.m_nIRFrameNum = nIRPosFrameCount;
        grabPositionInfo.m_nAFType = m_vecGrabItemFrameInfo[nItem].m_nAFType;
        grabPositionInfo.m_nPlaneOffsetUM = m_vecGrabItemFrameInfo[nItem].m_nPlaneOffsetUM;

        o_vecGrabPositionInfo.push_back(grabPositionInfo);
    }
}

void VisionMainTR::SetReadySignal(BOOL bActive)
{
    SyncController::GetInstance().SetReadySignal(bActive);
}

void VisionMainTR::SaveGrabInfoLog(std::vector<NgrvPackageGrabInfo> i_vecPackageGrabInfo, BOOL i_bFlip)
{
    CTime time(CTime::GetCurrentTime());
    CString strTime = time.Format(_T("%Y%m%d_%H%M%S"));
    CString strErrorLogPath;
    if (i_bFlip)
    {
        strErrorLogPath.Format(_T("D:\\Review Image\\PackageGrabInfo_TopSide.csv"));
    }
    else
    {
        strErrorLogPath.Format(_T("D:\\Review Image\\PackageGrabInfo_BottomSide.csv"));
    }

    FILE* fp = nullptr;
    _tfopen_s(&fp, strErrorLogPath, _T("w"));

    if (fp != nullptr)
    {
        long nPackageNum = (long)i_vecPackageGrabInfo.size();
        long nGrabPointNum = 0;
        fprintf(fp, ("Time, %ls\n"), (LPCTSTR)strTime);
        for (long nPackage = 0; nPackage < nPackageNum; nPackage++)
        {
            auto packGrabInfo = i_vecPackageGrabInfo[nPackage];
            fprintf(fp, (", , PackageIO->, %03d, PackageID->,%ls\n"), packGrabInfo.m_nPackageID,
                (LPCTSTR)packGrabInfo.m_strPackageID);
            nGrabPointNum = (long)packGrabInfo.m_vecGrabPointInfo.size();
            for (long nGrab = 0; nGrab < nGrabPointNum; nGrab++)
            {
                auto grabInfo = packGrabInfo.m_vecGrabPointInfo[nGrab];
                fprintf(
                    fp, (", , , , , , ID->,%03d, ModuleName->, %ls\n"), nGrab, (LPCTSTR)grabInfo.m_strInspModuleName);
            }
        }
    }
    fclose(fp);
}

std::vector<CString> VisionMainTR::ExportRecipeToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrResult(0);
    std::vector<CString> vecStrTotalText(0);

    if (SystemConfig::GetInstance().IsVisionType3D() == TRUE)
        vecStrTotalText = ExportRecipe3DIllumToText();
    else
        vecStrTotalText = ExportRecipe2DIllumToText();

    static const BOOL bIsNGRV = SystemConfig::GetInstance().IsVisionTypeNGRV();

    if (bIsNGRV == FALSE) //NGRV면 조명 설정만 저장하고 검사 정보 쪽은 패쓰..
    {
        vecStrResult = GetPrimaryVisionUnit().ExportRecipeToText();
        vecStrTotalText.insert(vecStrTotalText.end(), vecStrResult.begin(), vecStrResult.end());
        vecStrResult.clear();
    }

    return vecStrTotalText;
}

std::vector<CString> VisionMainTR::ExportRecipe2DIllumToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrTotalText(0);

    if (SystemConfig::GetInstance().IsVisionType3D() == TRUE)
        return vecStrTotalText;

    const static CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;
    CString strHeader;
    strHeader.Format(_T("%s,Hardware"), (LPCTSTR)strVisionName);

    //개별 Parameter용 변수
    CString strCategory = _T("Illumination 2D");
    CString strGroup;
    CString strParaName;
    CString strParaNameAux;
    CString strValue;

    long nFrameNum = (long)m_illum2D.getTotalFrameCount();
    long nHighFrameNum = (long)m_illum2D.getHighFrameCount();
    strGroup = _T("Frame Info");
    vecStrTotalText.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Frame Number"), _T(""), nFrameNum));
    vecStrTotalText.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("High Position Frame Number"), _T(""), nHighFrameNum));

    strGroup = _T("Common");
    vecStrTotalText.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Use Color Frame"), _T(""), (bool)m_illum2D.m_2D_colorFrame));
    if (m_illum2D.m_2D_colorFrame == TRUE)
    {
        vecStrTotalText.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Color Frame"), _T("Red Frame"), (bool)m_illum2D.m_2D_colorFrameIndex_red));
        vecStrTotalText.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Color Frame"), _T("Green Frame"), (bool)m_illum2D.m_2D_colorFrameIndex_green));
        vecStrTotalText.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            _T("Color Frame"), _T("Blue Frame"), (bool)m_illum2D.m_2D_colorFrameIndex_blue));
    }

    static const float fMaxExpNormal = 65.f;
    static const float fMaxExpIRUV = 650.f;
    static const BOOL bIsNGRV = SystemConfig::GetInstance().IsVisionTypeNGRV();
    static const long nNgrvIRchID = SystemConfig::GetInstance().m_nNgrvIRchID;
    static const long nNgrvUVchID = SystemConfig::GetInstance().m_nNgrvUVchID;
    float fMaxExp(0.f);

    strGroup = _T("Illuminations");
    for (long nFrame = 0; nFrame < nFrameNum; nFrame++)
    {
        strParaName.Format(_T("Frame %d"), nFrame + 1);
        for (long nCh = 0; nCh < 16; nCh++)
        {
            if (bIsNGRV && (nCh == nNgrvIRchID || nCh == nNgrvUVchID))
                fMaxExp = fMaxExpIRUV;
            else
                fMaxExp = fMaxExpNormal;

            strParaNameAux.Format(_T("CH%02d"), nCh + 1);
            float fExp = m_illum2D.getIllumChannel_ms(nFrame, nCh);
            fExp = (float)max(0, min(fMaxExp, fExp));
            vecStrTotalText.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                strHeader, strCategory, strGroup, strParaName, strParaNameAux, fExp));
        }
    }

    return vecStrTotalText;
}

std::vector<CString> VisionMainTR::ExportRecipe3DIllumToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrTotalText(0);

    if (SystemConfig::GetInstance().IsVisionType3D() == FALSE)
        return vecStrTotalText;

    const static CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;
    CString strHeader;
    strHeader.Format(_T("%s,Hardware"), (LPCTSTR)strVisionName);

    //개별 Parameter용 변수
    CString strCategory = _T("Illumination 3D");
    CString strGroup;

    auto& para = m_pSlitBeam3DCommon;

    strGroup = _T("Illumination parameters");
    vecStrTotalText.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Illumination time"), _T(""), para->m_illuminationTime_ms));
    vecStrTotalText.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Illumination value"), _T(""), para->m_illuminationValue));

    strGroup = _T("Scanning parameters");
    vecStrTotalText.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Camera vertical binning"), _T(""), (bool)para->m_cameraVerticalBinning));
    vecStrTotalText.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Scan step"), _T(""), para->m_scanStep_um));
    vecStrTotalText.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Scan depth"), _T(""), para->m_scanDepth_um));
    vecStrTotalText.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Scan length"), _T(""), para->m_scanLength_mm));

    strGroup = _T("Algorithm parameters");
    vecStrTotalText.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Noise threshold"), _T(""), para->m_noiseThreshold));
    vecStrTotalText.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("Noise threshold (Final filtering)"), _T(""), para->m_noiseThreshold_FinalFiltering));
    vecStrTotalText.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Slitbeam thickness"), _T(""), para->m_beamThickness_um));

    return vecStrTotalText;
}

void VisionMainTR::SendToAutomationBatchEnd(
    long nInspTime) //SDY automation을 위해 Batch inspection end 메세지를 보낸다.
{
    // automation 관련 함수
    if (m_automationMessageSocket->IsConnected() == false)
        return;

    m_automationMessageSocket->Write(
        (int32_t)MSG_V2A::AUTOMATION_MSG_BATCH_INSPECTION_END, sizeof(nInspTime), &nInspTime);
}

void VisionMainTR::SetSideVisionSection(long i_nSideSection)
{
    m_nSideVisionSection = i_nSideSection;
    m_primaryVisionUnit->SetSideVisionSection(i_nSideSection);
}

long VisionMainTR::GetSideVisionSection()
{
    //SetupUI 제대로 집어넣을때 다시 생각해보자
    /*long nSideVisionSection;

	nSideVisionSection = m_primaryVisionUnit->GetSideVisionSection();

	if (nSideVisionSection != m_nSideVisionSection)
	{
		return nSideVisionSection;
	}
	else
	{
		return m_nSideVisionSection;
	}*/

    return m_nSideVisionSection;
}

void VisionMainTR::SetPackageInfo_NGRV(std::vector<NgrvPackageGrabInfo> packageInfo)
{
    m_primaryVisionUnit->SetPackageInfo_NGRV(packageInfo);
}

std::vector<NgrvPackageGrabInfo>& VisionMainTR::GetPackageInfo_NGRV()
{
    return m_primaryVisionUnit->GetPackageInfo_NGRV();
}

void VisionMainTR::SetAFInfo_NGRV(NgrvAfRefInfo afInfo)
{
    m_primaryVisionUnit->SetAFInfo_NGRV(afInfo);
}

NgrvAfRefInfo& VisionMainTR::GetAFInfo()
{
    return m_primaryVisionUnit->GetAFInfo();
}

void VisionMainTR::SetGrabInfo_NGRV(std::vector<NgrvGrabInfo> grabInfo)
{
    m_primaryVisionUnit->SetGrabInfo_NGRV(grabInfo);
}

std::vector<NgrvGrabInfo>& VisionMainTR::GetGrabInfo_NGRV()
{
    return m_primaryVisionUnit->GetGrabInfo_NGRV();
}

void VisionMainTR::Set2DImageInfo_NGRV(long visionID, Ngrv2DImageInfo imageInfo)
{
    m_primaryVisionUnit->Set2DImageInfo_NGRV(visionID, imageInfo);
}

Ngrv2DImageInfo& VisionMainTR::Get2DImageInfo_NGRV(long visionID)
{
    return m_primaryVisionUnit->Get2DImageInfo_NGRV(visionID);
}

void VisionMainTR::SetNGRVSingleRunInfo(long visionID, NgrvSingleRunInfo singleRunInfo)
{
    m_primaryVisionUnit->SetNGRVSingleRunInfo(visionID, singleRunInfo);
}

NgrvSingleRunInfo& VisionMainTR::GetNGRVSingleRunInfo(long visionID)
{
    return m_primaryVisionUnit->GetNGRVSingleRunInfo(visionID);
}

BOOL VisionMainTR::GetFrameInfoNGRV_SingleRun(CString i_strInspModuleName, BOOL& o_bUseIR, BOOL& o_bUseUV,
    std::vector<long>& o_vecFrameID, std::vector<long>& o_vecUVIRFrameID) //NGRV SingleRun - MED#6
{
    o_bUseIR = false;
    o_bUseUV = false;
    o_vecFrameID.clear();
    o_vecUVIRFrameID.clear();

    auto& ngrvSingleRunInfo = GetNGRVSingleRunInfo(m_isNGRVSingleRunVisionType);

    long pointNum = (long)GetNGRVSingleRunInfo(m_isNGRVSingleRunVisionType).m_vecOnePointGrabInfo.size();

    for (long idx = 0; idx < pointNum; idx++)
    {
        if (ngrvSingleRunInfo.m_vecOnePointGrabInfo[idx].m_defectPositionName == i_strInspModuleName)
        {
            long totalFrameNum = (long)ngrvSingleRunInfo.m_vecOnePointGrabInfo[idx].m_vecIllumUse.size();

            //총 사용하는 frame 개수를 알아온다.
            std::vector<long> usedFrame(0);
            for (long index = 0; index < totalFrameNum; index++)
            {
                if (ngrvSingleRunInfo.m_vecOnePointGrabInfo[idx].m_vecIllumUse[index] == TRUE)
                {
                    usedFrame.push_back(index);
                }
            }

            for (long frame = 0; frame < (long)usedFrame.size(); frame++) //먼저 UV가 사용되는지 확인한다.
            {
                if (usedFrame[frame] == NGRV_FrameID_UV)
                    o_bUseUV = TRUE;
                else if (usedFrame[frame] == NGRV_FrameID_IR)
                    o_bUseIR = TRUE;

                if (usedFrame[frame] < NGRV_FrameID_UV)
                    o_vecFrameID.push_back(usedFrame[frame]);
                else
                    o_vecUVIRFrameID.push_back(usedFrame[frame]);
            }

            return true;
        }
    }

    return false;
}

BOOL VisionMainTR::GetFrameInfoNGRV_SingleRun(
    CString i_strInspModuleName, BOOL& o_bUseIR, std::vector<long>& o_vecFrameID) //kircheis_NGRV
{
    o_bUseIR = false;
    o_vecFrameID.clear();

    auto& currentNGRVSingleInfo = GetNGRVSingleRunInfo(m_isNGRVSingleRunVisionType);

    long pointNum = (long)currentNGRVSingleInfo.m_vecOnePointGrabInfo.size();

    for (long idx = 0; idx < pointNum; idx++)
    {
        if (currentNGRVSingleInfo.m_vecOnePointGrabInfo[idx].m_defectPositionName == i_strInspModuleName)
        {
            long totalFrameNum = (long)currentNGRVSingleInfo.m_vecOnePointGrabInfo[idx].m_vecIllumUse.size();
            for (long frame = 0; frame < totalFrameNum; frame++)
            {
                if (currentNGRVSingleInfo.m_vecOnePointGrabInfo[idx].m_vecIllumUse[frame] == TRUE)
                    o_vecFrameID.push_back(frame);
            }

            o_bUseIR = FALSE;
            return true;
        }
    }

    return false;
}

void VisionMainTR::SetSocketInfo(Ipvm::SocketMessaging* pMessageSocket)
{
    m_primaryVisionUnit->SetSocketInfo(pMessageSocket);
}

void VisionMainTR::SetSyncStartFlag()
{
    ::SetEvent(m_Flag_Sync_Start);
}

void VisionMainTR::SetInlineStartFlag(const bool& isOn)
{
    if (isOn == true)
        ::SetEvent(m_Flag_Inline_Start);
    else
        ::ResetEvent(m_Flag_Inline_Start);
}