//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "InspectionOrderer.h"

//CPP_2_________________________________ This project's headers
#include "FOVImageGrabber.h"
#include "ImageClassifier.h"
#include "VisionMain.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MainUiModules/VisionInlineUI/VisionInlineUI.h"
#include "../../ManagementModules/VisionUnit/VisionUnit.h"
#include "../../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
InspectionOrderer::InspectionOrderer(
    CVisionMain* main, ImageClassifier* imageClassifier, VisionUnit** inlineUnits, long threadNum)
    : m_main(main)
    , m_imageClassifier(imageClassifier)
    , m_threadNum(threadNum)
    , m_lastestGrabBufferIndex(-1)
    , m_callback_inspectionEnd(new CallBackList_InspectionEnd)
    , m_nInspectionRepeatIndex(0)
{
    m_items = new SThreadControl[m_threadNum];

    for (long nThread = 0; nThread < m_threadNum; nThread++)
    {
        auto& control = m_items[nThread];

        control.m_visionUnit = inlineUnits[nThread];
        control.m_inlineUI = nullptr;
        control.m_Signal_KillThread = ::CreateEvent(NULL, FALSE, FALSE, NULL);
        control.m_Signal_ThreadStarted = ::CreateEvent(NULL, FALSE, FALSE, NULL);
        control.m_Signal_Ready = ::CreateEvent(NULL, FALSE, FALSE, NULL);
        control.m_Signal_StartInspection = ::CreateEvent(NULL, FALSE, FALSE, NULL);
        control.m_Signal_Idle = ::CreateEvent(NULL, TRUE, TRUE, NULL);

        HANDLE hNewThread = ::AfxBeginThread(ThreadInspectionFunc, (void*)this, THREAD_PRIORITY_NORMAL);
        ::DuplicateHandle(::GetCurrentProcess(), hNewThread, ::GetCurrentProcess(), &control.m_Handle_Thread, 0, FALSE,
            DUPLICATE_SAME_ACCESS);

        if (::WaitForSingleObject(control.m_Signal_ThreadStarted, 10000) != WAIT_OBJECT_0)
        {
            ::AfxMessageBox(_T("Cannot start inspection thread normally.\r\n"), MB_OK | MB_ICONERROR);
            ::exit(0);
        }
    }
}

InspectionOrderer::~InspectionOrderer()
{
    for (long nThread = 0; nThread < m_threadNum; nThread++)
    {
        auto& control = m_items[nThread];

        ::SetEvent(control.m_Signal_KillThread);

        if (::WaitForSingleObject(control.m_Handle_Thread, 10000) != WAIT_OBJECT_0)
            ::TerminateThread(control.m_Handle_Thread, 555);

        ::CloseHandle(control.m_Signal_KillThread);
        ::CloseHandle(control.m_Signal_ThreadStarted);
        ::CloseHandle(control.m_Signal_Ready);
        ::CloseHandle(control.m_Signal_StartInspection);
        ::CloseHandle(control.m_Signal_Idle);
        ::CloseHandle(control.m_Handle_Thread);
    }

    delete[] m_items;
    delete m_callback_inspectionEnd;
}

VisionUnit& InspectionOrderer::getVisionUnit(long visionUnitIndex)
{
    return *m_items[visionUnitIndex].m_visionUnit;
}

bool InspectionOrderer::isAllInspectionThreadsIdle()
{
    for (long threadIndex = 0; threadIndex < m_threadNum; threadIndex++)
    {
        auto& inspControl = m_items[threadIndex];

        if (::WaitForSingleObject(inspControl.m_Signal_Idle, 0) != WAIT_OBJECT_0)
        {
            return false;
        }
    }

    return true;
}

void InspectionOrderer::setInlineUI(long visionUnitIndex, CVisionInlineUI& ui, HWND parent)
{
    ui.SetVisionModules(m_main, m_items[visionUnitIndex].m_visionUnit, parent);
    m_items[visionUnitIndex].m_inlineUI = &ui;
}

void InspectionOrderer::update(const VisionTrayScanSpec& trayScanSpec, const long InspectionRepeatIndex)
{
    CSingleLock lock(&m_csUpdate, TRUE);

    m_imageClassifier->update(trayScanSpec);

    std::vector<long> validBufferList;
    m_imageClassifier->getValidBufferList(validBufferList);
    if (validBufferList.size() == 0)
        return;

    for (auto& validBufferIndex : validBufferList)
    {
        bool isLastPaneIndex = false;

        while (!isLastPaneIndex)
        {
            // IDLE 상태의 검사 Thread 에게 IMAGE 버퍼를 할당한다
            long controlIndex = getIdleControl();
            if (controlIndex < 0)
            {
                // IDLE 상태의 검사 Thread 가 없다
                return;
            }

            auto buffer = m_imageClassifier->getBuffer(validBufferIndex);
            buffer->m_imageLot->SetInspectionRepeatIndex(InspectionRepeatIndex);
            m_nInspectionRepeatIndex = buffer->m_imageLot->GetInspectionRepeatIndex();

            long paneIndex = buffer->popCalculationPaneIndex(isLastPaneIndex);
            auto& control = m_items[controlIndex];

            control.m_imageBuffer = buffer;
            control.m_batchKey = buffer->getBatchKey();
            control.m_batchBarcode = buffer->getBatchBarcode();
            control.m_paneIndex = paneIndex;
            control.m_visionUnit->setImageLot(buffer->m_imageLot);

            // 계산 Thread 에 명령을 내린다
            SetEvent(control.m_Signal_StartInspection);
        }
    }
}

void InspectionOrderer::updateForSide(
    const VisionTrayScanSpec& trayScanSpec, const long InspectionRepeatIndex, const FOVID& fovID)
{
    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);

    if (bIsSideVision == false)
    {
        update(trayScanSpec, InspectionRepeatIndex);
        return;
    }

    CSingleLock lock(&m_csUpdate, TRUE);

    m_imageClassifier->updateForSide(trayScanSpec, fovID);

    std::vector<long> validBufferList;
    m_imageClassifier->getValidBufferList(validBufferList);
    if (validBufferList.size() == 0)
        return;

    for (auto& validBufferIndex : validBufferList)
    {
        bool isLastPaneIndex = false;

        while (!isLastPaneIndex)
        {
            // IDLE 상태의 검사 Thread 에게 IMAGE 버퍼를 할당한다
            long controlIndex = getIdleControl();
            if (controlIndex < 0)
            {
                // IDLE 상태의 검사 Thread 가 없다
                return;
            }

            auto buffer = m_imageClassifier->getBuffer(validBufferIndex);
            buffer->m_imageLot->SetInspectionRepeatIndex(InspectionRepeatIndex);
            m_nInspectionRepeatIndex = buffer->m_imageLot->GetInspectionRepeatIndex();

            long paneIndex = buffer->popCalculationPaneIndex(isLastPaneIndex);
            auto& control = m_items[controlIndex];

            control.m_imageBuffer = buffer;
            control.m_batchKey = buffer->getBatchKey();
            control.m_batchBarcode = buffer->getBatchBarcode();
            control.m_paneIndex = paneIndex;
            control.m_visionUnit->setImageLot(buffer->m_imageLot);

            // 계산 Thread 에 명령을 내린다
            SetEvent(control.m_Signal_StartInspection);
        }
    }
}

bool InspectionOrderer::copyFromLastestCalculationImage(VisionImageLot& imageLot)
{
    if (m_lastestGrabBufferIndex < 0)
        return false;

    imageLot.CopyFrom(*m_items[m_lastestGrabBufferIndex].m_imageBuffer->m_imageLot);
    m_lastestGrabBufferIndex = -1;

    return true;
}

void InspectionOrderer::registerCallBack_InspectionEnd(
    void* userData, VisionMainAgent::FUNC_INSPECTION_END_CALLBACK* callback)
{
    m_callback_inspectionEnd->Add(userData, callback);
}

void InspectionOrderer::unRegisterCallBack_InspectionEnd(VisionMainAgent::FUNC_INSPECTION_END_CALLBACK* callback)
{
    m_callback_inspectionEnd->Del(callback);
}

UINT InspectionOrderer::ThreadInspectionFunc(LPVOID pParam)
{
    InspectionOrderer* control = (InspectionOrderer*)pParam;

    control->ThreadInspection();

    return 0;
}

UINT InspectionOrderer::ThreadInspection()
{
    static long nLatestThreadID = 0;
    const long nThreadID = nLatestThreadID++;

    auto& control = m_items[nThreadID];
    auto& visionUnit = *control.m_visionUnit;

    /*static const bool bIsSIdeVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);*/

    CString FilePath;

    // 쓰레드가 정상적으로 시작되었음을 알려준다.
    // 쓰레드 아이디 중복을 막기 위함임.
    ::SetEvent(control.m_Signal_ThreadStarted);

    // WaitForMultipleObjects 함수에서 여러 이벤트가 동시에 발생했을 때 낮은 번호 것을 리턴함.
    // 그러므로 StartInspection 이벤트가 KillThread 이벤트에 묻히지 않게 하기 위해서는 배열에 집어넣는 순서를 지켜줘야 함.
    HANDLE pHandles[] = {control.m_Signal_StartInspection, control.m_Signal_KillThread};

    while (1)
    {
        // 검사 가능 상태임을 알려준다.
        ::SetEvent(control.m_Signal_Idle);
        ::SetEvent(control.m_Signal_Ready);

        update(visionUnit.getTrayScanSpec(), m_nInspectionRepeatIndex);

        const DWORD dwRet = ::WaitForMultipleObjects(sizeof(pHandles) / sizeof(HANDLE), pHandles, FALSE, INFINITE);

        // 쓰레드 종료 상황이므로 함수에서 나간다.
        if (dwRet > WAIT_OBJECT_0)
        {
            return 0;
        }

        visionUnit.setImageLotInsp();

        // 검사가 시작되었음을 알려준다.
        ::ResetEvent(control.m_Signal_Idle);

        bool manualInp = (control.m_batchKey >= 0);

        visionUnit.SetCurrentPaneID(control.m_paneIndex);
        visionUnit.getTrayScanSpec().SetBatchKey(control.m_batchKey, control.m_batchBarcode);

        DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab,
            _T("[Thread%d] Inline Inspection - T:%03d, S:%02d P:%02d - Start"), nThreadID,
            visionUnit.getImageLot().GetInfo().m_trayIndex, visionUnit.getImageLot().GetInfo().m_scanAreaIndex,
            control.m_paneIndex);

        auto& inlineUI = *control.m_inlineUI;
        long nNeedHandlerStop = inlineUI.OnThreadInspect(manualInp);

        DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab,
            _T("[Thread%d] Inline Inspection - T:%03d, S:%02d P:%02d - End"), nThreadID,
            visionUnit.getImageLot().GetInfo().m_trayIndex, visionUnit.getImageLot().GetInfo().m_scanAreaIndex,
            control.m_paneIndex);

        m_callback_inspectionEnd->CallForEnd(visionUnit, nThreadID);
        m_lastestGrabBufferIndex = nThreadID;

        if (nNeedHandlerStop == 1)
        {
            // 영훈 20160503 : 해당 항목에 불량이 발생했을 경우 Host에 Debug Stop 메세지를 보낸다.
            m_main->iPIS_HandlerStop();

            Sleep(1000);
        }

        // Grab Buffer를 다 사용하였다 이제 Grab Buffer를 다 사용했다고 알려주자
        control.m_imageBuffer->calculatationCompleted(control.m_paneIndex);
    }

    return 0;
}

long InspectionOrderer::getIdleControl()
{
    std::vector<HANDLE> vecReady;
    std::vector<long> vecThreadIndex;

    for (long nThreadID = 0; nThreadID < m_threadNum; nThreadID++)
    {
        vecReady.push_back(m_items[nThreadID].m_Signal_Ready);
        vecThreadIndex.push_back(nThreadID);
    }

    if (vecReady.size() == 0)
        return -1;

    const DWORD dwRetReady = ::WaitForMultipleObjects(DWORD(vecReady.size()), &vecReady[0], FALSE, 0);
    if (dwRetReady == WAIT_TIMEOUT)
        return -1;

    return vecThreadIndex[long(dwRetReady - WAIT_OBJECT_0)];
}
