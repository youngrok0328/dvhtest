#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "Framegrabber_Base.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
namespace IntekPlus
{
namespace VisionMadang
{
class SbtCore;
struct SbtCoreParameters;
} // namespace VisionMadang
} // namespace IntekPlus

//HDR_6_________________________________ Header body
//
#define SIDE_CAMERA_NUM 2
#define XQSWAP(x) \
    ((x & 0xff000000) >> 24) | ((x & 0x000000ff) << 24) | ((x & 0x00ff0000) >> 8) | ((x & 0x0000ff00) << 8)

typedef enum _BootstrapInterface
{
    _IInteger,
    _IString,
    _IEnumerate
} INTERFACE_TYPE;

union HexToFloat
{
    unsigned char m_nHex[4];
    float m_fValue;
};

enum enumCamCommAddress
{
    Gain_Length = 4,
    Gain_Address = 0x10040044,
};

class Framegrabber2D_iGrabXQ : public FrameGrabber_Base
{
public:
    Framegrabber2D_iGrabXQ();
    ~Framegrabber2D_iGrabXQ() override;

public:
    virtual bool grab(const int& grabber_idx, const int& camera_idx, BYTE** image, const int& grab_num,
        Ipvm::AsyncProgress* progress) override;
    virtual bool wait_grab_end(const int& grabber_idx, const int& camera_idx, Ipvm::AsyncProgress* progress) override;

    virtual bool SetDigitalGainValue(float i_fInputGain, BOOL bInline) override;

    virtual bool OpenCameraComm(const int& camera_id) override;
    virtual bool CloseCameraComm() override;

    virtual float GetCurrentCameraGain() override;

public: //F/W & Lib. Version
    virtual void get_firmware_version(
        const int& grabber_id, unsigned int& cur_version, unsigned int& need_version) override;
    virtual void get_library_version(unsigned int& cur_version, unsigned int& need_version) override;

public:
    virtual bool apply_grab_image_offset_Y(const int& grabber_idx, const int& camera_idx, const long& offset) override;

protected:
    virtual void init_grabber_info() override;
    virtual void init_camera_info(const CString& camera_param_file_path) override;
    virtual bool LiveCallback(const int& graaber_id = 0, const int& camera_id = 0) override;
    virtual bool SetTriggerMode(int nMode) override; //	FrameGrabber의 Trigger Mode

private:
    //최대로 사용할 수 있는 camera 개수만큼 할당된 channel을 확인, channel을 할당하지 않았다면 fase
    bool check_is_valid_camera_channel(const int& id_graaber, const int& camera_idx);

    bool init_camera_serial();
    bool init_camera_default_param(const int& camera_type);

    void CheckCameraInfoNGRV();
    void CheckCameraInfoSide();
    bool GetDeviceSerialNumber(const int& camera_id, CString& serialNumber);

    bool SetCameraAcqusitionStart(const int& camera_id);
    bool SetSideCameraAcquisition(const int& camera_id, BOOL i_bAcquisition);

    BYTE** m_ppbyGrabBuffers;
    BYTE** m_ppbyColorGrabBuffers;
    short m_sGrabCounts;

    CString m_strErrorMessage;

    bool GetCurrentGainValue(float& o_fCurrentGain);
    bool SetVerticalSizeY(const int& id_grabber, const int& id_camera, const int& id_icf, const bool i_bFullSizeY,
        const long i_nScanSizeY, const bool i_bSideVision);

    float m_fCurrentCameraGain; // 현재 Camera Gain 값 저장 - JHB_2023.02.07

private:
    std::map<grabber_id, std::map<camera_channel, CString>> camera_serials;

private: // Camera Comm.
    HANDLE m_hActiveDev;

    BYTE _SendCmd(BOOL RW, INTERFACE_TYPE ifType, ULONG nLength, ULONG nAddr, BYTE* pSendData, BYTE* pRecvData);

public: // Camera Comm.
    BYTE WriteInteger(ULONG nAddr, ULONG nLength, UINT32 nSendData);
    BYTE WriteString(ULONG nAddr, ULONG nLength, BYTE* pSendData);
    BYTE WriteDouble(ULONG nAddr, ULONG nLength, DOUBLE fSendData);
    BYTE WriteFloat(ULONG nAddr, ULONG nLength, FLOAT fSendData);

    BYTE ReadInteger(ULONG nAddr, ULONG nLength, UINT32* pRecvData);
    BYTE ReadString(ULONG nAddr, ULONG nLength, BYTE* pRecvData);
    BYTE ReadDouble(ULONG nAddr, ULONG nLength, DOUBLE* pRecvData);
    BYTE ReadFloat(ULONG nAddr, ULONG nLength, FLOAT* fSendData);

    void CalcTime(CString i_strFileName);
};