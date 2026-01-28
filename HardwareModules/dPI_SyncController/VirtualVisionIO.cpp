//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VirtualVisionIO.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_SystemIni/PersonalConfig.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedCommunicationModules/VisionHostCommon/iPIS_MsgDefinitions.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Gadget/SocketMessaging.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VirtualVisionIO* VirtualVisionIO::m_instance = nullptr;

VirtualVisionIO& VirtualVisionIO::GetInstance()
{
    if (m_instance == nullptr)
    {
        m_instance = new VirtualVisionIO;
    }

    return *m_instance;
}

void VirtualVisionIO::DelInstance()
{
    delete m_instance;
    m_instance = nullptr;
}

VirtualVisionIO::VirtualVisionIO()
    : m_enabled(false)
    , m_socket(nullptr)
{
    if (SystemConfig::GetInstance().IsHardwareExist() == FALSE
        && PersonalConfig::getInstance().isSocketConnectionEnabled())
    {
        // No hardware 일 때 Host와 가상 통신을 위한 IO을 Open 하자

        m_socket = new Ipvm::SocketMessaging({0, 9500, this, msgCallback, evtCallback});
        m_enabled = true;
    }
}

VirtualVisionIO::~VirtualVisionIO()
{
    delete m_socket;
}

bool VirtualVisionIO::IsEnabled() const
{
    if (!m_enabled)
        return false;

    return m_socket->IsConnected();
}

void VirtualVisionIO::SetVisionReady(bool enable)
{
    m_info.m_ready = enable;
    m_socket->Write(long(enumVirtualVisionIO::Ready), sizeof(enable), &enable);
}

bool VirtualVisionIO::IsVisionReady()
{
    return m_info.m_ready;
}

void VirtualVisionIO::SetVisionAcquisition(bool enable)
{
    m_info.m_acqusition = enable;
    m_socket->Write(long(enumVirtualVisionIO::Acqusition), sizeof(enable), &enable);
}

bool VirtualVisionIO::IsVisionAcqusition()
{
    return m_info.m_acqusition;
}

bool VirtualVisionIO::IsVisionSlowAcqusition(long probe_id)
{
    UNREFERENCED_PARAMETER(probe_id);

    return m_info.m_slow_acqusition;
}

bool VirtualVisionIO::IsVisionStart()
{
    return m_info.m_start;
}

long VirtualVisionIO::GetFovIndex()
{
    return m_info.m_fovIndex;
}

long VirtualVisionIO::GetIndexNGRV() //kircheis_NGRV_Sync
{
    return 0; //kircheis_NGRV_Sync //나중에 제대로 넣자
}

void VirtualVisionIO::msgCallback(const void* userData, const int32_t identifier, const int32_t messageIndex,
    const uint32_t messageLength, const void* message)
{
    ((VirtualVisionIO*)userData)->msgCallback(identifier, messageIndex, messageLength, message);
}

void VirtualVisionIO::evtCallback(
    const void* userData, const int32_t identifier, const bool connected, const bool disconnected)
{
    ((VirtualVisionIO*)userData)->evtCallback(identifier, connected, disconnected);
}

void VirtualVisionIO::msgCallback(
    const int32_t identifier, const int32_t messageIndex, const uint32_t messageLength, const void* message)
{
    UNREFERENCED_PARAMETER(identifier);
    UNREFERENCED_PARAMETER(messageLength);

    switch (enumVirtualVisionIO(messageIndex))
    {
        case enumVirtualVisionIO::Start:
            m_info.m_start = *((bool*)message);
            break;

        case enumVirtualVisionIO::ScanID:
            m_info.m_fovIndex = *((long*)message);
            break;

        case enumVirtualVisionIO::StitchID:
            break;
    }
}

void VirtualVisionIO::evtCallback(const int32_t identifier, const bool connected, const bool disconnected)
{
    UNREFERENCED_PARAMETER(identifier);
    UNREFERENCED_PARAMETER(disconnected);

    if (connected)
    {
        // Host Output 들을 Vision으로 보내주자
        SetVisionReady(m_info.m_ready);
        SetVisionAcquisition(m_info.m_acqusition);
    }
}
