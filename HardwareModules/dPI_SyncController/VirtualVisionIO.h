#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class VirtualVisionIO
{
public:
    ~VirtualVisionIO();

    static VirtualVisionIO& GetInstance();
    static void DelInstance();

    bool IsEnabled() const;
    void SetVisionReady(bool enable);
    bool IsVisionReady();
    void SetVisionAcquisition(bool enable);
    bool IsVisionAcqusition();
    bool IsVisionSlowAcqusition(long probe_id);
    bool IsVisionStart();
    long GetFovIndex();
    long GetIndexNGRV(); //kircheis_NGRV_Sync

private:
    VirtualVisionIO();

    static VirtualVisionIO* m_instance;

    struct VisionInfo
    {
        bool m_ready;
        bool m_acqusition;
        bool m_slow_acqusition;
        bool m_start;
        long m_fovIndex;

        VisionInfo()
            : m_ready(false)
            , m_acqusition(false)
            , m_slow_acqusition(false)
            , m_start(false)
            , m_fovIndex(0)
        {
        }
    };

    bool m_enabled;
    Ipvm::SocketMessaging* m_socket;
    VisionInfo m_info;

    static void msgCallback(const void* userData, const int32_t identifier, const int32_t messageIndex,
        const uint32_t messageLength, const void* message);
    static void evtCallback(
        const void* userData, const int32_t identifier, const bool connected, const bool disconnected);

    void msgCallback(
        const int32_t identifier, const int32_t messageIndex, const uint32_t messageLength, const void* message);
    void evtCallback(const int32_t identifier, const bool connected, const bool disconnected);
};
