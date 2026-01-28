//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "FOVImageGrabber.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
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
FOVID::FOVID()
    : m_runtimeIndex(-1)
    , m_trayIndex(-1)
    , m_fovIndex(-1)
    , m_stitchIndex(-1)
{
}

FOVID::FOVID(LPCTSTR trayKey, long runtimeIndex, long trayIndex, long fovIndex, long stitchIndex)
    : m_trayKey(trayKey)
    , m_runtimeIndex(runtimeIndex)
    , m_trayIndex(trayIndex)
    , m_fovIndex(fovIndex)
    , m_stitchIndex(stitchIndex)
{
}

//==============================================================================================================

FOVImageGrabber::FOVImageGrabber(CVisionMain* visionMain)
    : m_visionMain(visionMain)
{
    for (long index = 0; index < m_bufferMaxCount; index++)
    {
        auto buffer = std::make_shared<FOVBaseBuffer>();
        buffer->m_handleReady = ::CreateEvent(NULL, FALSE, TRUE, NULL);
        buffer->m_grabed = false;
        buffer->m_isReady = true;

        m_baseBuffers[index] = buffer;
    }

    _writeLog_BufferState();
}

FOVImageGrabber::~FOVImageGrabber()
{
    for (auto buffer : m_baseBuffers)
    {
        ::CloseHandle(buffer->m_handleReady);
    }
}

void FOVImageGrabber::setAllBuffersToIdle()
{
    CSingleLock lock(&m_csLock, TRUE);

    for (int bufferIndex = 0; bufferIndex < m_bufferMaxCount; bufferIndex++)
    {
        _setIdleBuffer(bufferIndex);
    }

    _writeLog_BufferState();
}

FOVID FOVImageGrabber::getBufferFovID(long bufferIndex)
{
    CSingleLock lock(&m_csLock, TRUE);

    return m_baseBuffers[bufferIndex]->m_fovID;
}

void FOVImageGrabber::getValidBufferList(std::vector<long>& o_validList)
{
    CSingleLock lock(&m_csLock, TRUE);

    o_validList.clear();

    for (long grabBufferIndex = 0; grabBufferIndex < m_bufferMaxCount; grabBufferIndex++)
    {
        if (m_baseBuffers[grabBufferIndex]->m_grabed)
        {
            o_validList.push_back(grabBufferIndex);
        }
    }
}

bool FOVImageGrabber::getIdleBufferIndex(HANDLE signalForOut1, HANDLE signalForOut2, long& o_bufferIndex)
{
    std::vector<HANDLE> vecReady;
    std::vector<long> vecGrabBufferIndex;

    if (true)
    {
        CSingleLock lock(&m_csLock, TRUE);

        o_bufferIndex = -1;

        for (long grabBufferIndex = 0; grabBufferIndex < m_bufferMaxCount; grabBufferIndex++)
        {
            vecReady.push_back(m_baseBuffers[grabBufferIndex]->m_handleReady);
            vecGrabBufferIndex.push_back(grabBufferIndex);
        }

        vecReady.push_back(signalForOut1);
        vecGrabBufferIndex.push_back(-1);

        vecReady.push_back(signalForOut2);
        vecGrabBufferIndex.push_back(-1);
    }

    const DWORD dwRetReady = ::WaitForMultipleObjects(DWORD(vecReady.size()), &vecReady[0], FALSE, INFINITE);
    o_bufferIndex = vecGrabBufferIndex[long(dwRetReady - WAIT_OBJECT_0)];

    if (o_bufferIndex == -1)
        return false;

    m_baseBuffers[o_bufferIndex]->m_isReady = false;
    _writeLog_BufferState();

    return true;
}

void FOVImageGrabber::setIdleBuffer(long bufferIndex)
{
    CSingleLock lock(&m_csLock, TRUE);

    _setIdleBuffer(bufferIndex);
    _writeLog_BufferState();
}

void FOVImageGrabber::linkDecrease(long bufferIndex)
{
    CSingleLock lock(&m_csLock, TRUE);

    m_baseBuffers[bufferIndex]->m_linkCount--;

    if (m_baseBuffers[bufferIndex]->m_linkCount <= 0)
    {
        setIdleBuffer(bufferIndex);
    }
}

bool FOVImageGrabber::grab(
    long bufferIndex, long linkCount, const FOVID& fovID, const enSideVisionModule i_eSideVisionModule)
{
    CSingleLock lock(&m_csLock, TRUE);

    m_baseBuffers[bufferIndex]->m_linkCount = linkCount;
    m_baseBuffers[bufferIndex]->m_fovID = fovID;

    bool bBufferGrabResult = buffer_grab(bufferIndex, fovID, i_eSideVisionModule);

    if (bBufferGrabResult == true || SystemConfig::GetInstance().IsVisionType2D() == TRUE
        || SystemConfig::GetInstance().IsVisionTypeSide() == TRUE)
    {
        m_baseBuffers[bufferIndex]->m_grabed = true;
        _writeLog_BufferState();
        return true;
    }

    return false;
}

void FOVImageGrabber::paste(long bufferIndex, long stitchIndex, long stitchCountX, long stitchCountY,
    std::shared_ptr<VisionImageLot> inspectionImage)
{
    CSingleLock lock(&m_csLock, TRUE);

    long inspectionID = inspectionImage->GetInfo().m_scanAreaIndex;
    DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab, _T("FOVImageGrabber::paste (I%d:B%d:S%d)"), inspectionID,
        bufferIndex, stitchIndex);
    buffer_paste(bufferIndex, stitchIndex, stitchCountX, stitchCountY, inspectionImage);
}

void FOVImageGrabber::pasteforSide(long i_nBufferIndex, long i_nStitchIndex, long i_nScanID, long i_nStitchCountX,
    long i_nStitchCountY, std::shared_ptr<VisionImageLot> inspectionImage)
{
    UNREFERENCED_PARAMETER(i_nScanID);

    CSingleLock lock(&m_csLock, TRUE);

    long inspectionID = inspectionImage->GetInfo().m_scanAreaIndex;
    DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab, _T("FOVImageGrabber::paste (I%d:B%d:S%d)"), inspectionID,
        i_nBufferIndex, i_nStitchIndex);
    buffer_paste(i_nBufferIndex, i_nStitchIndex, i_nStitchCountX, i_nStitchCountY, inspectionImage);
}

void FOVImageGrabber::_setIdleBuffer(long bufferIndex)
{
    m_baseBuffers[bufferIndex]->m_grabed = false;
    m_baseBuffers[bufferIndex]->m_isReady = true;
    ::SetEvent(m_baseBuffers[bufferIndex]->m_handleReady);
}

void FOVImageGrabber::_writeLog_BufferState()
{
    CString text;
    text.Format(_T("BufferState:"));
    for (long index = 0; index < m_bufferMaxCount; index++)
    {
        auto buffer = m_baseBuffers[index];
        if (buffer->m_isReady)
        {
            text.AppendFormat(_T("_"));
        }
        else
        {
            if (buffer->m_grabed)
            {
                text.AppendFormat(_T("%d"), buffer->m_linkCount);
            }
            else
            {
                text.AppendFormat(_T("!"));
            }
        }
    }

    DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab, text);
}

CVisionMain& FOVImageGrabber::GetVisionMainInfo()
{
    return *m_visionMain;
}
