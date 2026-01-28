//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionInlineUI.h"

//CPP_2_________________________________ This project's headers
#include "VisionInlineUI.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../InspectionItems/VisionInspectionSurface/VisionInspectionSurface.h"
#include "../../InspectionItems/VisionInspectionSurface/VisionInspectionSurfacePara.h"
#include "../../ManagementModules/VisionUnit/VisionUnit.h"
#include "../../SharedCommunicationModules/VisionHostCommon/ArchiveAllType.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DebugOptionData.h"
#include "../../SharedCommunicationModules/VisionHostCommon/SideVisionDeviceResult.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionDeviceResult.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionResult.h"
#include "../../SharedComponent/Persistence/IniHelper.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionMainAgent.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Image8u3.h>
#include <Ipvm/Base/ImageFile.h>
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Base/TimeCheck.h>
#include <Ipvm/Gadget/Miscellaneous.h>
#include <afxmt.h>
#include <io.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_GRID_SPEC 100
#define UM_INSPECTION_START (WM_USER + 100)
#define UM_INSPECTION_END (WM_USER + 102)

//CPP_7_________________________________ Implementation body
//
#pragma comment(lib, "version.lib")

IMPLEMENT_DYNAMIC(CDlgVisionInlineUI, CDialog)

void Wait(DWORD dwMillisecond)
{
    MSG msg;
    DWORD t0, t1, diff;

    t0 = GetTickCount();

    while (TRUE)
    {
        t1 = GetTickCount();

        if (t0 <= t1)
            diff = t1 - t0;
        else
            diff = 0xFFFFFFFF - t0 + t1;

        if (diff > dwMillisecond)
            break;

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(1);
    }
}

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
        fprintf(fp, ("%ls\n"), (LPCTSTR)_strLogData);
        fclose(fp);
    }
}

CCriticalSection g_csSaveLock;

CDlgVisionInlineUI::CDlgVisionInlineUI(CVisionInlineUI& visionInlineUI, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionInlineUI::IDD, pParent)
    , m_visionInlineUI(visionInlineUI)
    , m_visionUnit(*visionInlineUI.m_pVisionUnit)
    , m_systemConfig(m_visionUnit.m_systemConfig)
    , m_imageLotView(nullptr)
{
    m_overlayResult = new VisionInspectionOverlayResult;
    m_pbyDevResultSaveBuffer = new BYTE[MAX_SEND_RESULT_SIZE];
}

CDlgVisionInlineUI::~CDlgVisionInlineUI()
{
    delete m_overlayResult;
    delete m_imageLotView;
    delete[] m_pbyDevResultSaveBuffer;
}

void CDlgVisionInlineUI::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgVisionInlineUI, CDialog)
ON_WM_DESTROY()
ON_WM_CLOSE()
ON_WM_SIZE()
ON_MESSAGE(UM_INSPECTION_START, &CDlgVisionInlineUI::OnInspectionStart)
ON_MESSAGE(UM_INSPECTION_END, &CDlgVisionInlineUI::OnInspectionEnd)
END_MESSAGE_MAP()

// CDlgVisionInlineUI 메시지 처리기입니다.
BOOL CDlgVisionInlineUI::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtDlg;
    GetClientRect(rtDlg);

    // Image Window 영역 설정
    CRect rtWindow;
    GetWindowRect(rtWindow);
    CRect rtImageWindow;

    rtImageWindow.left = 0;
    rtImageWindow.right = 320;
    rtImageWindow.top = 30 + 46;
    rtImageWindow.bottom = 570;
    rtImageWindow.InflateRect(-3, -3);

    m_imageLotView = new ImageLotView(rtImageWindow, m_visionUnit, false, GetSafeHwnd());

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgVisionInlineUI::OnDestroy()
{
    CDialog::OnDestroy();
}

void CDlgVisionInlineUI::OnClose()
{
    CDialog::OnDestroy();
}

void CDlgVisionInlineUI::OnSize(UINT nType, int cx, int cy)
{
    if (m_imageLotView->GetSafeHwnd())
    {
        ::MoveWindow(m_imageLotView->GetSafeHwnd(), 0, 0, cx, cy, TRUE);
    }

    __super::OnSize(nType, cx, cy);
}

long CDlgVisionInlineUI::OnThreadInspect(bool manualInsp)
{
    PostMessage(UM_INSPECTION_START);

    SYSTEMTIME ThreadInspectionStartTime;
    Ipvm::TimeCheck OnThreadInspectionTime;
    Ipvm::TimeCheck
        InspectionCheckOverTime; //mc_Handler로 Reuslt를 Send하기까지의 시간을 확인, Handler에 설정된 DataReceiveTime[TimeOut]시간이면, Log를 남길 수 있도록 한다
    bool bisLogAndRawimageSave(false);

    OnThreadInspectionTime.GetLocalTime(ThreadInspectionStartTime);

    Ipvm::TimeCheck inlineUIThreadReadyTime;

    //kircheis_SideVision //여기부터 Front/Rear 분리 필요/
    auto& imageLot = m_visionUnit.getImageLot();

    const auto& imageInfo = imageLot.GetInfo(enSideVisionModule::
            SIDE_VISIONMODULE_FRONT); //mc_TrayID와 ScanArea만 가져다 사용하므로, Front,Near 분기가 필요없다.

    const long nTrayIndex = imageInfo.m_trayIndex;
    const long scanAreaIndex = imageInfo.m_scanAreaIndex;
    long nPaneNum = (long)m_visionUnit.getInspectionAreaInfo().m_unitIndexList.size();
    long scanAreaCount = m_visionUnit.getTrayScanSpec().GetInspectionItemCount();

    long nImageSizeCheck = imageLot.GetImageFrameCount(enSideVisionModule::
            SIDE_VISIONMODULE_FRONT); //mc_Front와 Near는 같은 FrameCount를 가지니 별도로 분기가 필요없다.

    if (nImageSizeCheck <= 0)
        return -1;

    long paneIndex = m_visionUnit.GetCurrentPaneID();
    //{{ //kircheis_UPH_up//개별 검사 Thread가 개별 Package를 검사하니까 조건을 줘서 필요한 상황에서만 하자
    auto& sysConfig = SystemConfig::GetInstance();
    static const bool bIsHardwareExist = sysConfig.IsHardwareExist();
    static const bool bIsVision2D = sysConfig.IsVisionTypeBased2D(); //kircheis_SWIR
    static const bool bIsVision3D = sysConfig.GetVisionType() == VISIONTYPE_3D_INSP;
    static const bool bIsVisionSide = (bool)(sysConfig.GetVisionType() == VISIONTYPE_SIDE_INSP);
    BOOL bInlineImageSend = (paneIndex == 0 || nPaneNum == 1);
    const long nSendFrameID
        = (sysConfig.m_nSendHostFrameNumber < nImageSizeCheck) ? sysConfig.m_nSendHostFrameNumber : 0;

    if (m_visionInlineUI.m_pVisionMainAgent->isConnect() && bIsHardwareExist && bInlineImageSend)
    {
        if (bIsVisionSide == true)
            m_visionInlineUI.m_pVisionMainAgent->SendImage(
                imageLot.GetImageFrame(nSendFrameID, enSideVisionModule::SIDE_VISIONMODULE_FRONT),
                imageLot.GetImageFrame(nSendFrameID,
                    enSideVisionModule::
                        SIDE_VISIONMODULE_REAR)); //mc_SideVision Near image도 보내줘야하나..?==>너의 질문에 대한 답변임
        else
            m_visionInlineUI.m_pVisionMainAgent->SendImage(imageLot.GetImageFrame(nSendFrameID,
                enSideVisionModule::SIDE_VISIONMODULE_FRONT)); //mc_SideVision Near image도 보내줘야하나..?
    }
    //}}

    if (scanAreaIndex < 0 || scanAreaIndex >= scanAreaCount)
    {
        ASSERT(!_T("뭘 Scan하라고 한건지 모르겠다"));
        return -1;
    }

    Ipvm::TimeCheck TotalTimeInspectionEndtoSendResult, Panetime, ETC_Time;
    long paneResult(-1), nSumResult(0);
    VisionInspectionOverlayResult overlayResult;

    VisionDeviceResult deviceResult; //Normal Vision
    SideVisionDeviceResult SideVisionDeviceReulst; //Side Vision

    if (bIsVisionSide == false)
        overlayResult.SetImage(imageLot.GetImageFrame(
            nSendFrameID, enSideVisionModule::SIDE_VISIONMODULE_FRONT)); //mc_SideVision Near image도 보내줘야하나..?
    else
    {
        auto& imageFront = imageLot.GetImageFrame(nSendFrameID, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
        auto& imageRear = imageLot.GetImageFrame(nSendFrameID, enSideVisionModule::SIDE_VISIONMODULE_REAR);
        Ipvm::Rect32s rtImage = (Ipvm::Rect32s)imageFront;
        Ipvm::Rect32s rtMerged = rtImage;
        rtMerged.m_bottom *= 2;
        Ipvm::Rect32s rtDstRear = rtMerged;
        rtDstRear.m_top = rtImage.m_bottom;
        Ipvm::Image8u imageMerged;
        imageMerged.Create(rtMerged.Width(), rtMerged.Height());

        Ipvm::ImageProcessing::Copy(imageFront, rtImage, rtImage, imageMerged); //머지 버퍼에 Front Image 복사하고
        Ipvm::ImageProcessing::Copy(imageRear, rtImage, rtDstRear, imageMerged); //머지 버퍼의 하단부에 Rear Image 복사

        overlayResult.SetImage(imageMerged);
    }

    // 여기서 Reapeat Index Count를 계산해줬는데, 검사는 무조건 Side Vision 처럼 돌아간다...
    const long nRepeatCount = SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP ? 2 : 1;
    //for (long nRepeatidx = enSideVisionModule::SIDE_VISIONMODULE_START; nRepeatidx < enSideVisionModule::SIDE_VISIONMODULE_END; nRepeatidx++)
    for (long nRepeatidx = 0; nRepeatidx < nRepeatCount; nRepeatidx++)
    {
        nSumResult = 0;

        if (SystemConfig::GetInstance().Get_SaveSequenceAndInspectionTimeLog() == true)
        {
            m_sinlineSequenceLogInfo.SetInlineSequenceLog(
                _T("InlineUI_Preprocessing"), CAST_FLOAT(inlineUIThreadReadyTime.Elapsed_ms()));
        }

        Ipvm::TimeCheck InspectionTime;

        if (SystemConfig::GetInstance().Get_SaveSequenceAndInspectionTimeLog() == true)
        {
            m_sinlineSequenceLogInfo.SetInlineSequenceLog(
                _T("InlineUI_Preprocessing"), (float)inlineUIThreadReadyTime.Elapsed_ms());
        }

        SystemConfig::GetInstance().Save_VisionLog_InspectionTimeLog(m_visionUnit.m_threadIndex, nTrayIndex,
            scanAreaIndex, m_visionUnit.GetCurrentPaneID(), _T("Inspection_Start"));

        m_visionUnit.RunInspection(nullptr, false, enSideVisionModule(nRepeatidx));

        SystemConfig::GetInstance().Save_VisionLog_InspectionTimeLog(m_visionUnit.m_threadIndex, nTrayIndex,
            scanAreaIndex, m_visionUnit.GetCurrentPaneID(), _T("Inspection_End"));

        if (SystemConfig::GetInstance().Get_SaveSequenceAndInspectionTimeLog() == true)
        {
            m_sinlineSequenceLogInfo.SetInlineSequenceLog(
                _T("InlineUI_InspectionTime"), CAST_FLOAT(InspectionTime.Elapsed_ms()));
        }

        TotalTimeInspectionEndtoSendResult.Reset();

        if (SystemConfig::GetInstance().Get_SaveSequenceAndInspectionTimeLog() == true)
        {
            m_sinlineSequenceLogInfo.SetInlineSequenceLog(
                _T("InlineUI_InspectionTime"), (float)InspectionTime.Elapsed_ms());
        }

        overlayResult.AddOverlay(m_visionUnit.GetInspectionOverlayResult());

        paneResult = m_visionUnit.GetInspTotalResult();

        // Debug Stop...
        SetDebugStop(nSumResult, paneResult);

        Ipvm::TimeCheck InspectionSendTime;

        if (bIsVisionSide == true)
        {
            SideVisionDeviceReulst.m_vecVisionDeviceResult[nRepeatidx].SetinspectionStartTime(
                ThreadInspectionStartTime); //mc_Vision inspection StartTime

            m_visionInlineUI.m_pVisionMainAgent->PostExecutionProcessing(manualInsp, m_visionUnit,
                SideVisionDeviceReulst.m_vecVisionDeviceResult[nRepeatidx], enSideVisionModule(nRepeatidx));
        }
        else
        {
            deviceResult.SetinspectionStartTime(ThreadInspectionStartTime); //mc_Vision inspection StartTime

            m_visionInlineUI.m_pVisionMainAgent->PostExecutionProcessing(
                manualInsp, m_visionUnit, deviceResult, enSideVisionModule(nRepeatidx));
        }

        if (SystemConfig::GetInstance().Get_SaveSequenceAndInspectionTimeLog() == true)
        {
            m_sinlineSequenceLogInfo.SetInlineSequenceLog(
                _T("InlineUI_InspectionSendResultTime"), CAST_FLOAT(InspectionSendTime.Elapsed_ms()));
        }

        ETC_Time.Reset();
        if (m_systemConfig.m_bUseSaveCollectMVRejectData)
        {
            CMutex mutex(FALSE, _T("CollectData"));
            mutex.Lock();
            CollectMVRejectData(deviceResult);
            mutex.Unlock();
        }

        if (bIsVisionSide == true
            && nRepeatidx > enSideVisionModule::SIDE_VISIONMODULE_START) //Side Vision이고, Near까지 검사가 끝났다면
            SideVisionDeviceReulst.UpdateCommonInfo();
    }
    ///////////////////////////////////////////////////////////////////////////// mc_여기까지 묶여야한다

    // Send Deviceresult to shared memory
    BOOL b2ndInsp = FALSE;
    BOOL bSendSuccess(FALSE);

    if (deviceResult.m_nTotalResult != REJECT)
    {
        CString strLog;
        strLog.Format(_T("Only vision >> QueuePush = TrayID : %d, PocketID : %d, PaneID : %d"), deviceResult.m_nTrayID,
            deviceResult.m_nPocketID, deviceResult.m_nPane);
        LogMaster(_T("SendMVResultToHost"), strLog, 0, TRUE);
        if (bIsVisionSide == true)
            m_visionInlineUI.m_pVisionMainAgent->SendResult(m_visionUnit, SideVisionDeviceReulst, bSendSuccess);
        else
            m_visionInlineUI.m_pVisionMainAgent->SendResult(m_visionUnit, deviceResult, bSendSuccess);
    }
    else
    {
        if (m_systemConfig.m_bStateiDL)
            b2ndInsp = Send2ndInspInfo(deviceResult);

        if (b2ndInsp)
        {
            // D/L 검사를 위해 공유 메모리에 검사 데이터 올림
        }
        else
        {
            CString strLog;
            strLog.Format(_T("Only vision >> QueuePush = TrayID : %d, PocketID : %d, PaneID : %d"),
                deviceResult.m_nTrayID, deviceResult.m_nPocketID, deviceResult.m_nPane);
            LogMaster(_T("SendMVResultToHost"), strLog, 0, TRUE);
            if (bIsVisionSide == true)
                m_visionInlineUI.m_pVisionMainAgent->SendResult(m_visionUnit, SideVisionDeviceReulst, bSendSuccess);
            else
                m_visionInlineUI.m_pVisionMainAgent->SendResult(m_visionUnit, deviceResult, bSendSuccess);
        }
    }

    double dTotalTimeInspectionEndtoSendResultTime_ms = TotalTimeInspectionEndtoSendResult.Elapsed_ms();

    double dinspectionAndSendResultTotalTime_ms = InspectionCheckOverTime.Elapsed_ms();
    long nConvertinspectionAndSendResultTotalTime_ms
        = (long)(dinspectionAndSendResultTotalTime_ms + .5); //long 형태로 주고 받기 때문에
    if (nConvertinspectionAndSendResultTotalTime_ms >= SystemConfig::GetInstance().GetInspectionResultTimeoutTime_ms())
        bisLogAndRawimageSave = true;

    if (m_visionInlineUI.m_pVisionMainAgent->GetSend2DAllFrame() == true)
        m_visionInlineUI.m_pVisionMainAgent->Send2DAllFrame();

    float paneTime = (float)Panetime.Elapsed_ms(); // Time Check 끝...

    CString strTime;
    strTime.Format(_T("%s / %.2f ms"), (LPCTSTR)Result2String(paneResult), paneTime);
    overlayResult.AddText(Ipvm::Point32s2(0, 0), strTime, Result2Color(paneResult));

    if (m_visionUnit.m_systemConfig.m_saveInlineRawImage && !manualInsp)
    {
        if (m_visionInlineUI.m_pVisionMainAgent->CheckFreeDiskForImageSave(false))
        {
            // Manual Ispection (Batch) 일 때는 Raw Image을 또 저장하지 않는다
            const auto& lotStartTime = m_visionInlineUI.m_pVisionMainAgent->m_timeLotStart;

            CTime time(CTime::GetCurrentTime());

            CString strDirectory;
            strDirectory.Format(_T("%s%04d.%02d.%02d\\%s\\%s\\Run_%02d\\"), RAW_IMAGE_DIRECTORY, lotStartTime.GetYear(),
                lotStartTime.GetMonth(), lotStartTime.GetDay(), m_visionUnit.GetJobFileName(),
                LPCTSTR(m_visionInlineUI.m_pVisionMainAgent->m_lotID),
                m_visionInlineUI.m_pVisionMainAgent->GetInlineRunCnt());

            CString strDummyRawImageFilePath(_T(""));
            Ipvm::CreateDirectories(LPCTSTR(strDirectory));
            imageLot.SaveTo(strDirectory, strDummyRawImageFilePath);
        }
    }

    m_csOverlay.Lock();
    *m_overlayResult = overlayResult;
    m_csOverlay.Unlock();

    if (SystemConfig::GetInstance().Get_SaveSequenceAndInspectionTimeLog() == true)
    {
        m_sinlineSequenceLogInfo.SetInlineSequenceLog(_T("InlineUI_ETC Time"), (float)ETC_Time.Elapsed_ms());

        m_sinlineSequenceLogInfo.SetInlineSequenceLog(
            _T("InlineUI_OnThreadInspectionTime"), (float)OnThreadInspectionTime.Elapsed_ms());
    }

    if (bisLogAndRawimageSave == true)
    {
        //현재 Raw image를 저장
        //Vision Modules별 LogText저장

        CString strLogDirectory(_T("D:\\TimeoutLog\\"));
        CString strSaveDirectory = strLogDirectory;

        Ipvm::CreateDirectories(strSaveDirectory);

        CTime LotStartTime = m_visionInlineUI.m_pVisionMainAgent->m_timeLotStart;

        strSaveDirectory.AppendFormat(
            _T("%04d_%02d_%02d\\"), LotStartTime.GetYear(), LotStartTime.GetMonth(), LotStartTime.GetDay());

        Ipvm::CreateDirectories(strSaveDirectory);

        CString strLotID = m_visionInlineUI.m_pVisionMainAgent->m_lotID;
        CString strReicpeName = m_visionUnit.GetJobFileName();

        strSaveDirectory.AppendFormat(_T("%s\\"), (LPCTSTR)strReicpeName);

        Ipvm::CreateDirectories(strSaveDirectory);

        strSaveDirectory.AppendFormat(_T("%s_%02d_%02d_%02d\\"), (LPCTSTR)strLotID, LotStartTime.GetHour(),
            LotStartTime.GetMinute(), LotStartTime.GetSecond());

        Ipvm::CreateDirectories(strSaveDirectory);

        if (SystemConfig::GetInstance().GetInspectionResultTimeoutSaveRawimage() == true) //iPack Option에서 활성화
        {
            CString strDummy("");
            m_visionUnit.getImageLot().SaveTo(strSaveDirectory, strDummy);
        }

        CFile fp;
        strSaveDirectory.AppendFormat(_T("Tray[%03d]_ScanArea[%03d]_Repeat[%02d].txt"),
            m_visionUnit.getImageLot().GetInfo().m_trayIndex, m_visionUnit.getImageLot().GetInfo().m_scanAreaIndex,
            m_visionUnit.getImageLot().GetInspectionRepeatIndex());

        if (fp.Open(strSaveDirectory, CFile::modeCreate | CFile::modeWrite, NULL))
        {
            CString strLogData = m_visionUnit.GetLastInspection_Text(TRUE);
            strLogData.AppendFormat(
                _T("Total InspectionEndtoSendResult Time : %f"), dTotalTimeInspectionEndtoSendResultTime_ms);

            fp.Write(strLogData, strLogData.GetLength() * sizeof(TCHAR));
            fp.Close();

            strLogData.Empty();
        }

        strSaveDirectory.Empty();
    }

    //kircheis_SideVision //여기까지 Front/Rear 분리 필요/

    PostMessage(UM_INSPECTION_END);

    return nSumResult;
}

BOOL CDlgVisionInlineUI::Send2ndInspInfo(VisionDeviceResult& devResult, const enSideVisionModule i_eSideVisionModule)
{
    if (devResult.m_nTotalResult != REJECT)
        return FALSE;

    long nNumInspVsModule = (long)m_visionUnit.m_vecVisionModules.size();

    long nTrayID = devResult.m_nTrayID;
    long nPocketID = devResult.m_nPocketID;
    long nPaneID = devResult.m_nPane;

    //::Ipvm::Rect32s pane = m_visionUnit.m_pScanAreaImageInfo->GetCurrentPaneRectRaw();
    //CRect rtPane = CRect(pane.TopLeft().m_x, pane.TopLeft().m_y, pane.BottomRight().m_x, pane.BottomRight().m_y);

    CString strLotID = m_visionInlineUI.m_pVisionMainAgent->m_lotID;
    CString strJobName = m_visionUnit.GetJobFileName();

    //
    if (strLotID.GetLength() <= 0)
    {
        strLotID = _T("ManualInsp");
    }

    CTime curTime = m_visionInlineUI.m_pVisionMainAgent->m_timeLotStart;

    long nYear = curTime.GetYear();
    long nMonth = curTime.GetMonth();
    long nDay = curTime.GetDay();
    long nHour = curTime.GetHour();
    //long nMinute = curTime.GetMinute();

    CString strLotStartDate, strLotStartTime;

    strLotStartDate.Format(_T("%04d%02d%02d_"), nYear, nMonth, nDay);
    strLotStartTime.Format(_T("%02d_%02d"), nHour, 0);
    strLotID = strLotID;

    long nInspItemNum = (long)devResult.m_vecVisionInspResult.size();

    VisionImageLotInsp& lotInspImage = m_visionUnit.getImageLotInsp();
    long nFrmNum = lotInspImage.GetImageFrameCount();

    CString strInspModuleName;
    std::vector<CString> vecstrRejInspName;
    std::vector<CString> vecstr2ndInspCode;
    std::vector<BOOL> vecbUseCompInsp;

    long i, j, idx, k;

    BOOL bPASS = TRUE;
    BOOL bComponentInsp = FALSE;

    std::vector<BOOL> vecbCompUse2ndInsp;
    std::vector<CString> vecstrComp2ndInspCode;
    std::vector<CString> vecstrRejComp2ndInspCode; // Reject인 component의 2nd insp code => reject불량 ROI개수와 같다.

    std::vector<int> vecnRejCompIndx;

    for (i = 0; i < nNumInspVsModule; i++)
    {
        if (m_visionUnit.m_vecVisionModules[i]->m_bUse2ndInspection)
        {
            strInspModuleName = m_visionUnit.m_vecVisionModules[i]->m_strModuleName;

            for (j = 0; j < nInspItemNum; j++)
            {
                if (strInspModuleName == (*(devResult.m_vecVisionInspResult)[j]).m_inspName)
                {
                    if (!m_visionUnit.m_vecVisionModules[i]
                            ->m_bIsComponentInsp) // Component 검사가 아닌경우 08.20.20 KSY Component D/L Inspection
                    {
                        if ((*(devResult.m_vecVisionInspResult)[j]).m_totalResult == REJECT)
                        {
                            vecstrRejInspName.push_back(strInspModuleName);
                            vecstr2ndInspCode.push_back(m_visionUnit.m_vecVisionModules[i]->m_str2ndInspCode);
                            vecbUseCompInsp.push_back(FALSE);
                            bPASS = FALSE;
                        }
                    }
                    else
                    {
                        if ((*(devResult.m_vecVisionInspResult)[j]).m_totalResult == REJECT
                            && (((*(devResult.m_vecVisionInspResult)[j])).m_resultName == _T("PASSIVE_MISSING")
                                || ((*(devResult.m_vecVisionInspResult)[j])).m_resultName == _T("PASSIVE_ROTATE")
                                || ((*(devResult.m_vecVisionInspResult)[j])).m_resultName == _T("PASSIVE_LENGTH")))
                        {
                            if ((*(devResult.m_vecVisionInspResult)[j]).m_objectResults.size()
                                != m_visionUnit.m_vecVisionModules[i]->m_vecbUseComp2ndInsp.size())
                            {
                                continue;
                            }

                            long nNumComp = (long)(*(devResult.m_vecVisionInspResult)[j]).m_objectResults.size();

                            if (vecbCompUse2ndInsp.size() == 0)
                            {
                                vecbCompUse2ndInsp.resize(nNumComp, FALSE);
                                vecstrComp2ndInspCode.resize(nNumComp);
                            }

                            for (long comp = 0; comp < nNumComp; comp++)
                            {
                                if ((*(devResult.m_vecVisionInspResult)[j]).m_objectResults[comp] == REJECT)
                                {
                                    // 중복체크
                                    BOOL bDup = FALSE;
                                    for (long d = 0; d < vecnRejCompIndx.size(); d++)
                                    {
                                        if (comp == vecnRejCompIndx[d])
                                            bDup = TRUE;
                                    }

                                    if (!bDup)
                                        vecnRejCompIndx.push_back(comp);
                                    // Component에 하나의 불량이라도 있고 D/L 검사를 한다면 TRUE
                                    if (m_visionUnit.m_vecVisionModules[i]->m_vecbUseComp2ndInsp[comp])
                                    {
                                        bComponentInsp = TRUE;
                                        vecbCompUse2ndInsp[comp]
                                            = m_visionUnit.m_vecVisionModules[i]->m_vecbUseComp2ndInsp[comp];
                                        vecstrComp2ndInspCode[comp]
                                            = m_visionUnit.m_vecVisionModules[i]->m_vecstrComp2ndMatchCode[comp];
                                    }
                                    else
                                    {
                                        vecbCompUse2ndInsp[comp] = FALSE;
                                        vecstrComp2ndInspCode[comp] = _T("");
                                    }
                                }
                            }
                        }
                    }
                }

                if (bComponentInsp)
                {
                    bComponentInsp = FALSE;
                    BOOL bDuplicate = FALSE;
                    for (long a = 0; a < vecstrRejInspName.size(); a++)
                    {
                        if (vecstrRejInspName[a] == strInspModuleName)
                        {
                            bDuplicate = TRUE;
                            break;
                        }
                    }
                    if (bDuplicate)
                        continue;

                    vecstrRejInspName.push_back(strInspModuleName);
                    vecstr2ndInspCode.push_back(
                        _T("CompInsp")); // <- 임시로 넣어주는 코드, 실제로는 Component 유형별로 각기 다른 Code를 사용할 수 있기 때문에 따로 정보를 보내줌
                    //vecbUseDLOnly.push_back(m_visionUnit.m_vecVisionModules[i]->m_bUseOnlyDLInsp);
                    vecbUseCompInsp.push_back(TRUE);

                    bPASS = FALSE;
                }
            }
        }
    }

    VisionAlignResult* sEdgeAlignResult = NULL;

    // Body align information
    long nDataNum;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
        sEdgeAlignResult = (VisionAlignResult*)pData;

    CRect rtArea = (sEdgeAlignResult != NULL)
        ? CRect((int)sEdgeAlignResult->fptLT.m_x, (int)(int)sEdgeAlignResult->fptLT.m_y,
              (int)sEdgeAlignResult->fptRB.m_x, (int)sEdgeAlignResult->fptRB.m_y)
        : CRect(0, 0, 0, 0);
    ::Ipvm::Rect32s Area(0, 0, lotInspImage.GetImageSizeX(), lotInspImage.GetImageSizeY());

    // 덤프 이미지 테스트 코드
    //CString strFileName;
    //std::string str;
    //for (i = 0; i < nFrmNum; i++)
    //{
    //	strFileName.Format(_T("C:\\test\\Frame%d.BMP"), i);
    //	cv::Mat img(cvSize(lotInspImage.GetImageSizeX(), lotInspImage.GetImageSizeY()), CV_8UC1, lotInspImage.m_vecImages[i].GetMem(), cv::Mat::AUTO_STEP);
    //	str = CT2CA(strFileName);
    //	cv::imwrite(str, img);
    //}

    // 불량이 있으면..
    if (bPASS == FALSE && m_visionUnit.m_ptrSharedMem != NULL)
    {
        long nImageSizeX, nImageSizeY, nDepth;
        m_visionUnit.m_Mutex.Lock();

        long nCount = 0;

        while (m_visionUnit.m_ptrSharedMem->bAvailable)
        {
            if (nCount > m_visionUnit.m_systemConfig.m_nDLWaitTimeSendRejectData)
            {
                m_visionUnit.m_ptrSharedMem->bAvailable = FALSE;
                m_visionUnit.m_Mutex.Unlock();
                return FALSE;
            }

            Sleep(1);
            nCount++;
        }

        for (i = 0; i < lotInspImage.m_vecImages[i_eSideVisionModule].size(); i++)
        {
            nImageSizeX = lotInspImage.GetImageSizeX();
            nImageSizeY = lotInspImage.GetImageSizeY();

            if (nImageSizeX == 0 || nImageSizeY == 0)
                return FALSE;

            nDepth = (lotInspImage.m_vecImages[i_eSideVisionModule][i].GetWidthBytes() / nImageSizeX) * 8;
            BYTE* pbyImage = lotInspImage.m_vecImages[i_eSideVisionModule][i].GetMem();

            memcpy(m_visionUnit.m_ptrSharedImageMem + (MAX_DL_IMAGE_SIZE_X * MAX_DL_IMAGE_SIZE_Y * i), pbyImage,
                (nImageSizeX * (nDepth / 8)) * nImageSizeY);
        }

        if (sEdgeAlignResult != NULL)
        {
            m_visionUnit.m_ptrSharedMem->cTime = curTime;
            m_visionUnit.m_ptrSharedMem->nTrayID = nTrayID;
            m_visionUnit.m_ptrSharedMem->nPocketID = nPocketID;
            m_visionUnit.m_ptrSharedMem->nPaneID = nPaneID;
            ::strcpy_s(m_visionUnit.m_ptrSharedMem->lotID, (CStringA)strLotID);
            ::strcpy_s(m_visionUnit.m_ptrSharedMem->jobName, (CStringA)strJobName);
            m_visionUnit.m_ptrSharedMem->nNum2ndInsp = (int)vecstrRejInspName.size();
            m_visionUnit.m_ptrSharedMem->nNumFrame = nFrmNum;
            m_visionUnit.m_ptrSharedMem->nWidth = nImageSizeX;
            m_visionUnit.m_ptrSharedMem->nHeight = nImageSizeY;
            m_visionUnit.m_ptrSharedMem->nDepth = nDepth;
            m_visionUnit.m_ptrSharedMem->ptAlign[0]
                = CPoint((int)sEdgeAlignResult->fptLT.m_x, (int)sEdgeAlignResult->fptLT.m_y);
            m_visionUnit.m_ptrSharedMem->ptAlign[1]
                = CPoint((int)sEdgeAlignResult->fptRT.m_x, (int)sEdgeAlignResult->fptRT.m_y);
            m_visionUnit.m_ptrSharedMem->ptAlign[2]
                = CPoint((int)sEdgeAlignResult->fptLB.m_x, (int)sEdgeAlignResult->fptLB.m_y);
            m_visionUnit.m_ptrSharedMem->ptAlign[3]
                = CPoint((int)sEdgeAlignResult->fptRB.m_x, (int)sEdgeAlignResult->fptRB.m_y);
            //m_visionUnit.m_ptrSharedMem->ptPaneLT = rtPane.TopLeft(); // Insp 영상이 Pane기준으로 자른 영상이므로 Pane ROI가 필요없음 (Intel 500 iPack)

            // 08.24.20 KSY Component Inspection -----------------------------------------
            for (k = 0; k < vecbUseCompInsp.size(); k++)
                m_visionUnit.m_ptrSharedMem->bIsCompInsp[k] = vecbUseCompInsp[k];

            for (k = 0; k < vecbCompUse2ndInsp.size(); k++)
            {
                m_visionUnit.m_ptrSharedMem->bCompUse2ndInsp[k] = vecbCompUse2ndInsp[k];
                ::strcpy_s(m_visionUnit.m_ptrSharedMem->cComp2ndInspCode[k], (CStringA)vecstrComp2ndInspCode[k]);
            }

            m_visionUnit.m_ptrSharedMem->nNumTotRejectComponent = (int)vecnRejCompIndx.size();
            for (k = 0; k < vecnRejCompIndx.size(); k++)
            {
                m_visionUnit.m_ptrSharedMem->nRejCompIndex[k] = vecnRejCompIndx[k];
            }
            // ---------------------------------------------------------------------------

            for (i = 0; i < vecstrRejInspName.size(); i++)
            {
                ::strcpy_s(m_visionUnit.m_ptrSharedMem->c2ndInspName[i], (CStringA)vecstrRejInspName[i]);
                ::strcpy_s(m_visionUnit.m_ptrSharedMem->c2ndInspCode[i], (CStringA)vecstr2ndInspCode[i]);

                for (idx = 0; idx < m_visionUnit.m_visionInspectionSurfaces.size(); idx++)
                {
                    std::vector<CSurfaceCriteria>& vecCriteria
                        = m_visionUnit.m_visionInspectionSurfaces[idx]->m_surfacePara->m_SurfaceItem.vecCriteria;
                    SItemSpec sCriteriaSpec;

                    if (vecstr2ndInspCode[i]
                        == m_visionUnit.m_visionInspectionSurfaces[idx]
                            ->m_surfacePara->m_SurfaceItem.AlgoPara.m_str2ndInspCode)
                    {
                        if (vecCriteria.size() == 0)
                            continue;

                        sCriteriaSpec = vecCriteria[0].GetSpecMin(VisionSurfaceCriteria_Column::Length);
                        m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].length.fMin = sCriteriaSpec.GetPassMax();
                        m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].length.fMax = sCriteriaSpec.GetPassMin();
                        sCriteriaSpec = vecCriteria[0].GetSpecMin(VisionSurfaceCriteria_Column::Area);
                        m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].area.fMin = sCriteriaSpec.GetPassMax();
                        m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].area.fMax = sCriteriaSpec.GetPassMin();
                        sCriteriaSpec = vecCriteria[0].GetSpecMin(VisionSurfaceCriteria_Column::LengthX);
                        m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].lengthX.fMin = sCriteriaSpec.GetPassMax();
                        m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].lengthX.fMax = sCriteriaSpec.GetPassMin();
                        sCriteriaSpec = vecCriteria[0].GetSpecMin(VisionSurfaceCriteria_Column::LengthY);
                        m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].lengthY.fMin = sCriteriaSpec.GetPassMax();
                        m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].lengthY.fMax = sCriteriaSpec.GetPassMin();
                        sCriteriaSpec = vecCriteria[0].GetSpecMin(VisionSurfaceCriteria_Column::Thickness);
                        m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].thickness.fMin = sCriteriaSpec.GetPassMax();
                        m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].thickness.fMax = sCriteriaSpec.GetPassMin();
                        sCriteriaSpec = vecCriteria[0].GetSpecMin(VisionSurfaceCriteria_Column::Locus);
                        m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].locus.fMin = sCriteriaSpec.GetPassMax();
                        m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].locus.fMax = sCriteriaSpec.GetPassMin();
                        sCriteriaSpec = vecCriteria[0].GetSpecMin(VisionSurfaceCriteria_Column::Width);
                        m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].width.fMin = sCriteriaSpec.GetPassMax();
                        m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].width.fMax = sCriteriaSpec.GetPassMin();
                    }
                }
            }
        }

        if (m_visionUnit.m_pbySharedDevResult != NULL)
        {
            ConvertDeviceResultToBuffer(m_visionUnit.m_pbySharedDevResult, devResult, TRUE);
        }
        // Wait vision 11.08.20 KSY
        long nWaitCount = 0;

        while (m_visionUnit.m_ptrSharedMem->nQueueSize > 10)
        {
            if (nWaitCount > 50000 || m_visionUnit.m_ptrSharedMem->nQueueSize < 10)
                break;

            nWaitCount++;
            Sleep(1);
        }

        //SendMessage to iDL
        HWND hWnd = ::FindWindow(NULL, _T("iEasyAI"));
        if (hWnd == 0)
        {
            //AfxMessageBox(_T("윈도우를 찾지 못하였습니다."));
            m_visionUnit.m_Mutex.Unlock();
            return FALSE;
        }
        else
        {
            DWORD pid = ::GetCurrentProcessId();
            CString strLog;
            strLog.Format(_T("QueuePush = TrayID : %d, PocketID : %d, PaneID : %d"), nTrayID, nPocketID, nPaneID);
            LogMaster(_T("SendMVResultToiEasyAI"), strLog, 0, TRUE);
            m_visionUnit.m_ptrSharedMem->bAvailable
                = TRUE; // AI Runtime이 데이터가 Shared Memory에 들어왔는지 알아챈다.
            ::SendMessage(hWnd, MsgSendInspDataVsToDL, NULL, pid);
            m_visionUnit.m_Mutex.Unlock();

            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }
}

BOOL CDlgVisionInlineUI::SaveAllInspImage(VisionDeviceResult& devResult, const enSideVisionModule i_eSideVisionModule)
{
    VisionImageLotInsp& lotInspImage = m_visionUnit.getImageLotInsp();
    //long nFrmNum = lotInspImage.GetImageFrameCount();

    CString strFolderPath = _T("C:\\intekplus\\LearningImage\\");

    ::CreateDirectory(strFolderPath, NULL);

    strFolderPath = strFolderPath + _T("AllImages\\");
    ::CreateDirectory(strFolderPath, NULL);

    CString strFilePath = strFolderPath;
    CString strFileName, strFrameFolder, strAdd;

    if ((devResult.m_nTotalResult != NOT_MEASURED) && (devResult.m_nTotalResult != EMPTY))
    {
        long nImageSizeX, nImageSizeY, nDepth;

        for (long i = 0; i < lotInspImage.m_vecImages[i_eSideVisionModule].size(); i++)
        {
            strFrameFolder.Format(_T("Frame[%d]"), i);
            ::CreateDirectory(strFolderPath + strFrameFolder, NULL);
            strFileName = strFolderPath + strFrameFolder + _T("\\") + strFrameFolder + _T(".BMP");
            const char* file;
            file = (CStringA)strFileName;

            if (!::_access(file, 0))
            {
                long cnt = 0;
                strAdd.Format(_T("_%d"), cnt);
                strFileName = strFolderPath + strFrameFolder + _T("\\") + strFrameFolder + strAdd + _T(".BMP");
                file = (CStringA)strFileName;
                while (!::_access(file, 0))
                {
                    cnt++;
                    strAdd.Format(_T("_%d"), cnt);
                    strFileName = strFolderPath + strFrameFolder + _T("\\") + strFrameFolder + strAdd + _T(".BMP");
                    file = (CStringA)strFileName;
                }
            }
            else
            {
                strFileName = strFolderPath + strFrameFolder + _T("\\") + strFrameFolder + _T(".BMP");
            }

            nImageSizeX = lotInspImage.GetImageSizeX();
            nImageSizeY = lotInspImage.GetImageSizeY();

            if (nImageSizeX == 0 || nImageSizeY == 0)
                return FALSE;

            nDepth = (lotInspImage.m_vecImages[i_eSideVisionModule][i].GetWidthBytes() / nImageSizeX) * 8;
            BYTE* pbyImage = lotInspImage.m_vecImages[i_eSideVisionModule][i].GetMem();

            const Ipvm::Image8u image(nImageSizeX, nImageSizeY, pbyImage, nImageSizeX);
            Ipvm::ImageFile::SaveAsBmp(image, strFileName);
        }
    }

    return TRUE;
}

BOOL CDlgVisionInlineUI::CollectMVRejectData(
    VisionDeviceResult& devResult, const enSideVisionModule i_eSideVisionModule)
{
    int nRejectNum, i, frm, roi;
    //CString strFilePath = _T("");
    //IniHelper::SaveINT(strFilePath, _T("Vision_Setting"), _T("Send_Host_Frame_Num"), 0);

    VisionImageLotInsp& lotInspImage = m_visionUnit.getImageLotInsp();
    long nFrmNum = lotInspImage.GetImageFrameCount();
    ::CreateDirectory(_T("D:\\intekfiles"), NULL);
    CString strFolderPath = _T("D:\\intekfiles\\LearningImage\\");

    ::CreateDirectory(strFolderPath, NULL);

    strFolderPath = strFolderPath + _T("CollectedData\\");
    ::CreateDirectory(strFolderPath, NULL);

    CString strFilePath = strFolderPath;
    CString strFileName, strFrameFolder, strAdd, strLotStartDate, strLotStartTime;
    CString strSection, strKey, strValue;
    CString strLotID = m_visionInlineUI.m_pVisionMainAgent->m_lotID;
    CString strLotID_time;
    CString strJobName = m_visionUnit.GetJobFileName();
    BOOL bDeadbug = m_visionInlineUI.m_pVisionUnit->m_pPackageSpec->m_deadBug;

    CTime curTime = m_visionInlineUI.m_pVisionMainAgent->m_timeLotStart;

    long nYear = curTime.GetYear();
    long nMonth = curTime.GetMonth();
    long nDay = curTime.GetDay();
    long nHour = curTime.GetHour();
    long nMinute = curTime.GetMinute();
    long nSecond = curTime.GetSecond();

    strLotStartDate.Format(_T("%04d_%02d_%02d"), nYear, nMonth, nDay);
    strLotStartTime.Format(_T("%02d_%02d"), nHour, 0);

    if (strLotID.GetLength() <= 0)
    {
        strLotID = _T("BatchInsp_") + strLotStartTime;
        strLotID_time = _T("BatchInsp_") + strLotStartTime;
    }
    else
    {
        strLotID_time = strLotID + _T("_") + strLotStartTime;
    }
    strFolderPath = strFolderPath + strLotStartDate;
    ::CreateDirectory(strFolderPath, NULL);
    CString strLotPath = strFolderPath + _T("\\") + strLotID_time + _T("\\");
    ::CreateDirectory(strLotPath, NULL);

    long nNumInsp = (long)devResult.m_vecVisionInspResult.size();

    CString strInspName, strInspFolderPath, strInspIniFilePath, strFramePath;
    CString strImageFile, strFile, strText;

    long nImageWidth(0), nImageHeight(0), nImageDepth(8);

    const char* file;

    for (i = 0; i < nNumInsp; i++)
    {
        if (devResult.m_vecVisionInspResult[i]->m_totalResult == REJECT
            && (devResult.m_vecVisionInspResult[i]->m_hostReportCategory == HostReportCategory::SURFACE
                || devResult.m_vecVisionInspResult[i]->m_hostReportCategory == HostReportCategory::BALL
                || devResult.m_vecVisionInspResult[i]->m_hostReportCategory == HostReportCategory::COMPONENT))
        {
            nRejectNum = 0;
            strInspName = (*(devResult.m_vecVisionInspResult)[i]).m_inspName;
            strInspFolderPath = strLotPath + strInspName + _T("\\");
            strInspIniFilePath = strInspFolderPath + strInspName + (_T(".ini"));
            file = (CStringA)strInspFolderPath;

            strSection = _T("Public");

            if (::_access(file, 0))
            {
                ::CreateDirectory(strInspFolderPath, NULL);

                for (frm = 0; frm < nFrmNum; frm++)
                {
                    strFramePath.Format(strInspFolderPath + _T("\\Frame[%d]"), frm);
                    ::CreateDirectory(strFramePath, NULL);
                }

                // ini 파일 쓰기..
                strKey.Format(_T("TotalRejectNum"));
                IniHelper::SaveINT(strInspIniFilePath, strSection, strKey, 0);
            }

            strKey.Format(_T("TotalRejectNum"));
            nRejectNum = IniHelper::LoadINT(strInspIniFilePath, strSection, strKey, 0);

            strSection.Format(_T("InspProcess_%d"), nRejectNum);

            strKey.Format(_T("NumFrame"));
            IniHelper::SaveINT(strInspIniFilePath, strSection, strKey, nFrmNum);
            strKey.Format(_T("ImageWidth"));
            IniHelper::SaveINT(strInspIniFilePath, strSection, strKey, lotInspImage.GetImageSizeX());
            strKey.Format(_T("ImageHeight"));
            IniHelper::SaveINT(strInspIniFilePath, strSection, strKey, lotInspImage.GetImageSizeY());
            strKey.Format(_T("ImageDepth"));
            IniHelper::SaveINT(strInspIniFilePath, strSection, strKey, nImageDepth);
            strKey.Format(_T("Reject_Item_Name"));
            IniHelper::SaveSTRING(strInspIniFilePath, strSection, strKey, strInspName);
            strKey.Format(_T("VisionType"));
            IniHelper::SaveSTRING(strInspIniFilePath, strSection, strKey, bDeadbug == TRUE ? _T("BOTTOM") : _T("TOP"));

            strKey.Format(_T("LotStartTime"));
            strValue.Format(_T("%04d-%02d-%02dT%02d:%02d:%02d"), nYear, nMonth, nDay, nHour, nMinute, nSecond);
            IniHelper::SaveSTRING(strInspIniFilePath, strSection, strKey, strValue);
            strKey.Format(_T("LotID"));
            strValue = strLotID;
            IniHelper::SaveSTRING(strInspIniFilePath, strSection, strKey, strValue);
            strKey.Format(_T("JobName"));
            IniHelper::SaveSTRING(strInspIniFilePath, strSection, strKey, strJobName);

            for (frm = 0; frm < nFrmNum; frm++)
            {
                BYTE* pbyImage = lotInspImage.m_vecImages[i_eSideVisionModule][frm].GetMem();
                nImageWidth = lotInspImage.GetImageSizeX();
                nImageHeight = lotInspImage.GetImageSizeY();
                nImageDepth = 8;

                strFramePath.Format(strInspFolderPath + _T("\\Frame[%d]"), frm);
                strFile.Format(strFramePath + _T("\\") + strInspName + _T("_Frame%d") + _T("_[%d.%d.%d]"), frm, nHour,
                    nMinute, nSecond);
                strImageFile = strFile + _T(".BMP");
                file = (CStringA)strImageFile;
                if (!::_access(file, 0))
                {
                    long cnt = 0;
                    strText.Format(_T("%d"), cnt);
                    strImageFile = strFile + _T("_") + strText + _T(".BMP");

                    file = (CStringA)strImageFile;
                    while (!::_access(file, 0))
                    {
                        cnt++;
                        strText.Format(_T("%d"), cnt);
                        strImageFile = strFile + _T("_") + strText + _T(".BMP");
                        file = (CStringA)strImageFile;
                    }
                }

                const Ipvm::Image8u image(nImageWidth, nImageHeight, pbyImage, nImageWidth);
                Ipvm::ImageFile::SaveAsBmp(image, strFileName);

                strKey.Format(_T("Frame%d_ImagePath"), frm);
                IniHelper::SaveSTRING(strInspIniFilePath, strSection, strKey, strImageFile);
            }

            // Reject Information 쓰기
            long nRoiNum = (long)(*(devResult.m_vecVisionInspResult)[i]).vecrtRejectROI.size();
            strKey.Format(_T("FAIL_Item_%d_Roi"), nRejectNum);
            IniHelper::SaveINT(strInspIniFilePath, strSection, strKey, nRoiNum);
            for (roi = 0; roi < nRoiNum; roi++)
            {
                strKey.Format(_T("FAIL_Item_%d_Roi%d"), nRejectNum, roi);
                CRect rtROI = CRect((*(devResult.m_vecVisionInspResult)[i]).vecrtRejectROI[roi].TopLeft().m_x,
                    (*(devResult.m_vecVisionInspResult)[i]).vecrtRejectROI[roi].TopLeft().m_y,
                    (*(devResult.m_vecVisionInspResult)[i]).vecrtRejectROI[roi].BottomRight().m_x,
                    (*(devResult.m_vecVisionInspResult)[i]).vecrtRejectROI[roi].BottomRight().m_y);

                strValue.Format(_T("%d,%d,%d,%d"), rtROI.left, rtROI.top, rtROI.right, rtROI.bottom);
                IniHelper::SaveSTRING(strInspIniFilePath, strSection, strKey, strValue);
            }

            // TotalRejectNum에 1 더하기
            strSection = _T("Public");
            strKey.Format(_T("TotalRejectNum"));
            IniHelper::SaveINT(strInspIniFilePath, strSection, strKey, ++nRejectNum);
        }
    }

    return TRUE;
}

// 디버그 시퀀스용 임 이걸 이용해서 밖에서 핸들러스탑을 보낸다...
void CDlgVisionInlineUI::SetDebugStop(long& nSumResult, long nResult)
{
    if (!m_visionInlineUI.m_pVisionMainAgent->m_DebugStopOption->bUse)
    {
        return;
    }

    auto visionInspectionResults = m_visionInlineUI.m_pVisionUnit->GetVisionInspectionResults();

    // 영훈 [DebugStop] 20130808 : Host에서 전체 검사결과와 함게 보내도록 한다.
    // 앞에 5개는 Debug_Info로 보내므로 실제 검사항목은 5번부터가 시작이다.
    long nResultSize = (long)(visionInspectionResults.size());
    long nDebigStopSize = (long)(m_visionInlineUI.m_pVisionMainAgent->m_DebugStopOption->vecnItemCheckStatus.size());

    // 영훈 [DebugStop] 20130808 : Result 개수와 Host에서 보낸 개수가 다르면 그냥 종료한다.
    if ((nResultSize + DebugOptionData::DEBUG_END_) != nDebigStopSize)
        return;

    long nItemID = 0;

    for (long n = DebugOptionData::DEBUG_BEGIN_; n < nDebigStopSize; n++)
    {
        switch (n)
        {
            case DebugOptionData::DEBUG_EMPTY:
                if (nResult == EMPTY && m_visionInlineUI.m_pVisionMainAgent->m_DebugStopOption->vecnItemCheckStatus[n])
                {
                    nSumResult = 1;
                }
                continue;
            case DebugOptionData::DEBUG_INVALID:
                if (nResult == INVALID
                    && m_visionInlineUI.m_pVisionMainAgent->m_DebugStopOption->vecnItemCheckStatus[n])
                {
                    nSumResult = 1;
                }
                continue;
            case DebugOptionData::DEBUG_PASS:
                if (nResult == PASS && m_visionInlineUI.m_pVisionMainAgent->m_DebugStopOption->vecnItemCheckStatus[n])
                {
                    nSumResult = 1;
                }
                continue;
            case DebugOptionData::DEBUG_NOTPASS:
                if (nResult != PASS && m_visionInlineUI.m_pVisionMainAgent->m_DebugStopOption->vecnItemCheckStatus[n])
                {
                    nSumResult = 1;
                }
                continue;
            case DebugOptionData::DEBUG_UNCONDITIONAL:
                if (m_visionInlineUI.m_pVisionMainAgent->m_DebugStopOption->vecnItemCheckStatus[n])
                {
                    nSumResult = 1;
                }
                continue;
        }

        // 영훈 [Debug Stop] 02130808 : 위에서 이미 걸렸으면 밑에는 의미없다.
        if (nSumResult == 1)
        {
            return;
        }

        nItemID = n - DebugOptionData::DEBUG_END_; // 영훈 [DebugStop] 20130808 : Result는 0부터 시작한다.

        if (m_visionInlineUI.m_pVisionMainAgent->m_DebugStopOption->vecnItemCheckStatus[n]
            && (visionInspectionResults[nItemID]->m_totalResult == REJECT
                || visionInspectionResults[nItemID]->m_totalResult == EMPTY))
        {
            nSumResult = 1;
        }
    }
}

LRESULT CDlgVisionInlineUI::OnInspectionStart(WPARAM, LPARAM)
{
    m_imageLotView->SetScanIndexField(_T("***"));
    if (SystemConfig::GetInstance().Get_SaveSequenceAndInspectionTimeLog() == true)
    {
        m_sinlineSequenceLogInfo.Init();

        const auto& imageInfo = m_visionUnit.getImageLot().GetInfo();
        m_sinlineSequenceLogInfo.SetInlineSequenceTrayInfo(m_visionUnit.getImageLot().GetInspectionRepeatIndex(),
            imageInfo.m_trayIndex, imageInfo.m_scanAreaIndex, m_visionUnit.GetCurrentPaneID());
    }

    return 0;
}

LRESULT CDlgVisionInlineUI::OnInspectionEnd(WPARAM, LPARAM)
{
    m_imageLotView->SetScanIndexField(_T(""));

    m_csOverlay.Lock();
    auto overlayResult = *m_overlayResult;
    m_csOverlay.Unlock();

    overlayResult.Apply(m_imageLotView->GetCoreView());

    //mc_Inspection Log
    if (SystemConfig::GetInstance().Get_SaveSequenceAndInspectionTimeLog() == true)
    {
        SaveInspectionTimeLog();
    }
    //

    return 0;
}

void CDlgVisionInlineUI::ConvertDeviceResultToBuffer(BYTE* pbyBuffer, VisionDeviceResult& deviceResult, BOOL bSave)
{
    if (pbyBuffer == NULL)
        return;

    if (bSave)
    {
        memset(m_pbyDevResultSaveBuffer, 0, MAX_SEND_RESULT_SIZE);
        CMemFile memFile(m_pbyDevResultSaveBuffer, MAX_SEND_RESULT_SIZE, MAX_SEND_RESULT_SIZE);
        ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);
        deviceResult.Serialize(ar);
        ar.Close();

        memFile.SeekToBegin();
        BYTE* pByte = memFile.Detach();
        memcpy(pbyBuffer, pByte, MAX_SEND_RESULT_SIZE);

        if (pByte != m_pbyDevResultSaveBuffer)
        {
            ASSERT(!"A new memory allocation occurs.");
            ::free(pByte);
        }
    }
    else
    {
        CMemFile memFile;
        memFile.Attach((BYTE*)pbyBuffer, MAX_SEND_RESULT_SIZE);
        ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);
        deviceResult.Serialize(ar);
        ar.Close();
    }
}

void CDlgVisionInlineUI::SetColorImageForNGRV(Ipvm::Image8u3 image) //kircheis_NGRVINLINE
{
    VisionInspectionOverlayResult overlayResult;
    overlayResult.SetImage(image);
    overlayResult.Apply(m_imageLotView->GetCoreView());
}

static CCriticalSection g_ProcessingSaveLOG_CS;

void CDlgVisionInlineUI::SaveInspectionTimeLog()
{
    auto InlineLogInfo = m_sinlineSequenceLogInfo;
    sUnitinspectionLogInfo sInlineUnitInspectionLogInfo = m_visionUnit.GetUnitinspLogInfo();

    CFile File;
    CString strFileName("");
    g_ProcessingSaveLOG_CS.Lock();

    // 파일을 생성한다. 혹은 기존에 있으면 추가 한다.
    Ipvm::CreateDirectories(SystemConfig::GetInstance().Get_TimeLog_SaveFolderPath());
    CString strTimeLogRecipeFolderPath
        = SystemConfig::GetInstance().Get_TimeLog_SaveFolderPath() + CString(m_visionUnit.GetJobFileName());
    Ipvm::CreateDirectories(strTimeLogRecipeFolderPath);
    CString LotStartFromHost("");
    CTime Time = SystemConfig::GetInstance().Get_Lot_Start_TimeFromHost();
    LotStartFromHost.Format(
        _T("\\%d-%d-%d-%d-%d"), Time.GetYear(), Time.GetMonth(), Time.GetDay(), Time.GetHour(), Time.GetMinute());
    CString strTimeLogLotStartFolderPath = strTimeLogRecipeFolderPath + LotStartFromHost;
    Ipvm::CreateDirectories(strTimeLogLotStartFolderPath);
    CString strTimeLogLotIDFolderPath
        = strTimeLogLotStartFolderPath + _T("\\") + SystemConfig::GetInstance().Get_Lot_IDFromHost();
    Ipvm::CreateDirectories(strTimeLogLotIDFolderPath);
    CString strTimeLogSavePath = strTimeLogLotIDFolderPath + _T("\\");

    SystemConfig::GetInstance().Set_TimeLog_SavePath(strTimeLogSavePath);

    strFileName.Format(_T("%sVisionProcessingDebugLog(%s)_Thread_%d.txt"), (LPCTSTR)strTimeLogSavePath,
        (LPCTSTR)SystemConfig::GetInstance().m_strVisionInfo, sInlineUnitInspectionLogInfo.m_nThreadindex);

    if (!File.Open(strFileName, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite))
    {
        g_ProcessingSaveLOG_CS.Unlock();
        return;
    }
    else
    {
        // 파일의 맨 마지막을 찾는다.
        File.SeekToEnd();

        CArchive ar(&File, CArchive::store);

        const long nInlineSequenceLogNum = (long)InlineLogInfo.m_vecmapInlineSequenceTimeLog.size();
        const long nVisionUnitLogNum = (long)sInlineUnitInspectionLogInfo.m_vecProcessExcuteTimeLog.size();
        CString strWriteTime("");
        CString strWriteLog("");

        bool bNeedWriteProcessingLog(true); //중복으로 쓰는걸 방지하는 변수
        bool bNeedWriteVisionUnitLog(true); //중복으로 쓰는걸 방지하는 변수

        for (long nInlineUILogidx = 0; nInlineUILogidx < nInlineSequenceLogNum; nInlineUILogidx++)
        {
            if (nInlineUILogidx == 0)
            {
                strWriteLog.Format(_T("TrayID:%d, ScanID:%d, Pane:%d, Repeatidx:%d\n"), InlineLogInfo.m_nTrayindex,
                    InlineLogInfo.m_nScanID, InlineLogInfo.m_nPaneIndex, InlineLogInfo.m_nInspectionRepeatindex);
                ar.WriteString(strWriteLog);
                strWriteLog.Empty();
            }

            strWriteTime.Format(_T("[%s]\t"), (LPCTSTR)CTime::GetCurrentTime().Format(_T("%H:%M:%S")));

            ar.WriteString(strWriteTime);

            strWriteTime.Empty();

            auto InlineUILog = InlineLogInfo.m_vecmapInlineSequenceTimeLog[nInlineUILogidx];

            for (auto InlineUIWriteData = InlineUILog.begin(); InlineUIWriteData != InlineUILog.end();
                InlineUIWriteData++)
            {
                strWriteLog.Format(
                    _T(",%s,%s\n"), (LPCTSTR)InlineUIWriteData->first, (LPCTSTR)InlineUIWriteData->second);
                ar.WriteString(strWriteLog);
                strWriteLog.Empty();
            }

            if (bNeedWriteVisionUnitLog == true)
            {
                for (long nVisionUnitLogidx = 0; nVisionUnitLogidx < nVisionUnitLogNum; nVisionUnitLogidx++)
                {
                    strWriteTime.Format(_T("[%s]\t"), (LPCTSTR)CTime::GetCurrentTime().Format(_T("%H:%M:%S")));

                    ar.WriteString(strWriteTime);

                    strWriteTime.Empty();

                    auto VisionUnitLog = sInlineUnitInspectionLogInfo.m_vecProcessExcuteTimeLog[nVisionUnitLogidx];

                    for (auto VisionUnitWriteData = VisionUnitLog.begin(); VisionUnitWriteData != VisionUnitLog.end();
                        VisionUnitWriteData++)
                    {
                        strWriteLog.Format(
                            _T(",%s,%s\n"), (LPCTSTR)VisionUnitWriteData->first, (LPCTSTR)VisionUnitWriteData->second);
                        ar.WriteString(strWriteLog);
                        strWriteLog.Empty();
                    }

                    if (bNeedWriteProcessingLog == true)
                    {
                        CString strWriteModuleName("");
                        auto ProcessingModules = sInlineUnitInspectionLogInfo.m_mapProcessingFunctionExcuteTimeLog;

                        for (auto ProcessingModulekey = ProcessingModules.begin();
                            ProcessingModulekey != ProcessingModules.end(); ProcessingModulekey++)
                        {
                            const long nCurProcessingModuleLogSize
                                = (long)ProcessingModules[ProcessingModulekey->first].size(); //Item LogSize

                            if (nCurProcessingModuleLogSize == 0)
                                continue;

                            //strWriteModuleName.Format(_T("%s\n"), ProcessingModulekey->first); //ModuleName
                            //ar.WriteString(strWriteModuleName);
                            //strWriteModuleName.Empty();

                            strWriteTime.Format(_T("[%s]\t"), (LPCTSTR)CTime::GetCurrentTime().Format(_T("%H:%M:%S")));

                            for (long nCurProcessingModuleLogidx = 0;
                                nCurProcessingModuleLogidx < nCurProcessingModuleLogSize; nCurProcessingModuleLogidx++)
                            {
                                for (auto CurProcessingModule
                                    = ProcessingModules[ProcessingModulekey->first][nCurProcessingModuleLogidx].begin();
                                    CurProcessingModule
                                    != ProcessingModules[ProcessingModulekey->first][nCurProcessingModuleLogidx].end();
                                    CurProcessingModule++)
                                {
                                    if (nCurProcessingModuleLogidx == 0) //0일경우에만 시간을 기록한다
                                    {
                                        strWriteLog.Format(_T("%s,%s,%s,%s"), (LPCTSTR)strWriteTime,
                                            (LPCTSTR)ProcessingModulekey->first, (LPCTSTR)CurProcessingModule->first,
                                            (LPCTSTR)CurProcessingModule->second);
                                        strWriteTime.Empty();
                                    }
                                    else
                                    {
                                        CString strCurProceesingLogData("");
                                        strCurProceesingLogData.Format(_T(",%s,%s"),
                                            (LPCTSTR)CurProcessingModule->first, (LPCTSTR)CurProcessingModule->second);

                                        strWriteLog += strCurProceesingLogData;
                                    }

                                    if (nCurProcessingModuleLogidx
                                        == nCurProcessingModuleLogSize - 1) //마지막 줄이라면 줄바꿈을 해야하니..
                                    {
                                        strWriteLog += _T("\n");
                                        ar.WriteString(strWriteLog);
                                        strWriteLog.Empty();
                                    }
                                }
                            }
                        }

                        bNeedWriteProcessingLog = false;
                    }
                }

                bNeedWriteVisionUnitLog = false;
            }
        }

        ar.Close();
        File.Close();
    }

    g_ProcessingSaveLOG_CS.Unlock();
}
