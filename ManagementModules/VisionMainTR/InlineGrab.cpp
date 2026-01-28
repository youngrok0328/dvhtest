//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "InlineGrab.h"

//CPP_2_________________________________ This project's headers
#include "FOVImageGrabber.h"
#include "FOVImageGrabber2D.h"
#include "FOVImageGrabber3D.h"
#include "FOVImageGrabberSide.h"
#include "FOVImageGrabberTR.h"
#include "ImageClassifier.h"
#include "InspectionOrderer.h"
#include "VisionMainTR.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../HardwareModules/dPI_Framegrabber/Framegrabber.h"
#include "../../HardwareModules/dPI_SyncController/SyncController.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../ManagementModules/VisionUnitTR/VisionUnit.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/ImageFile.h>
#include <Ipvm/Gadget/Miscellaneous.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
InlineGrab::InlineGrab(VisionMainTR* main, long grabBufferNum, VisionUnit** unitBuffers, long unitNum)
    : m_main(main)
    , m_grabBufferNum(grabBufferNum)
    , m_trayIndexFromHost(0)
    , m_runCntFromHost(0)
    , m_isBatchInspection(false)
    , m_fovImageGrabber(nullptr)
{
    if (SystemConfig::GetInstance().IsHardwareExist())
    {
        switch (SystemConfig::GetInstance().GetVisionType())
        {
            case VISIONTYPE_2D_INSP:
                m_fovImageGrabber = new FOVImageGrabber2D(main);
                break;
            case VISIONTYPE_3D_INSP:
                m_fovImageGrabber = new FOVImageGrabber3D(main);
                break;
            case VISIONTYPE_SIDE_INSP:
                m_fovImageGrabber = new FOVImageGrabberSide(main);
                break;
            case VISIONTYPE_TR:
                m_fovImageGrabber = new FOVImageGrabberTR(main);
                break;
            default:
                ASSERT(!_T("??"));
                break;
        }
    }

    m_imageClassifier = new ImageClassifier(main, m_fovImageGrabber, m_grabBufferNum);
    m_inspectionOrderer = new InspectionOrderer(main, m_imageClassifier, unitBuffers, unitNum);

    m_main->GetPrimaryVisionUnit().setImageLot(std::make_shared<VisionImageLot>());

    m_Signal_KillThread = ::CreateEvent(NULL, FALSE, FALSE, NULL);
    m_Flag_thread_idle = ::CreateEvent(NULL, TRUE, TRUE, NULL);
    m_Flag_thread_inlineMode = ::CreateEvent(NULL, TRUE, FALSE, NULL);
    m_Flag_thread_offline = ::CreateEvent(NULL, TRUE, TRUE, NULL);
    m_Handle_Thread = NULL;

    CWinThread* pGrabThread = nullptr;
    pGrabThread = ::AfxBeginThread(ThreadGrabFunc, (void*)this, THREAD_PRIORITY_HIGHEST);
    if (pGrabThread == NULL)
    {
        ::SimpleMessage(_T("Failed to create Grab Thread\nPlease restart S.W"), MB_OK);
        return;
    }
    HANDLE hNewThread = pGrabThread->m_hThread;
    ::DuplicateHandle(
        ::GetCurrentProcess(), hNewThread, ::GetCurrentProcess(), &m_Handle_Thread, 0, FALSE, DUPLICATE_SAME_ACCESS);

    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP)
    {
        CWinThread* pNGRVSaveThread = nullptr;
        pNGRVSaveThread = ::AfxBeginThread(ThreadNGRVSaveFunc, (void*)this, THREAD_PRIORITY_NORMAL);
        if (pNGRVSaveThread == NULL)
        {
            ::SimpleMessage(_T("Failed to create NGRV Save Thread\nPlease restart S.W"), MB_OK);
            return;
        }
        HANDLE hNGRVThread = pNGRVSaveThread->m_hThread;
        //HANDLE hNGRVThread = ::AfxBeginThread(ThreadNGRVSaveFunc, (void*)this, THREAD_PRIORITY_NORMAL)->m_hThread;
        ::DuplicateHandle(::GetCurrentProcess(), hNGRVThread, ::GetCurrentProcess(), &m_Handle_Save_Thread, 0, FALSE,
            DUPLICATE_SAME_ACCESS);
    }
}

InlineGrab::~InlineGrab()
{
    ::SetEvent(m_Signal_KillThread);

    if (::WaitForSingleObject(m_Handle_Thread, 10000) != WAIT_OBJECT_0)
        ::TerminateThread(m_Handle_Thread, 555);

    ::CloseHandle(m_Signal_KillThread);
    ::CloseHandle(m_Flag_thread_idle);
    ::CloseHandle(m_Flag_thread_inlineMode);
    ::CloseHandle(m_Flag_thread_offline);
    ::CloseHandle(m_Handle_Thread);

    delete m_inspectionOrderer;
    delete m_imageClassifier;
    delete m_fovImageGrabber;
}

void InlineGrab::initialize2D(long totalFrameCount, long highPosFrameCount)
{
    DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab, _T("InlineGrab::initialize2D)"));

    CheckChangedVisionTrayScanSpec();

    const auto& trayScanSpec = m_main->GetPrimaryVisionUnit().getTrayScanSpec();

    long stitchCountX = max(1, trayScanSpec.GetMaxStitchCountX());
    long stitchCountY = max(1, trayScanSpec.GetMaxStitchCountY());
    long sensorSizeX = FrameGrabber::GetInstance().get_grab_image_width();
    long sensorSizeY = FrameGrabber::GetInstance().get_grab_image_height();

    m_imageBufferSize = VisionImageLot::Get2DMaxBufferSize(sensorSizeX, sensorSizeY, stitchCountX, stitchCountY);

    for (long grabBufferIndex = 0; grabBufferIndex < m_grabBufferNum; grabBufferIndex++)
    {
        auto grabBuffer = m_imageClassifier->getBuffer(grabBufferIndex)->m_imageLot;
        grabBuffer->Setup2D(sensorSizeX, sensorSizeY, m_imageBufferSize, totalFrameCount);
    }

    m_main->GetPrimaryVisionUnit().getImageLot().Setup2D(sensorSizeX, sensorSizeY, m_imageBufferSize, totalFrameCount);

    if (m_fovImageGrabber != nullptr)
    {
        (dynamic_cast<FOVImageGrabber2D*>(m_fovImageGrabber))->allocateBuffer(totalFrameCount, highPosFrameCount);
    }

    m_needCountForFOV.clear();
    m_needCountForFOV.resize(trayScanSpec.m_vecFovInfo.size(), 0);
    for (int inspectionIndex = 0; inspectionIndex < trayScanSpec.GetInspectionItemCount(); inspectionIndex++)
    {
        auto& inspectionItem = trayScanSpec.GetInspectionItem(inspectionIndex);

        for (auto& fovIndex : inspectionItem.m_fovList)
        {
            m_needCountForFOV[fovIndex]++;
        }
    }
}

void InlineGrab::initialize3D(long logicalScanLength_px)
{
    DevelopmentLog::AddLog(
        DevelopmentLog::Type::ImageGrab, _T("InlineGrab::initialize3D Length:%dpx"), logicalScanLength_px);

    CheckChangedVisionTrayScanSpec();

    auto& trayScanSpec = m_main->GetPrimaryVisionUnit().getTrayScanSpec();

    long stitchCountY = trayScanSpec.GetMaxStitchCountY();
    long sensorSizeX = FrameGrabber::GetInstance().get_grab_image_width();
    long sensorSizeY = FrameGrabber::GetInstance().get_grab_image_height();

    m_imageBufferSize = VisionImageLot::Get3DMaxBufferSize(sensorSizeX, stitchCountY, logicalScanLength_px);

    for (long grabBufferIndex = 0; grabBufferIndex < m_grabBufferNum; grabBufferIndex++)
    {
        auto grabBuffer = m_imageClassifier->getBuffer(grabBufferIndex)->m_imageLot;
        grabBuffer->Setup3D(sensorSizeX, sensorSizeY, m_imageBufferSize);
    }

    m_main->GetPrimaryVisionUnit().getImageLot().Setup3D(sensorSizeX, sensorSizeY, m_imageBufferSize);

    if (m_fovImageGrabber != nullptr)
    {
        (dynamic_cast<FOVImageGrabber3D*>(m_fovImageGrabber))->allocateBuffer(logicalScanLength_px);
    }

    m_needCountForFOV.clear();
    m_needCountForFOV.resize(trayScanSpec.m_vecFovInfo.size(), 0);

    for (int inspectionIndex = 0; inspectionIndex < trayScanSpec.GetInspectionItemCount(); inspectionIndex++)
    {
        auto& inspectionItem = trayScanSpec.GetInspectionItem(inspectionIndex);

        for (auto& fovIndex : inspectionItem.m_fovList)
        {
            m_needCountForFOV[fovIndex]++;
        }
    }

    for (long fovIndex = 0; fovIndex < (long)m_needCountForFOV.size(); fovIndex++)
    {
        DevelopmentLog::AddLog(
            DevelopmentLog::Type::ImageGrab, _T("Fov%d - reference:%d"), fovIndex, m_needCountForFOV[fovIndex]);
    }

    for (long inspIndex = 0; inspIndex < (long)trayScanSpec.GetInspectionItemCount(); inspIndex++)
    {
        auto& inspItem = trayScanSpec.GetInspectionItem(inspIndex);
        for (auto& unitIndex : inspItem.m_unitIndexList)
        {
            DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab, _T("Insp%d - Unit:%d"), inspIndex, unitIndex);
        }
    }
}

void InlineGrab::initializeColorVision(long totalFrameCount, long highPosFrameCount)
{
    DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab, _T("InlineGrab::initializeColorVision)"));

    CheckChangedVisionTrayScanSpec();

    const auto& trayScanSpec = m_main->GetPrimaryVisionUnit().getTrayScanSpec();

    auto& vecPackageGrabInfo = m_main->m_vecPackageGrabInfo;

    long stitchCountX;
    long stitchCountY;

    if (!vecPackageGrabInfo.empty() && !vecPackageGrabInfo[0].m_vecGrabPointInfo.empty())
    {
        stitchCountX = vecPackageGrabInfo[0].m_vecGrabPointInfo[0].m_nStitchingCountX;
        stitchCountY = vecPackageGrabInfo[0].m_vecGrabPointInfo[0].m_nStitchingCountY;
    }
    else
    {
        stitchCountX = 1;
        stitchCountY = 1;
    }

    long sensorSizeX = FrameGrabber::GetInstance().get_grab_image_width();
    long sensorSizeY = FrameGrabber::GetInstance().get_grab_image_height();

    m_imageBufferSize = VisionImageLot::Get2DMaxBufferSize(sensorSizeX, sensorSizeY, stitchCountX, stitchCountY);

    for (long grabBufferIndex = 0; grabBufferIndex < m_grabBufferNum; grabBufferIndex++)
    {
        auto grabBuffer = m_imageClassifier->getBuffer(grabBufferIndex)->m_imageLot;
        grabBuffer->SetupColorVision(sensorSizeX, sensorSizeY, m_imageBufferSize, totalFrameCount);
    }

    m_main->GetPrimaryVisionUnit().getImageLot().SetupColorVision(
        sensorSizeX, sensorSizeY, m_imageBufferSize, totalFrameCount);

    m_needCountForFOV.clear();
    m_needCountForFOV.resize(trayScanSpec.m_vecFovInfo.size(), 0);
    for (int inspectionIndex = 0; inspectionIndex < trayScanSpec.GetInspectionItemCount(); inspectionIndex++)
    {
        auto& inspectionItem = trayScanSpec.GetInspectionItem(inspectionIndex);

        for (auto& fovIndex : inspectionItem.m_fovList)
        {
            m_needCountForFOV[fovIndex]++;
        }
    }
}

void InlineGrab::initializeSide(long totalFrameCount, long highPosFrameCount)
{
    DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab, _T("InlineGrab::initialize Side)"));

    CheckChangedVisionTrayScanSpec();

    const auto& trayScanSpec = m_main->GetPrimaryVisionUnit().getTrayScanSpec();

    long stitchCountX = max(1, trayScanSpec.GetMaxStitchCountX());
    long stitchCountY = max(1, trayScanSpec.GetMaxStitchCountY());
    long sensorSizeX = FrameGrabber::GetInstance().get_grab_image_width();
    long sensorSizeY = FrameGrabber::GetInstance().get_grab_image_height();

    m_imageBufferSize = VisionImageLot::Get2DMaxBufferSize(sensorSizeX, sensorSizeY, stitchCountX, stitchCountY);

    //mc_임시
    for (long nSideidx = enSideVisionModule::SIDE_VISIONMODULE_START;
        nSideidx < enSideVisionModule::SIDE_VISIONMODULE_END; nSideidx++)
    {
        for (long grabBufferIndex = 0; grabBufferIndex < m_grabBufferNum; grabBufferIndex++)
        {
            auto grabBuffer = m_imageClassifier->getBuffer(grabBufferIndex)->m_imageLot;
            grabBuffer->Setup2D(
                sensorSizeX, sensorSizeY, m_imageBufferSize, totalFrameCount, enSideVisionModule(nSideidx));
        }

        m_main->GetPrimaryVisionUnit().getImageLot().Setup2D(
            sensorSizeX, sensorSizeY, m_imageBufferSize, totalFrameCount, enSideVisionModule(nSideidx));

        if (m_fovImageGrabber != nullptr)
        {
            (dynamic_cast<FOVImageGrabberSide*>(m_fovImageGrabber))->allocateBuffer(totalFrameCount, highPosFrameCount);
        }

        m_needCountForFOV.clear();
        m_needCountForFOV.resize(trayScanSpec.m_vecFovInfo.size(), 0);
        for (int inspectionIndex = 0; inspectionIndex < trayScanSpec.GetInspectionItemCount(); inspectionIndex++)
        {
            auto& inspectionItem = trayScanSpec.GetInspectionItem(inspectionIndex);

            for (auto& fovIndex : inspectionItem.m_fovList)
            {
                m_needCountForFOV[fovIndex]++;
            }
        }
    }
}

void InlineGrab::initializeTR(long totalFrameCount, long highPosFrameCount)
{
    DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab, _T("InlineGrab::initializeTR)"));

    CheckChangedVisionTrayScanSpec();

    const auto& trayScanSpec = m_main->GetPrimaryVisionUnit().getTrayScanSpec();

    long stitchCountX = max(1, trayScanSpec.GetMaxStitchCountX());
    long stitchCountY = max(1, trayScanSpec.GetMaxStitchCountY());
    long sensorSizeX = FrameGrabber::GetInstance().get_grab_image_width();
    long sensorSizeY = FrameGrabber::GetInstance().get_grab_image_height();

    m_imageBufferSize = VisionImageLot::Get2DMaxBufferSize(sensorSizeX, sensorSizeY, stitchCountX, stitchCountY);

    for (long grabBufferIndex = 0; grabBufferIndex < m_grabBufferNum; grabBufferIndex++)
    {
        auto grabBuffer = m_imageClassifier->getBuffer(grabBufferIndex)->m_imageLot;
        grabBuffer->Setup2D(sensorSizeX, sensorSizeY, m_imageBufferSize, totalFrameCount);
    }

    m_main->GetPrimaryVisionUnit().getImageLot().Setup2D(sensorSizeX, sensorSizeY, m_imageBufferSize, totalFrameCount);

    if (m_fovImageGrabber != nullptr)
    {
        (dynamic_cast<FOVImageGrabber2D*>(m_fovImageGrabber))->allocateBuffer(totalFrameCount, highPosFrameCount);
    }

    m_needCountForFOV.clear();
    m_needCountForFOV.resize(trayScanSpec.m_vecFovInfo.size(), 0);
    for (int inspectionIndex = 0; inspectionIndex < trayScanSpec.GetInspectionItemCount(); inspectionIndex++)
    {
        auto& inspectionItem = trayScanSpec.GetInspectionItem(inspectionIndex);

        for (auto& fovIndex : inspectionItem.m_fovList)
        {
            m_needCountForFOV[fovIndex]++;
        }
    }
}

InspectionOrderer& InlineGrab::getInspectionOrderer()
{
    return *m_inspectionOrderer;
}

long InlineGrab::GetTrayIndex() const
{
    return m_trayIndexFromHost;
}

long InlineGrab::GetInlineRunCnt() const
{
    return m_runCntFromHost;
}

bool InlineGrab::CopyLastGrabBufferTo(VisionImageLot& imageLot)
{
    return m_inspectionOrderer->copyFromLastestCalculationImage(imageLot);
}

bool InlineGrab::PushBatchImage(long batchKey, LPCTSTR imageFilePath, long paneIndex, LPCTSTR batchBarcode)
{
    m_csBatchFile.Lock();
    if (!m_batchFilePath.m_filePath.IsEmpty())
    {
        m_csBatchFile.Unlock();
        return false;
    }

    m_batchFilePath.m_filePath = imageFilePath;
    m_batchFilePath.m_paneIndex = paneIndex;
    m_batchFilePath.m_batchKey = batchKey;
    m_batchFilePath.m_batchBarcode = batchBarcode;
    m_csBatchFile.Unlock();

    return true;
}

void InlineGrab::SetScanTrayIndex(long trayIndex)
{
    DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab, _T("InlineGrab::SetScanTrayIndex Tray:%d"), trayIndex);

    //------------------------------------------------------------------------------------------------
    // 인라인중에 호출되는 함수이다
    // 새로운 Tray 가 들어왔음을 알려준다.
    //------------------------------------------------------------------------------------------------

    m_trayIndexFromHost = trayIndex;
    m_trayKey = ::CreateGUID(); // Tray 고유 ID

    const auto& trayScanSpec = m_main->GetPrimaryVisionUnit().getTrayScanSpec();

    //------------------------------------------------------------------------------------------------
    // 지금까지 완성할 수 없는 Inspection Image Buffer는 더 이상 완성될 가능성이 없다
    // 이미 다른 Tray 검사로 넘어갔기 때문이다. 이제 파기하자
    //------------------------------------------------------------------------------------------------

    m_imageClassifier->removeIncompleteImages(trayScanSpec);

    //------------------------------------------------------------------------------------------------
    // Image 분류기에 디바이스 리스트를 추가한다
    //------------------------------------------------------------------------------------------------

    for (long inspectionID = 0; inspectionID < trayScanSpec.GetInspectionItemCount(); inspectionID++)
    {
        const auto& inspection = trayScanSpec.GetInspectionItem(inspectionID);
        std::vector<long> vecStitchList;

        if (inspection.m_fovList.size() > 0)
        {
            // FOV Index를 Stitch Index로 변경
            auto firstFovIndex = inspection.m_fovList[0];

            for (auto& fovIndex : inspection.m_fovList)
            {
                vecStitchList.push_back(fovIndex - firstFovIndex);
            }
        }

        m_imageClassifier->addCandidateDevice(std::make_shared<ImageClassifier_Info>(m_trayKey, vecStitchList,
            m_runCntFromHost, m_trayIndexFromHost, inspection.m_id, (long)inspection.m_unitIndexList.size()));
    }
}

void InlineGrab::SetInlineRunCnt(long inlineRunCnt)
{
    m_runCntFromHost = inlineRunCnt;
}

void InlineGrab::Start(bool isBatchInspection)
{
    m_isBatchInspection = isBatchInspection;

    ::ResetEvent(m_Flag_thread_offline);
    ::SetEvent(m_Flag_thread_inlineMode);
}

void InlineGrab::Stop()
{
    if (m_isBatchInspection)
    {
        while (1)
        {
            m_csBatchFile.Lock();
            bool batchFileExist = m_batchFilePath.m_filePath.IsEmpty() ? false : true;
            m_csBatchFile.Unlock();

            if (!batchFileExist)
            {
                break;
            }

            // Batch File로 들어간 모든 항목이 다 처리되길 기다린다
            Sleep(100);
        }

        m_isBatchInspection = false;
    }

    ::SetEvent(m_Flag_thread_offline);
    WaitForSingleObject(m_Flag_thread_idle, INFINITE);

    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP)
    {
        FrameGrabber::GetInstance().CloseCameraComm(); // Inline Mode가 끝나면 Camera 통신을 닫는다 - JHB_NGRV
    }
}

bool InlineGrab::IsInlineMode()
{
    if (m_isBatchInspection)
        return false;
    return ::WaitForSingleObject(m_Flag_thread_inlineMode, 0) == WAIT_OBJECT_0;
}

bool InlineGrab::isIdleForBatch()
{
    m_csBatchFile.Lock();
    bool bathFileIsEmpty = m_batchFilePath.m_filePath.IsEmpty();

    m_csBatchFile.Unlock();

    if (bathFileIsEmpty && m_imageClassifier->isAllIdleExceptCandidate())
    {
        return m_inspectionOrderer->isAllInspectionThreadsIdle();
    }

    m_csBatchFile.Unlock();

    return false;
}

UINT InlineGrab::ThreadGrabFunc(LPVOID pParam)
{
    InlineGrab* control = (InlineGrab*)pParam;

    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP)
    {
        return control->ThreadGrabFunc_NGRV();
    }

    return control->ThreadGrabFunc();
}

UINT InlineGrab::ThreadGrabFunc()
{
    if (SystemConfig::GetInstance().Get_is_iGrabFirmware_and_LibraryVersion_Mismatch() == true
        && SystemConfig::GetInstance().IsHardwareExist() == TRUE) //Hardware 사용시 조건문 추가
    {
        UINT Cur_FirmwareVersion = 0x00, Need_FirmwareVersion = 0x00;
        UINT Cur_LibraryVersion = 0x00, Need_LibraryVersion = 0x00;

        FrameGrabber::GetInstance().get_firmware_version(0, Cur_FirmwareVersion, Need_FirmwareVersion);
        FrameGrabber::GetInstance().get_library_version(Cur_LibraryVersion, Need_LibraryVersion);

        CString strCurVersionInfo
            = FrameGrabber::GetInstance().GetstrVersioninfo(Cur_FirmwareVersion, Cur_LibraryVersion);
        CString strNeedVersionInfo
            = FrameGrabber::GetInstance().GetstrVersioninfo(Need_FirmwareVersion, Need_LibraryVersion);

        CString strErrorMsg{};
        strErrorMsg.Format(_T("Version is mismatch!!\r\nLoaded version : %s\r\nExpected version : Higher than ")
                           _T("%s\r\nCannot normal running Vision S/W!!!!"),
            (LPCTSTR)strCurVersionInfo, (LPCTSTR)strNeedVersionInfo); //Vision Type을 붙여줘야 하나..

        ::SimpleMessage(strErrorMsg, MB_OK);

        return 0;
    }

    HANDLE phGrabEnable[] = {m_Flag_thread_inlineMode, m_Signal_KillThread};
    HANDLE phGrabDisable[] = {m_Flag_thread_offline, m_Signal_KillThread};

    while (1)
    {
        SyncController::GetInstance().SetReadySignal(FALSE);
        SetEvent(m_Flag_thread_idle);

        const DWORD dwRetEnable
            = ::WaitForMultipleObjects(sizeof(phGrabEnable) / sizeof(HANDLE), phGrabEnable, FALSE, INFINITE);

        if (dwRetEnable != WAIT_OBJECT_0)
        {
            // Kill 메시지
            return 0;
        }

        ResetEvent(m_Flag_thread_idle);

        long sequenceStep = 0;
        long grabBufferIndex = -1;

        while (
            ::WaitForMultipleObjects(sizeof(phGrabDisable) / sizeof(HANDLE), phGrabDisable, FALSE, 0) == WAIT_TIMEOUT)
        {
            bool exit = false;

            if (m_isBatchInspection)
            {
                // Batch Inspection 모드일 때
                if (!ThreadGrabFunc_Batch())
                    exit = true;
                continue;
            }

            if (!SystemConfig::GetInstance().m_bHardwareExist)
            {
                // 하드웨어가 없을 때 동작은 없앴다.
                // 제대로 동작하지도 않을 것 같고, 필요하면 그때 다시 제대로 만들자
                continue;
            }

            // 하드웨어를 통해 영상 획득 모드일 때

            switch (sequenceStep)
            {
                case 0:
                    // Grab Buffer를 얻는다
                    if (!m_fovImageGrabber->getIdleBufferIndex(
                            m_Flag_thread_offline, m_Signal_KillThread, grabBufferIndex))
                    {
                        // Kill Signal 나 OffLine 명령을 받은 것이므로 나간다.
                        exit = true;
                        break;
                    }
                    else
                    {
                        if (grabBufferIndex < 0)
                        {
                            // Grab Buffer 로 할당된게 없는 것이다
                            // Job이 로드되어 있지 않은 상태라면,
                            // 필요한 버퍼수가 할당되어 있지 않아서 이런 현상이 있을 수 있다.

                            Sleep(100);
                        }
                        else
                        {
                            // Grab Buffer를 얻었다면 Ready 상태로 바꾼다
                            SyncController::GetInstance().SetReadySignal(TRUE);
                            sequenceStep++;
                            SystemConfig::GetInstance().Save_SequenceTimeLog(
                                _T("InlineGrab[ThreadGrabFunc]_ReadySignal On"));
                        }
                    }
                    break;

                case 1:
                    // Host Start Signal 이 들어올 때를 기다린다.
                    // Start 신호가 들어올 때까지 루프.
                    if (SyncController::GetInstance().GetStartSignal() == FALSE)
                    {
                        Sleep(1);
                    }
                    else
                    {
                        sequenceStep++;

                        SystemConfig::GetInstance().Save_SequenceTimeLog(
                            _T("InlineGrab[ThreadGrabFunc]_WaitStartSignal"));
                    }
                    break;

                case 2:
                    // Grab을 수행한다
                    ImageGrab(grabBufferIndex);

                    // Grab이 모두 끝났다.
                    // 혹시 모르니 Ready Signal을 꺼주고 처음 스탭으로 돌아가자
                    SyncController::GetInstance().SetReadySignal(FALSE);
                    sequenceStep = 0;
                    break;
            }

            if (exit)
                break;
        }

        if (grabBufferIndex >= 0)
        {
            // 얻었던 Grab Buffer를 다시 Idle 상태로 바꾸어 준다
            m_fovImageGrabber->setIdleBuffer(grabBufferIndex);
        }

        // Inline모드를 Off시킨다
        ::ResetEvent(m_Flag_thread_inlineMode);
    }

    return 0;
}

UINT InlineGrab::ThreadGrabFunc_NGRV()
{
    if (!SystemConfig::GetInstance().m_bHardwareExist)
    {
        // 하드웨어가 없을 때 동작은 없앴다.
        // 제대로 동작하지도 않을 것 같고, 필요하면 그때 다시 제대로 만들자
        return FALSE;
    }

    if (SystemConfig::GetInstance().Get_is_iGrabFirmware_and_LibraryVersion_Mismatch() == true
        && SystemConfig::GetInstance().IsHardwareExist() == TRUE) //Hardware 사용시 조건문 추가
    {
        UINT Cur_FirmwareVersion = 0x00, Need_FirmwareVersion = 0x00;
        UINT Cur_LibraryVersion = 0x00, Need_LibraryVersion = 0x00;

        FrameGrabber::GetInstance().get_firmware_version(0, Cur_FirmwareVersion, Need_FirmwareVersion);
        FrameGrabber::GetInstance().get_library_version(Cur_LibraryVersion, Need_LibraryVersion);

        CString strCurVersionInfo
            = FrameGrabber::GetInstance().GetstrVersioninfo(Cur_FirmwareVersion, Cur_LibraryVersion);
        CString strNeedVersionInfo
            = FrameGrabber::GetInstance().GetstrVersioninfo(Need_FirmwareVersion, Need_LibraryVersion);

        CString strErrorMsg("");

        strErrorMsg.Format(_T("Version is mismatch!!\r\nLoaded version : %s\r\nExpected version : Higher than %s\r\n ")
                           _T("Cannot normal running Vision S/W!!!!"),
            (LPCTSTR)strCurVersionInfo, (LPCTSTR)strNeedVersionInfo); //Vision Type을 붙여줘야 하나..

        ::SimpleMessage(strErrorMsg, MB_OK);

        return 0;
    }

    HANDLE phGrabEnable[] = {m_Flag_thread_inlineMode, m_Signal_KillThread};
    HANDLE phGrabDisable[] = {m_Flag_thread_offline, m_Signal_KillThread};

    while (1)
    {
        m_main->SetReadySignal(TRUE);
        Sleep(1);
        m_main->SetReadySignal(TRUE);
        SetEvent(m_Flag_thread_idle);

        const DWORD dwRetEnable
            = ::WaitForMultipleObjects(sizeof(phGrabEnable) / sizeof(HANDLE), phGrabEnable, FALSE, INFINITE);

        if (dwRetEnable == WAIT_OBJECT_0 + 1)
        {
            // Kill 메시지
            return 0;
        }

        ResetEvent(m_Flag_thread_idle);

        while (
            ::WaitForMultipleObjects(sizeof(phGrabDisable) / sizeof(HANDLE), phGrabDisable, FALSE, 0) == WAIT_TIMEOUT)
        {
            if (SyncController::GetInstance().GetStartSignal() == TRUE)
            {
                DevelopmentLog::AddLog(DevelopmentLog::Type::Develop, _T("NGRV_Grab - Get Start Signal"));

                // Grab을 수행한다
                ImageGrab(0);
            }

            m_main->SetReadySignal(TRUE);
            Sleep(10);
        }
        // Inline모드를 Off시킨다
        ::ResetEvent(m_Flag_thread_inlineMode);
    }

    return 0;
}

bool InlineGrab::ThreadGrabFunc_Batch()
{
    auto imageBuffer = m_imageClassifier->getIdleBuffer();

    if (imageBuffer == nullptr)
    {
        // 빈 이미지 버퍼가 없다
        Sleep(500);
        return true;
    }

    // 인라인인 경우
    BatchInfo batchFilePath;

    m_csBatchFile.Lock();
    if (m_batchFilePath.m_filePath.IsEmpty())
    {
        m_csBatchFile.Unlock();
        Sleep(1);
        return true;
    }

    batchFilePath = m_batchFilePath;
    m_batchFilePath.m_filePath.Empty();
    m_csBatchFile.Unlock();

    if (!imageBuffer->initialize(batchFilePath.m_filePath, batchFilePath.m_paneIndex, batchFilePath.m_batchKey,
            batchFilePath.m_batchBarcode))
    {
        imageBuffer->setIdle();
    }
    else
    {
        m_inspectionOrderer->update(
            m_main->GetPrimaryVisionUnit().getTrayScanSpec(), m_main->GetInspectionRepeatIndex());
    }

    return true;
}

void InlineGrab::ImageGrab(long grabBufferIndex)
{
    long nVisionType = SystemConfig::GetInstance().GetVisionType();

    switch (nVisionType)
    {
        case VISIONTYPE_2D_INSP:
            ImageGrab2D(grabBufferIndex);
            break;

        case VISIONTYPE_3D_INSP:
            ImageGrab3D(grabBufferIndex);
            break;

        case VISIONTYPE_NGRV_INSP:
            ImageGrabColor(grabBufferIndex);
            break;

        case VISIONTYPE_SIDE_INSP:
            ImageGrabSide(grabBufferIndex);
            break;

        case VISIONTYPE_TR:
            ImageGrabTR(grabBufferIndex);
            break;
    }
}

void InlineGrab::ImageGrab2D(long grabBufferIndex)
{
    const long fovIndex = SyncController::GetInstance().GetFovIndex();
    FOVID fovID(m_trayKey, m_runCntFromHost, m_trayIndexFromHost, fovIndex);

    DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab, _T("InlineGrab::ImageGrab2D - T:%03d, FOV:%02d"),
        m_trayIndexFromHost, fovIndex);

    bool success = m_fovImageGrabber->grab(grabBufferIndex, 1, fovID);

    if (!success)
    {
        //	m_fovImageGrabber->setIdleBuffer(grabBufferIndex);
    }

    m_inspectionOrderer->update(m_main->GetPrimaryVisionUnit().getTrayScanSpec(), m_main->GetInspectionRepeatIndex());
}

void InlineGrab::ImageGrab3D(long grabBufferIndex)
{
    const long fovIndex = SyncController::GetInstance().GetFovIndex();
    FOVID fovID(m_trayKey, m_runCntFromHost, m_trayIndexFromHost, fovIndex);

    DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab, _T("InlineGrab::ImageGrab3D - T:%03d, FOV:%02d"),
        m_trayIndexFromHost, fovIndex);

    // 해당 FOV가 얼마나 참조되고 있는지 알려준다
    long needCount = 1;
    if (fovIndex >= 0 && fovIndex < (long)m_needCountForFOV.size())
    {
        needCount = m_needCountForFOV[fovIndex];
    }

    bool success = m_fovImageGrabber->grab(grabBufferIndex, needCount, fovID);
    SystemConfig::GetInstance().Save_SequenceTimeLog(_T("InlineGrab[ImageGrab3D]_GrabEnd"));

    if (!success)
    {
        m_fovImageGrabber->setIdleBuffer(grabBufferIndex);
    }

    m_inspectionOrderer->update(m_main->GetPrimaryVisionUnit().getTrayScanSpec(), m_main->GetInspectionRepeatIndex());
    SystemConfig::GetInstance().Save_SequenceTimeLog(_T("InlineGrab[ImageGrab3D]_InspectionOrderer_UpdateEnd"));
}

void InlineGrab::ImageGrabColor(long grabBufferIndex)
{
    const long fovIndex = SyncController::GetInstance().GetFovIndex();
    FOVID fovID(m_trayKey, m_runCntFromHost, m_trayIndexFromHost, fovIndex);

    DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab, _T("InlineGrab::ImageGrabColor - T:%03d, FOV:%02d"),
        m_trayIndexFromHost, fovIndex);

    bool success = m_fovImageGrabber->grab(grabBufferIndex, 1, fovID);

    if (!success)
    {
        m_fovImageGrabber->setIdleBuffer(grabBufferIndex);
    }
}

void InlineGrab::ImageGrabSide(long grabBufferIndex)
{
    const auto& trayScanSpec = m_main->GetPrimaryVisionUnit().getTrayScanSpec();

    //long nStitchCountX = trayScanSpec.GetMaxStitchCountX();

    const long nScanID = SyncController::GetInstance().GetScanIDforSide();
    const long stitchIndex = SyncController::GetInstance().GetStitchingIndexForSide();

    // Side FOV Index 부여 방식 변경 - 2024.06.19_JHB
    // Tray의 전체 Scan Pocket 개수를 계산하여 검사 시의 Pocket 개수를 갱신해줘야함
    // ScanID % Total Scan ID를 실제 FovID로 가져가야 검사에 문제가 발생하지 않음
    // 실제로 받은 ScanID는 따로 저장하여 결과 보내주기 전에 다시 Set해주는 방향으로 설정
    const long nTotalFovIDNum = trayScanSpec.m_pocketNumX * trayScanSpec.m_pocketNumY;
    const long nInspectionScanID = nScanID > nTotalFovIDNum ? 0 : nScanID;

    m_main->GetPrimaryVisionUnit().getImageLot().SetSideScanID(nScanID);
    /////////////////////////////////////////////////////////////////////////////////////

    //FOVID fovID(m_trayKey, m_runCntFromHost, m_trayIndexFromHost, nInspectionScanID, stitchIndex);
    const long nFovIndex = trayScanSpec.m_vecUnits[nInspectionScanID].m_fovList[stitchIndex];
    FOVID fovID(m_trayKey, m_runCntFromHost, m_trayIndexFromHost, nFovIndex, stitchIndex);

    for (long nSideidx = enSideVisionModule::SIDE_VISIONMODULE_START;
        nSideidx < enSideVisionModule::SIDE_VISIONMODULE_END; nSideidx++)
    {
        //enSideVisionModule F,R에 따라 뭔가를 분류해야할 수 있기 때문에
        CString strSideType("");
        switch (enSideVisionModule(nSideidx))
        {
            case enSideVisionModule::SIDE_VISIONMODULE_FRONT:
                strSideType = _T("Front");
                break;
            case enSideVisionModule::SIDE_VISIONMODULE_REAR:
                strSideType = _T("Rear");
                break;
            default:
                strSideType = _T("Unknown");
                break;
        }

        DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab,
            _T("InlineGrab::ImageGrab2D Side - T:%03d, FOV:%02d, SideType :%s"), m_trayIndexFromHost, nScanID,
            (LPCTSTR)strSideType);

        bool success = m_fovImageGrabber->grab(grabBufferIndex, 1, fovID, enSideVisionModule(nSideidx));

        if (!success)
        {
            //m_fovImageGrabber->setIdleBuffer(grabBufferIndex);
        }
    }

    //m_inspectionOrderer->updateForSide(m_main->GetPrimaryVisionUnit().getTrayScanSpec(), m_main->GetInspectionRepeatIndex(), fovID);
    m_inspectionOrderer->update(m_main->GetPrimaryVisionUnit().getTrayScanSpec(), m_main->GetInspectionRepeatIndex());
}

void InlineGrab::ImageGrabTR(long grabBufferIndex)
{
    const long fovIndex = SyncController::GetInstance().GetFovIndex();
    FOVID fovID(m_trayKey, m_runCntFromHost, m_trayIndexFromHost, fovIndex);

    DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab, _T("InlineGrab::ImageGrabTR - T:%03d, FOV:%02d"),
        m_trayIndexFromHost, fovIndex);

    bool success = m_fovImageGrabber->grab(grabBufferIndex, 1, fovID);

    if (!success)
    {
        //	m_fovImageGrabber->setIdleBuffer(grabBufferIndex);
    }

    m_inspectionOrderer->update(m_main->GetPrimaryVisionUnit().getTrayScanSpec(), m_main->GetInspectionRepeatIndex());
}

void InlineGrab::CheckChangedVisionTrayScanSpec()
{
    //-------------------------------------------------------------------------------------
    // 기존에 Inline Grab이 알고있던 Tray Scan Spec과 현재 Tray Scan Spec이 서로 다르다면
    // Fov Image 등의 정보가 기존정보와 틀려서 정상적으로 removeIncompleteImages 에서
    // 기존 이미지를 클리어 시켜주지 못할 수 있으므로 강제로 기존 모든 버퍼내용을 클리어 시킨다
    //-------------------------------------------------------------------------------------

    auto& trayScanSpec = m_main->GetPrimaryVisionUnit().getTrayScanSpec();

    CiDataBase db1, db2;
    trayScanSpec.LinkDataBase(TRUE, db1);
    m_visionTrayScanSpec.LinkDataBase(TRUE, db2);

    if (db1 != db2)
    {
        DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab, _T("InlineGrab::Image Buffer Clear"));

        if (m_fovImageGrabber != nullptr)
        {
            //Hardware연결시에만 사용한다
            m_fovImageGrabber->setAllBuffersToIdle();
        }

        m_imageClassifier->setAllBuffersToIdle();
        m_visionTrayScanSpec.LinkDataBase(FALSE, db1);
    }
}

void InlineGrab::ResetInline_GrabTimeLOG()
{
    if (m_fovImageGrabber != nullptr)
        m_fovImageGrabber->ClearImageGrabber_GrabTime();
}

void InlineGrab::SaveInline_GrabTimeLOG()
{
    m_fovImageGrabber->SaveImageGrabber_GrabTime(DynamicSystemPath::get(DefineFolder::Config));
}

const Ipvm::Size32s2& InlineGrab::GetImageBufferSize() const
{
    return m_imageBufferSize;
}

UINT InlineGrab::ThreadNGRVSaveFunc(LPVOID pParam)
{
    InlineGrab* control = (InlineGrab*)pParam;

    return control->ThreadNGRVSaveFunc();
}

UINT InlineGrab::ThreadNGRVSaveFunc()
{
    HANDLE phGrabEnable[] = {m_Flag_thread_inlineMode, m_Signal_KillThread};
    HANDLE phGrabDisable[] = {m_Flag_thread_offline, m_Signal_KillThread};

    const static BOOL bIsNRGVVision = SystemConfig::GetInstance().IsVisionTypeNGRV();

    while (1)
    {
        SetEvent(m_Flag_thread_idle);

        const DWORD dwRetEnable
            = ::WaitForMultipleObjects(sizeof(phGrabEnable) / sizeof(HANDLE), phGrabEnable, FALSE, INFINITE);

        if (dwRetEnable != WAIT_OBJECT_0)
        {
            // Kill 메시지
            return 0;
        }

        ResetEvent(m_Flag_thread_idle);

        long sequenceStep = 0;
        long grabBufferIndex = -1;

        while (1)
        {
            bool exit = false;

            if (m_isBatchInspection)
            {
                // Batch Inspection 모드일 때
                if (!ThreadGrabFunc_Batch())
                    exit = true;
                continue;
            }

            if (!SystemConfig::GetInstance().m_bHardwareExist)
            {
                // 하드웨어가 없을 때 동작은 없앴다.
                // 제대로 동작하지도 않을 것 같고, 필요하면 그때 다시 제대로 만들자
                continue;
            }

            // 하드웨어를 통해 영상 획득 모드일 때

            switch (sequenceStep)
            {
                case 0:
                    // Grab Buffer를 얻는다
                    if (!bIsNRGVVision
                        && !m_fovImageGrabber->getIdleBufferIndex(
                            m_Flag_thread_offline, m_Signal_KillThread, grabBufferIndex))
                    {
                        // Kill Signal 나 OffLine 명령을 받은 것이므로 나간다.
                        exit = true;
                        break;
                    }
                    else
                    {
                        if (!bIsNRGVVision && grabBufferIndex < 0)
                        {
                            // Grab Buffer 로 할당된게 없는 것이다
                            // Job이 로드되어 있지 않은 상태라면,
                            // 필요한 버퍼수가 할당되어 있지 않아서 이런 현상이 있을 수 있다.

                            Sleep(100);
                        }
                        else
                        {
                            // Grab Buffer를 얻었다면 Ready 상태로 바꾼다
                            //m_main->SetReadySignal(TRUE);
                            sequenceStep++;
                        }
                    }
                    break;

                case 1:
                    // m_vecNGRVSaveImage Size가 0보다 큰 값인지 루프
                    if (m_main->m_vecNGRVImageSet.size() > 0)
                    {
                        ImageSaveNGRV();
                        continue;
                    }
                    else
                    {
                        sequenceStep = 0;
                    }
                    break;
            }

            if (exit)
                break;
            Sleep(100);
        }

        // Inline모드를 Off시킨다
        ::ResetEvent(m_Flag_thread_inlineMode);
    }

    return 0;
}

void InlineGrab::ImageSaveForNGRV(CString strImageSaveFullFilePath, Ipvm::Image8u3& ColorImage, long nImageSaveType)
{
    switch (nImageSaveType)
    {
        case NGRV_FILE_FORMAT_BMP:
            Ipvm::ImageFile::SaveAsBmp(ColorImage, strImageSaveFullFilePath);
            break;

        case NGRV_FILE_FORMAT_PNG:
            Ipvm::ImageFile::SaveAsPng(ColorImage, strImageSaveFullFilePath);
            break;

        case NGRV_FILE_FORMAT_JPG:
        default:
            Ipvm::ImageFile::SaveAsJpeg(ColorImage, strImageSaveFullFilePath);
            break;
    }
}

long InlineGrab::GetDemosaicSizeY(long nCameraSizeX, long nCameraSizeY, long nStitchSizeX, long nStitchSizeY)
{
    UNREFERENCED_PARAMETER(nStitchSizeY);
    UNREFERENCED_PARAMETER(nCameraSizeX);

    switch (nStitchSizeX)
    {
        case 1:
            return nCameraSizeY;
        case 2:
            return 3500;
        case 3:
            return 2336;
        case 4:
            return 1750;
        case 5:
            return 1400;
        case 6:
            return 1668;
        case 7:
            return 1000;
        case 8:
            return 876;
        case 9:
            return 780;
        default:
            return 700;
    }

    return 1000;
}

void InlineGrab::SplitColorImageToSendHost(long i_nSrcImageSizeX, long i_nSrcImageSizeY, Ipvm::Image8u3 i_ColorImage)
{
    static Ipvm::Image8u ImageRed; // Red 이미지 버퍼 : 실제 Host에 보내는 이미지
    static Ipvm::Image8u ImageGreen; // Green 이미지 버퍼
    static Ipvm::Image8u ImageBlue; // Blue 이미지 버퍼

    // Host에 보낼 이미지를 위해 R 채널 이미지만 분리
    // Color Image Buffer Set
    ImageRed.Create(i_nSrcImageSizeX, i_nSrcImageSizeY);
    ImageGreen.Create(i_nSrcImageSizeX, i_nSrcImageSizeY);
    ImageBlue.Create(i_nSrcImageSizeX, i_nSrcImageSizeY);
    //////////////////

    Ipvm::ImageProcessing::SplitRGB(
        i_ColorImage, Ipvm::Rect32s(i_ColorImage), ImageRed, ImageGreen, ImageBlue); // Split Image

    m_main->iPIS_Send_Image(ImageRed);
}

void InlineGrab::SaveEachNGRVImage(std::vector<Ipvm::Image8u3> i_vecGrabbedImage, NgrvGrabbedImageInfo GrabbedImageInfo,
    CString i_strInspItemName, CString i_strReviewSideName, const BOOL& isNGRV_Single_Run,
    const std::vector<long>& vecSingleRunUsingFrameID)
{
    CString strImageSaveTypeName; // 이미지 저장 타입, 0:BMP / 1:JPG / 2:PNG
    CString strDirectory; //	NGRV 이미지 저장 메인 경로 "D:\\ReviewImages\\LotID\\NGRV\\"
    CString strNGRVImageSaveFullFilePath; // NGRV 이미지 저장 최종 경로
    CString strNGRVImageSaveFilePath; // NGRV 이미지 저장 경로

    long nTotalFrameCount
        = isNGRV_Single_Run == FALSE ? (long)i_vecGrabbedImage.size() : (long)vecSingleRunUsingFrameID.size();

    long nImageSaveType
        = SystemConfig::GetInstance().m_nSaveImageTypeForNGRV; // 이미지 저장 타입 확인, 0:BMP / 1:JPG / 2:PNG

    switch (nImageSaveType) // 이미지 저장 타입 Check
    {
        case NGRV_FILE_FORMAT_BMP:
            strImageSaveTypeName = _T("bmp");
            break;

        case NGRV_FILE_FORMAT_PNG:
            strImageSaveTypeName = _T("png");
            break;

        case NGRV_FILE_FORMAT_JPG:
        default:
            strImageSaveTypeName = _T("jpg");
            break;
    }

    strDirectory.Format(_T("%s%s\\NGRV\\"), REVIEW_IMAGE_DIRECTORY, (LPCTSTR)m_main->m_lotID);
    Ipvm::CreateDirectories(strDirectory);

    CTime time(CTime::GetCurrentTime());
    CString strTime = time.Format(_T("%Y%m%d_%H%M%S"));
    // LOT_DATA_TIME_VISION_T_S_P_FRAME_inspection.jpg

    long SaveFrameidx(-1);
    for (long nFrame = 0; nFrame < nTotalFrameCount; nFrame++)
    {
        if (isNGRV_Single_Run == TRUE)
            SaveFrameidx = vecSingleRunUsingFrameID[nFrame] + 1; //mc_조명 Frame Setup창과 동일한 번호로 맞춘다
        else
            SaveFrameidx = nFrame;

        strNGRVImageSaveFilePath.Format(_T("%s_%s_%s_%02d_%02d_%02d_Frame%02d_%s.%s"), (LPCTSTR)m_main->m_lotID,
            (LPCTSTR)strTime, (LPCTSTR)i_strReviewSideName, GrabbedImageInfo.m_nTrayID, GrabbedImageInfo.m_nPackageID,
            GrabbedImageInfo.m_nGrabPointID, SaveFrameidx, (LPCTSTR)i_strInspItemName, (LPCTSTR)strImageSaveTypeName);

        strNGRVImageSaveFullFilePath.Format(_T("%s%s"), (LPCTSTR)strDirectory, (LPCTSTR)strNGRVImageSaveFilePath);

        GrabbedImageInfo.m_vecstrImageFileName.push_back(strNGRVImageSaveFullFilePath); //	Push back Image File Names

        ImageSaveForNGRV(strNGRVImageSaveFullFilePath, i_vecGrabbedImage[nFrame],
            nImageSaveType); // Save NGRV Image sort by Save type
    }

    // 4. Send NGRV Result Info data to Host
    m_main->iPIS_Send_NGRV_CompleteImageSave(GrabbedImageInfo);
}

void InlineGrab::ImageSaveNGRV()
{
    //----------------------------------------------------------
    // NGRV Image Save Sequence
    // 1. Copy Original Image from main buffer to minor buffer
    // 2. Split Color Image in RGB & Send Red Image to Host
    // 3. Erase main buffer to wait next image
    // 4. Save review image with minor buffer
    // 5. Send Image info data to Host
    //----------------------------------------------------------

    auto ngrvImageSet = m_main->m_vecNGRVImageSet[0];
    long nImageSizeX = ngrvImageSet.m_vecImageForSave[0].GetSizeX();
    long nImageSizeY = ngrvImageSet.m_vecImageForSave[0].GetSizeY();

    long nTotalFrameCount = (long)ngrvImageSet.m_vecImageForSave.size();

    std::vector<Ipvm::Image8u3> vecColorImage; // 원본 이미지에서 Color conversion 후 담아 놓을 버퍼
    vecColorImage.resize(nTotalFrameCount);

    /////////////////// NGRV 영상 정보 - 원본을 Erase하기 전에 Copy를 해둬야 한다
    NgrvGrabbedImageInfo GrabbedImageInfo; // Host로 보낼 영상 정보 클래스

    GrabbedImageInfo.m_nTrayID = ngrvImageSet.m_nTrayID; // Tray ID
    GrabbedImageInfo.m_nPackageID = ngrvImageSet.m_nPackageID; // Package ID(Number)
    GrabbedImageInfo.m_nGrabPointID = ngrvImageSet.m_nGrabPointID; // Grab Point ID
    GrabbedImageInfo.m_strPackageID = ngrvImageSet.m_strPackageID; // Package ID(String)
    GrabbedImageInfo.m_nSendImageSizeX = nImageSizeX;
    GrabbedImageInfo.m_nSendImageSizeY = nImageSizeY;
    CString strInspItemName = m_main->m_vecNGRVImageSet[0].m_strInspectionItemName; // Inspection Module Name
    CString strReviewSideName = m_main->m_vecNGRVImageSet[0].m_strReviewSideName; // Review Vision Name
    ///////////////////

    // Frame별로 Minor Buffer에 Image Copy
    for (long nFrame = 0; nFrame < nTotalFrameCount; nFrame++)
    {
        auto ngrvFrameBuffer = ngrvImageSet.m_vecImageForSave[nFrame];

        vecColorImage[nFrame].Create(nImageSizeX, nImageSizeY);
        vecColorImage[nFrame].FillZero(); // 일단 초기화를 한 번 하자

        Ipvm::ImageProcessing::Copy(ngrvFrameBuffer, Ipvm::Rect32s(ngrvFrameBuffer), vecColorImage[nFrame]);
    }

    //SingleRun인경우 Setup했던 Frame index를 부여한다
    std::vector<long> vecSingleRunUseFrameID;
    if (m_main->m_isNGRVSingleRun == TRUE)
    {
        vecSingleRunUseFrameID = ngrvImageSet.m_vecnFrameID;
    }

    m_main->m_mutNGRV.Lock();
    m_main->m_vecNGRVImageSet.erase(
        m_main->m_vecNGRVImageSet.begin()); // Conversion 및 지역변수에 저장이 끝났으니 원본 이미지 삭제
    m_main->m_mutNGRV.Unlock();

    // 1. Send Image to Host
    SplitColorImageToSendHost(nImageSizeX, nImageSizeY, vecColorImage[0]);

    // 2. Save Review Images and Set GrabbedImageInfo to Send Host
    long nFindDivider = strInspItemName.Find(_T("]"));
    if (nFindDivider > 0)
        strInspItemName.Delete(0, nFindDivider + 1);
    SaveEachNGRVImage(vecColorImage, GrabbedImageInfo, strInspItemName, strReviewSideName, m_main->m_isNGRVSingleRun,
        vecSingleRunUseFrameID);
}