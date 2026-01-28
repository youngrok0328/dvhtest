#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "FrameGrabber_Base.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
namespace IntekPlus
{
namespace VisionMadang
{
class SbtCore;
struct SbtCoreParameters; //kircheis_SDK127
} // namespace VisionMadang
} // namespace IntekPlus

class SlitBeam3DParameters;
class SlitbeamDistortionData;

//HDR_6_________________________________ Header body
//
class FrameGrabber3D_iGrabXE : public FrameGrabber_Base
{
public:
    FrameGrabber3D_iGrabXE(void);
    ~FrameGrabber3D_iGrabXE(void) override;

public: // 그랩 함수
    virtual bool grab(const int grabber_idx, const int camera_idx, const int stitchIndex, Ipvm::Image32r& zmapImage,
        Ipvm::Image16u& vmapImage, float& height_range_min, float& height_range_max,
        Ipvm::AsyncProgress* progress) override;

    virtual bool wait_grab_end(const int& grabber_idx, const int& camera_idx, Ipvm::AsyncProgress* progress) override;
    virtual bool wait_calc_end(const int& grabber_idx, const int& camera_idx, const bool& save_slit_images,
        Ipvm::AsyncProgress* progress) override;

    virtual bool re_calc_cuda_3D(const int& grabber_idx, const int& camera_idx) override; //kircheis_3DCalcRetry

public: // 3D 전용 함수
    virtual bool set_exposure_time(const int& grabber_idx, const int& camera_idx, const double& exposureHigh_ms,
        double& validFramePeriod_ms) override;
    virtual bool set_slitbeam_parameters(const int& grabber_idx, const int& camera_idx,
        const SlitBeam3DParameters& slitbeamPara, const float& height_scaling, const bool& use_height_quantization,
        const bool& use_distortion_compensation) override; //kircheis_3DGrabIssue

    virtual void set_live_image_size(
        const int& grabber_idx, const int& camera_idx, const bool& use_binning, const bool& is_full_size) override;
    virtual long get_grab_image_height_3D(
        const double& scan_depth_um, const bool& use_binning, const double& height_scaling) const override;
    virtual float get_height_resolution_px2um(
        const bool& use_binning, const float& height_scaling, const bool& use_ideal_motor_speed = true) const override;
    virtual long get_scan_length_px() const override;
    virtual long get_scan_count() const override;
    virtual void init_slit_beam_distortion() override;
    virtual CString Get3DGrabErrorMessage() override;
    virtual iGrab3D_Error_LOG GetiGrab3D_Error_LOG() override;
    virtual void SetiGrab3D_Error_LOG() override;

    //SDY 3D Noise 검사 알고리즘 추가
    // SDY 3D 카메라의 2D Grab을 통한 카메라 상태 확인 추가를 위해 추가
    virtual bool Check3DNoise(const unsigned int& grabber_id, const unsigned int& camera_id, BYTE** ppbyImg,
        Ipvm::Image8u& o_image, bool bFullSizeInsp = false) override;

public: //F/W & Lib. Version
    virtual void get_firmware_version(
        const int& grabber_id, unsigned int& cur_version, unsigned int& need_version) override;
    virtual void get_library_version(unsigned int& cur_version, unsigned int& need_version) override;

public: // iGrab Board Temperature
    virtual void set_grabber_temperature(const int& grabber_id = 0) override;

protected:
    virtual void init_grabber_info() override;
    virtual void init_camera_info(const CString& camera_param_file_path) override;
    virtual bool LiveCallback(const int& graaber_id = 0, const int& camera_id = 0) override;
    virtual bool SetTriggerMode(int nMode) override;

private:
    bool init_dsnu_image_buf(const int32_t& width, const int32_t& height);
    bool init_camera_serial();
    bool get_device_serial(const int& camera_id, CString& serialNumber);
    bool init_image_size_default(const unsigned int& width, const unsigned int& height);
    bool init_camera_default_param();
    bool apply_camera_additional_param();
    bool apply_camera_dsnu();

    void Save2DImage(const unsigned int& image_width, const unsigned int& image_height, const long splitSetIndex);

    bool load_dsnu_image(const CString& camera_serial, Ipvm::Image8u& dsum_img, Ipvm::Image8u& dsum_binning_img);
    bool make_dsnu_image(const int& grabber_id, const int& camera_id, const int& icf_id, int& custom_grab_count,
        Ipvm::Image8u& dsnu_image, Ipvm::Image8u& dsnu_binning_image, const CString& camera_serial);
    bool ChangeVerticalSize(const int& grabber_id, const int& camera_id, const int& icf_id,
        const bool cameraVerticalBinning, const bool fullSizeY, const long scanSizeY);

    static void ScanEnd(void* pUserData, DWORD nCount, void* pParam1, void* pParam2);
    void ScanEnd(DWORD nCount, BYTE* pDmaBuffer);

    static void LiveEnd(void* pUserData, DWORD nCount, void* pParam1, void* pParam2);
    void LiveEnd(DWORD nCount, BYTE* pDmaBuffer);

    void PostWaitCalcEndWork(); //kircheis_3DCalcRetry

    //최대로 사용할 수 있는 camera 개수만큼 할당된 channel을 확인, channel을 할당하지 않았다면 fase
    bool check_is_valid_camera_channel(const int& id_graaber, const int& camera_idx);

private:
    unsigned int grabber_id_use_grab; //grab당시 사용된 grabber id
    unsigned int camera_id_use_grab; //grab당시 사용된 camera id

    long m_profileNumber;
    long m_scanImageCount;
    CString m_strErrorMessage;

    std::map<grabber_id, std::map<camera_channel, CString>> camera_serials;
    std::map<grabber_id, std::map<camera_channel, int>> custom_grab_event_counts;
    std::map<grabber_id, std::map<camera_channel, Ipvm::Image8u>> dsnu_images;
    std::map<grabber_id, std::map<camera_channel, Ipvm::Image8u>> dsnu_binning_images;
    std::map<grabber_id, std::map<camera_channel, bool>> m_use_binning;

    std::vector<Ipvm::Image8u> m_scanBuffers;

    SlitbeamDistortionData* m_distortionData;

    Ipvm::Image32r* m_zmapImage;
    Ipvm::Image32r* m_vmapImage;
    Ipvm::Image16u* m_vmapResizeTempImage;

    DWORD m_dwCurrentThreadPriority;

    Ipvm::Image32r* m_resultZmapImage;
    Ipvm::Image16u* m_resultVmapImage;

    long m_resultStitchIndex;

    Ipvm::SbtCore* m_pCore;
    Ipvm::SbtCorePara* m_pCoreParam;

private: //mc_iGrabXE Error LOG
    CString GetCxpLinkfPacketError(const unsigned int& grabber_id, const unsigned int& camera_id);
    CString GetCurrentEnable_Channel_Connection_State(const unsigned int& grabber_id);
    long GetReciveExternalTrigger(const unsigned int& grabber_id, const unsigned int& camera_id);
    CString GetPageTrigger_State(const unsigned int& grabber_id, const unsigned int& camera_id);
    long GetReciveFrameCount2Camera(const unsigned int& grabber_id, const unsigned int& camera_id);

    CString GetExceptionErrorMSG(
        const BYTE i_byMSGValue); //mc_정의한 값에 대한 예외일 경우 입력받은 값을 그대로 String으로 출력한다

    iGrab3D_Error_LOG m_siGrab3D_Error_LOG;
    void ResetiGrab3D_Error_LOG();
};
