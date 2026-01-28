#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class FrameGrabber_Base;
class SlitBeam3DParameters;
struct iGrab3D_Error_LOG;

//HDR_6_________________________________ Header body
//

class DPI_FRAMEGRABBERDEF_API FrameGrabber
{
public:
    FrameGrabber(void);
    ~FrameGrabber(void);

public: // 기본 함수
    static FrameGrabber& GetInstance();

public:
    void get_firmware_version(const int& grabber_id, unsigned int& cur_version, unsigned int& need_version);
    void get_library_version(unsigned int& cur_version, unsigned int& need_version);

    unsigned int get_grab_image_width(const int& grabber_idx = 0, const int& camera_idx = 0) const;
    unsigned int get_grab_image_height(const int& grabber_idx = 0, const int& camera_idx = 0) const;
    long get_grab_image_offset_Y(const int& grabber_idx = 0, const int& camera_idx = 0) const;

    bool set_grab_image_offset_Y(const int& grabber_idx = 0, const int& camera_idx = 0, const long& offset = 0);

    void set_grabber_temperature(const int& grabber_id = 0);
    float get_grabber_temperature(const int& grabber_id = 0) const;

    long get_grab_image_height_3D(const double& scan_depth_um, const bool& use_binning, const double& height_scaling);

    static void Convert3DtoGray(const Ipvm::Image32r& zmap, const Ipvm::Image16u& vmap, float heightRangeMin,
        float heightRangeMax, Ipvm::Image8u& o_zmapGray, Ipvm::Image8u& o_vmapGray);

public: // 라이브 관련
    void live_on(const HWND& event_listener, const UINT& event_message, const long& livePeriod_ms,
        const int& camera_idx = 0, const int& grabMode = en3DVisionGrabMode::VISION_3D_GRABMODE_SPECULAR);
    void live_off();

    void get_live_image(Ipvm::Image8u& image);
    void get_live_image(Ipvm::Image8u3& image);

    void get_live_image2(Ipvm::Image8u& image);
    void get_live_image2(Ipvm::Image8u3& image);

    bool IsLive() const;

public: // 그랩 함수
    bool StartGrab2D(const int& grabber_idx, const int& camera_idx, BYTE** image, const int& grab_num,
        Ipvm::AsyncProgress* progress);
    bool StartGrab3D(const int grabber_idx, const int camera_idx, const int stitchIndex, Ipvm::Image32r& zmapImage,
        Ipvm::Image16u& vmapImage, float& height_range_min, float& height_range_max, Ipvm::AsyncProgress* progress);
    bool StartDualGrab3D(const int stitchIndex, VisionImageLot& imageLot, float& height_range_min,
        float& height_range_max, Ipvm::AsyncProgress* progress);

    bool wait_grab_end(const int& grabber_idx, const int& camera_idx, Ipvm::AsyncProgress* progress);
    bool wait_calc_end(
        const int& grabber_idx, const int& camera_idx, const bool& save_slit_images, Ipvm::AsyncProgress* progress);

    bool re_calc_cuda_3D(const int& grabber_idx, const int& camera_idx); //kircheis_3DCalcRetry

public: // 3D 전용 함수
    bool set_slitbeam_parameters(const int& grabber_idx, const int& camera_idx,
        const SlitBeam3DParameters& slitbeamPara, const float& height_scaling, const bool& use_height_quantization,
        const bool& use_distortion_compensation);
    bool set_exposure_time(
        const int& grabber_idx, const int& camera_idx, const double& exposureHigh_ms, double& validFramePeriod_ms);
    void set_live_image_size(
        const int& grabber_idx, const int& camera_idx, const bool& use_binning, const bool& is_full_size);
    long get_grab_image_height(
        const double& scan_depth_um, const bool& use_binning, const double& height_scaling) const;
    float get_height_resolution_px2um(
        const bool& use_binning, const float& height_scaling, const bool& use_ideal_motor_speed = true) const;
    long get_scan_length_px() const;
    long get_scan_count() const;
    void init_slit_beam_distortion();

    CString Get3DGrabErrorMessage();
    iGrab3D_Error_LOG GetiGrab3D_Error_LOG();
    void SetiGrab3D_Error_LOG();

    //SDY 3D Noise 검사 알고리즘 추가
    bool Check3DNoise(const unsigned int& grabber_id, const unsigned int& camera_id, BYTE** ppbyImg,
        Ipvm::Image8u& o_image, bool bFullSizeInsp = false);

public:
    bool DoColorConversion(Ipvm::Image8u i_imageMono, Ipvm::Image8u3& o_imageColor, const bool i_bIsWhiteBalance,
        const bool i_bChannelReverse); // JHB
    bool SendResizeImageHost(
        Ipvm::Image8u i_Srcimage, Ipvm::Image8u& o_DstImage, long& o_nImageSizeX, long& o_nImageSizeY);
    bool SendResizeImageHost(
        Ipvm::Image8u3 i_Srcimage, Ipvm::Image8u3& o_DstImage, long& o_nImageSizeX, long& o_nImageSizeY);
    bool DoSplitImageToRed(Ipvm::Image8u3 i_imageColor, Ipvm::Image8u& o_imageRed);
    bool DoSplitImageToGreen(Ipvm::Image8u3 i_imageColor, Ipvm::Image8u& o_imageGreen);
    bool DoSplitImageToBlue(Ipvm::Image8u3 i_imageColor, Ipvm::Image8u& o_imageBlue);

    bool OpenCameraComm();
    bool CloseCameraComm();
    bool SetDigitalGainValue(float i_fInputGain, BOOL bInline);
    float GetCurrentCameraGain(); // 현재 NGRV 카메라의 Gain 값 저장 - JHB_2023.02.07

public: //mc_F/W & Lib. Version String
    CString GetstrVersioninfo(const UINT i_FirmwareVersion, const UINT i_LibraryVersion);

private:
    FrameGrabber_Base* get_FrameGrabber(const BOOL& enable_hardware);

private:
    FrameGrabber_Base* m_impl;
};
