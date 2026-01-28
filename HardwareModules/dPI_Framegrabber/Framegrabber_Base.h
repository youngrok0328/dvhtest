#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "grabber_info.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"

//HDR_3_________________________________ External library headers
#include <afxmt.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class SlitBeam3DParameters;

//HDR_6_________________________________ Header body
//
enum _enumTriggerModeChoice
{
    Trigger_Freerun = 0,
    Trigger_Trigger,
    Trigger_UplinkTrigger,
};

//mc_iGrabXE LOG Struct
struct iGrab3D_Error_LOG
{
    CString m_strPacketError;
    CString m_strCxpLinkCableError;
    CString m_strPageTrigger;
    long m_nExternalTrigger;
    long m_nReciveFrametoCamera;
    long m_nCustomGrabEventCount;

    //2023.02.23_DEV. Debug Log
    int m_hard_intr_count_s;
    int m_hard_intr_count_e;
    int m_soft_intr_count_s;
    int m_soft_intr_count_e;
    int m_cb_call_count;
    int m_cb_sum_count;
    //

    //2023.03.08_DEV. Debug Log
    DWORD m_dwDbgRunState;
    //

    //2023.05.30_DEV. Debug Log
    DWORD m_dwDbgMultGrabProcessReturnVal;
    //

    //2023.03.14_H/W Info
    float m_fCPU_Temperature;
    float m_fPC_UsagePhysicalMemory;
    float m_fiPack_UsagePhysicalMemory;
    //

    //2023.04.26_Cable State return val.
    BYTE m_byCxpLinkCableState;

    //2023.06.13_MultGrabWait return val.
    DWORD m_dwMultGrabWaitReturnVal;

    void Init()
    {
        m_strPacketError = _T("Empty");
        m_strCxpLinkCableError = _T("Empty");
        m_strPageTrigger = _T("Empty");
        m_nExternalTrigger = -1;
        m_nReciveFrametoCamera = -1;
        m_nCustomGrabEventCount = -1;

        //2023.02.23_DEV. Debug Log
        m_hard_intr_count_s = -1;
        m_hard_intr_count_e = -1;
        m_soft_intr_count_s = -1;
        m_soft_intr_count_e = -1;
        m_cb_call_count = -1;
        m_cb_sum_count = -1;
        //

        //2023.03.08_DEV. Debug Log
        m_dwDbgRunState = CAST_DWORD(-99999);
        //

        //2023.05.30_DEV. Debug Log
        m_dwDbgMultGrabProcessReturnVal = CAST_DWORD(-99999);
        //

        //2023.03.14_H/W Info
        m_fCPU_Temperature = -1.f;
        m_fPC_UsagePhysicalMemory = -1.f;
        m_fiPack_UsagePhysicalMemory = 1.f;
        //

        //2023.04.26_Cable State return val.
        m_byCxpLinkCableState = 0x00;

        //2023.06.13_MultGrabWait return val.
        m_dwMultGrabWaitReturnVal = 0x00;
    }
};

struct ColorGrabTime
{
    float m_fGrabEnd_Time;
    float m_fDM_CalEnd_Time;
    float m_fLAB_CalEnd_Time;

    void InitData()
    {
        m_fGrabEnd_Time = 0.f;
        m_fDM_CalEnd_Time = 0.f;
        m_fLAB_CalEnd_Time = 0.f;
    }
};

struct sGrab2DErrorLogMessageFormat
{
    CString m_strLotID;
    long m_nTrayIndex;
    long m_nFOVID; //Stitching ID랑 공용
    CString m_strGrabberGrabFailLog;

    void InitData()
    {
        //Data가 없다면
        //CString의 자료형은 Empty 가 초기값
        //long 의 자료형은 -1이 초기값

        m_strLotID = _T("Empty");
        m_nTrayIndex = -1;
        m_nFOVID = -1;
        m_strGrabberGrabFailLog = _T("Empty");
    }
};

struct sGrab3DErrorLogMessageFormat
{
    CString m_strLotID;
    long m_nTrayIndex;
    long m_nFOVID; //Stitching ID랑 공용
    CString m_strGrabberGrabFailLog;
    CString m_strGrabberGrabFailReturnCode;
    long m_nGrabberRecivedExternalTriggerCount; //외부로부터 받은 Trigger 개수
    long m_nGrabberRecive2CameraFrameNum; //Grabber가 Camera로 부터 받은 총 Frame수
    long m_nGrabberCustomGrabEventCount;
    long m_nGrabSuccessCount;
    long m_nGrabLineCount;
    long m_nGrabTriggerCount;
    long m_nSyncTriggerCount;
    CString m_strCSG_Time;
    CString m_strESG_Time;
    CString m_strOAS_Time;
    CString m_strOffVStart_Time;
    CString m_strGrabEnd_Time;
    CString m_strGrabberCalbeConnectStatus; //2진수로 표기
    CString m_strGrabberPacketStatus; //2진수로 표기

    //2023.02.23_DEV. Debug Log
    int m_hard_intr_count_s;
    int m_hard_intr_count_e;
    int m_soft_intr_count_s;
    int m_soft_intr_count_e;
    int m_cb_call_count;
    int m_cb_sum_count;
    //

    //2023.03.08_DEV. Debug Log
    DWORD m_dwDbgRunState;
    //

    //2023.05.30_DEV. Debug Log
    DWORD m_dwDbgMultGrabProcessReturnVal;
    //

    //2023.03.14_H/W Info
    float m_fCPU_Temperature = -1.f;
    float m_fPC_UsagePhysicalMemory = -1.f;
    float m_fiPack_UsagePhysicalMemory = 1.f;
    //

    void InitData()
    {
        //Data가 없다면
        //CString의 자료형은 Empty 가 초기값
        //long 의 자료형은 -1이 초기값

        m_strLotID = _T("Empty");
        m_nTrayIndex = -1;
        m_nFOVID = -1;
        m_strGrabberGrabFailLog = _T("Empty");
        m_strGrabberGrabFailReturnCode = _T("Empty");
        m_nGrabberRecivedExternalTriggerCount = -1;
        m_nGrabberRecive2CameraFrameNum = -1;
        m_nGrabberCustomGrabEventCount = -1;
        m_nGrabSuccessCount = -1;
        m_nGrabLineCount = -1;
        m_nGrabTriggerCount = -1;
        m_nSyncTriggerCount = -1;
        m_strCSG_Time = _T("Empty");
        m_strESG_Time = _T("Empty");
        m_strOAS_Time = _T("Empty");
        m_strOffVStart_Time = _T("Empty");
        m_strGrabEnd_Time = _T("Empty");
        m_strGrabberCalbeConnectStatus = _T("Empty");
        m_strGrabberPacketStatus = _T("Empty");

        //2023.02.23_DEV. Debug Log
        m_hard_intr_count_s = -1;
        m_hard_intr_count_e = -1;
        m_soft_intr_count_s = -1;
        m_soft_intr_count_e = -1;
        m_cb_call_count = -1;
        m_cb_sum_count = -1;
        //

        //2023.03.08_DEV. Debug Log
        m_dwDbgRunState = CAST_DWORD(-99999);
        //

        //2023.05.30_DEV. Debug Log
        m_dwDbgMultGrabProcessReturnVal = CAST_DWORD(-99999);
        //

        //2023.03.14_H/W Info
        m_fCPU_Temperature = -1.f;
        m_fPC_UsagePhysicalMemory = -1.f;
        m_fiPack_UsagePhysicalMemory = 1.f;
        //
    }
};

class FrameGrabber_Base
{
public:
    FrameGrabber_Base(void);
    virtual ~FrameGrabber_Base(void);

public:
    //Camera type이 추가되면 사용하는 인자를 변경
    void set_camera_param_save_path(const CString& path, const int& vision_type);

public: //Grabber F/W, Lib
    virtual void get_firmware_version(const int& grabber_id, unsigned int& cur_version, unsigned int& need_version);
    virtual void get_library_version(unsigned int& cur_version, unsigned int& need_version);

    CString get_version_info(const UINT& i_FirmwareVersion, const UINT& i_LibraryVersion) const;

public: // 기본 함수
    int get_grabber_num() const;
    int get_grabber_id(const int& grabber_idx = 0) const;
    int get_camera_num(const int& grabber_id = 0) const;
    int get_camera_id(const int& grabber_id = 0, const int& camera_idx = 0) const;
    int get_camera_icf_id(const int& grabber_id = 0, const int& camera_id = 0) const;

    bool set_grab_image_width(const int& grabber_idx = 0, const int& camera_idx = 0, const unsigned int& width = 0);
    bool set_grab_image_height(const int& grabber_idx = 0, const int& camera_idx = 0, const unsigned int& height = 0);
    bool set_grab_image_offset_Y(const int& grabber_idx = 0, const int& camera_idx = 0, const long& offset = 0);

    virtual bool apply_grab_image_offset_Y(const int& grabber_idx, const int& camera_idx, const long& offset);

    unsigned int get_grab_image_width(const int& grabber_idx = 0, const int& camera_idx = 0);
    unsigned int get_grab_image_height(const int& grabber_idx = 0, const int& camera_idx = 0);
    int get_grab_image_offset_Y(const int& grabber_idx = 0, const int& camera_idx = 0);

    static bool ExtractResourceFile(LPCTSTR type, LPCTSTR name, LPCTSTR targetPath);
    static bool SavesettingFile(const LPCTSTR& targetPath);

public: // 라이브 관련
    void live_on(const HWND& hwndEventListener, const UINT& uiEventMessage, const long& livePeriod_ms,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT,
        const int& grabMode = en3DVisionGrabMode::VISION_3D_GRABMODE_SPECULAR);
    void live_off();

    void get_live_image(Ipvm::Image8u& image);
    void get_live_image(Ipvm::Image8u3& image);

    void get_live_image2(Ipvm::Image8u& image);
    void get_live_image2(Ipvm::Image8u3& image);

    bool IsLive() const;

public: // 그랩 함수
    virtual bool grab(const int& grabber_idx, const int& camera_idx, BYTE** image, const int& grab_num,
        Ipvm::AsyncProgress* progress);
    virtual bool grab(const int grabber_idx, const int camera_idx, const int stitchIndex, Ipvm::Image32r& zmapImage,
        Ipvm::Image16u& vmapImage, float& height_range_min, float& height_range_max, Ipvm::AsyncProgress* progress);
    virtual bool multigrab(const int stitchIndex, VisionImageLot& imageLot, float& height_range_min,
        float& height_range_max, Ipvm::AsyncProgress* progress);

    virtual bool wait_grab_end(const int& grabber_idx, const int& camera_idx, Ipvm::AsyncProgress* progress);
    virtual bool wait_calc_end(
        const int& grabber_idx, const int& camera_idx, const bool& save_slit_images, Ipvm::AsyncProgress* progress);

    virtual bool re_calc_cuda_3D(const int& grabber_idx, const int& camera_idx); //kircheis_3DCalcRetry

    // iGrab Board Temperature
    virtual void set_grabber_temperature(const int& grabber_id = 0);
    float get_grabber_temperature(const int& grabber_id = 0) const;

public: // 3D 전용 함수
    virtual bool set_slitbeam_parameters(const int& grabber_idx, const int& camera_idx,
        const SlitBeam3DParameters& slitbeamPara, const float& height_scaling, const bool& use_height_quantization,
        const bool& use_distortion_compensation);
    virtual bool set_exposure_time(
        const int& grabber_idx, const int& camera_idx, const double& exposureHigh_ms, double& validFramePeriod_ms);
    virtual void set_live_image_size(
        const int& grabber_idx, const int& camera_idx, const bool& use_binning, const bool& is_full_size);
    virtual long get_grab_image_height_3D(
        const double& scan_depth_um, const bool& use_binning, const double& height_scaling) const;
    virtual float get_height_resolution_px2um(
        const bool& use_binning, const float& height_scaling, const bool& use_ideal_motor_speed = true) const;
    virtual long get_scan_length_px() const;
    virtual long get_scan_count() const;
    virtual void init_slit_beam_distortion();

    /// 아직안바꾼거
    virtual bool SetTriggerMode(int nMode) = 0; //	FrameGrabber의 Trigger Mode를 설정

    virtual CString Get3DGrabErrorMessage();
    virtual iGrab3D_Error_LOG GetiGrab3D_Error_LOG();
    virtual void SetiGrab3D_Error_LOG();

    //SDY 3D Noise 검사 알고리즘 추가
    virtual bool Check3DNoise(const unsigned int& grabber_id, const unsigned int& camera_id, BYTE** ppbyImg,
        Ipvm::Image8u& o_image, bool bFullSizeInsp = false);
    ///

public:
    bool DoColorConversion(Ipvm::Image8u i_imageMono, Ipvm::Image8u3& o_imageColor, const bool i_bIsWhiteBalance,
        const bool i_bChannelReverse);
    bool SendResizeImageToHost(
        Ipvm::Image8u i_Srcimage, Ipvm::Image8u& o_DstImage, long& o_nImageSizeX, long& o_nImageSizeY);
    bool SendResizeImageToHost(
        Ipvm::Image8u3 i_Srcimage, Ipvm::Image8u3& o_DstImage, long& o_nImageSizeX, long& o_nImageSizeY);
    bool DoSplitImageToRed(Ipvm::Image8u3 i_imageColor, Ipvm::Image8u& o_imageRed);
    bool DoSplitImageToGreen(Ipvm::Image8u3 i_imageColor, Ipvm::Image8u& o_imageGreen);
    bool DoSplitImageToBlue(Ipvm::Image8u3 i_imageColor, Ipvm::Image8u& o_imageBlue);

public: // Camera 관련 함수
    virtual bool OpenCameraComm(const int& camera_id);
    virtual bool CloseCameraComm();
    virtual bool SetDigitalGainValue(float i_fInputGain, BOOL bInline);
    virtual float GetCurrentCameraGain();

protected:
    virtual void init_grabber_info();
    virtual void init_camera_info(const CString& camera_param_file_path);

    bool set_grabber_num(const long& graaber_num);
    void set_camera_icf(const int& graaber_id, const int& camera_id, const int& icf_id);

    //입력받은 인자에서 연결된 graaber와 camera index가 유효한지
    bool is_valid_index(const int& grabber_idx = 0, const int& camera_idx = 0);

    void set_library_version(const unsigned int& library_version_cur);
    unsigned int get_use_library_version() const;
    void set_firmware_version(const int& grabber_id, const unsigned int& firmware_version);
    unsigned int get_use_firmware_version(const int& grabber_id) const;

    void apply_grabber_temperature(const int& grabber_id = 0, const float& temperature = -1.f);

    static UINT LiveThread(LPVOID pParam);
    UINT LiveThread();
        void Conversion_NGRV_InspectionMode_buf();
        void Conversion_NGRV_Mode_buf(const BOOL UseBayerPatternGPU);

    virtual bool LiveCallback(const int& graaber_id = 0, const int& camera_id = 0) = 0;

    CString get_camera_param_save_path() const;
    LPCTSTR get_camera_param_resourceID(const int& vision_type);

    //{{ Channel Gain value
    float m_fNormal_Channel_GainRed;
    float m_fNormal_Channel_GainGreen;
    float m_fNormal_Channel_GainBlue;

    float m_fReverse_Channel_GainRed;
    float m_fReverse_Channel_GainGreen;
    float m_fReverse_Channel_GainBlue;
    //}}

    CCriticalSection m_csLiveLock;

    //private: //함수 실패시, debug할 수 있도록 함수 이름을 popup
    void popup_function_error(const CString& name_function);

protected: //Live buf //이것도 camera 개수 따라가야하나.. 하...
    Ipvm::Image8u* live_buffer_mono;
    Ipvm::Image8u3* live_buffer_color;

    Ipvm::Image8u* live_buffer_mono2;
    Ipvm::Image8u3* live_buffer_color2;

private:
    unsigned int library_version;

    enSideVisionModule m_eCurVisionModule; //나중에 삭제예정...

    std::vector<grabber_info> grabber_info; //연결된 grabber 정보 및 camera index는 통합으로 관리

    std::map<grabber_id, std::map<camera_channel, unsigned int>> grab_image_widths;
    std::map<grabber_id, std::map<camera_channel, unsigned int>> grab_image_heights;
    std::map<grabber_id, std::map<camera_channel, long>> grab_image_offsets_Y;

    HWND hwnd_event_listener;
    UINT ui_event_message;

    HANDLE live_flag;
    HANDLE live_thread;
    HANDLE live_thread_kill;

    long live_period_ms;
    
    CString camera_param_path;

    long m_grabMode;
};
