//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "FrameGrabber.h"

//CPP_2_________________________________ This project's headers
#include "Framegrabber2D_Dummy.h"
#include "Framegrabber2D_iGrabG.h"
#include "Framegrabber2D_iGrabXQ.h"
#include "Framegrabber2D_iGrabXE.h"
#include "Framegrabber3D_Dummy.h"
#include "Framegrabber3D_iGrabXE.h"
#include "Framegrabber3D_iGrabXQ.h"
#include "Framegrabber_Base.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedBuildConfiguration/iGrabXQ/iManX.h"
#include "../../SharedBuildConfiguration/iGrabXQ/iManXDef.h"
#include "../../SharedBuildConfiguration/iGrabXE/iManX.h"
#include "../../SharedBuildConfiguration/iGrabXE/iManXDef.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Image16u.h>
#include <Ipvm/Base/Image32r.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Image8u3.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
FrameGrabber::FrameGrabber(void)
    : m_impl(nullptr)
{
    m_impl = get_FrameGrabber(SystemConfig::GetInstance().m_bHardwareExist);
}

FrameGrabber_Base* FrameGrabber::get_FrameGrabber(const BOOL& enable_hardware)
{
    //여기서 CameraType을 정의한게 있어야 하는데..?
    if (enable_hardware == TRUE)
    {
        static const int FrameGrabberType = SystemConfig::GetInstance().GetFrameGrabberType();

        switch (SystemConfig::GetInstance().GetVisionType())
        {
            case VISIONTYPE_2D_INSP:
            {
                switch (FrameGrabberType)
                {
                    case enFrameGrabberType::FG_TYPE_IGRAB_G2_SERIES:
                        return new Framegrabber2D_iGrabG();
                    case enFrameGrabberType::FG_TYPE_IGRAB_XQ:
                        return new Framegrabber2D_iGrabXQ();
                    case enFrameGrabberType::FG_TYPE_IGRAB_XE:
                        return new Framegrabber2D_iGrabXE();
                }
            }
            break;
            case VISIONTYPE_3D_INSP:
                switch (FrameGrabberType)
                {
                    case enFrameGrabberType::FG_TYPE_IGRAB_XE:
                        return new FrameGrabber3D_iGrabXE();
                        break;
                    case enFrameGrabberType::FG_TYPE_IGRAB_XQ:
                        return new Framegrabber3D_iGrabXQ();
                        break;
                }
                break;
            case VISIONTYPE_NGRV_INSP:
                return new Framegrabber2D_iGrabXQ();
                break;
            case VISIONTYPE_SIDE_INSP:
                return new Framegrabber2D_iGrabXQ();
                break;
            case VISIONTYPE_SWIR_INSP:
                return new Framegrabber2D_iGrabXQ();
                break;
            case VISIONTYPE_TR:
                return new Framegrabber2D_iGrabG();
                break;
            default:
                ::AfxMessageBox(_T("Vision type is invalid"), MB_ICONERROR | MB_OK);
                ::exit(0);
        }
    }
    else
    {
        switch (SystemConfig::GetInstance().GetVisionType())
        {
            case VISIONTYPE_2D_INSP:
                return new Framegrabber2D_Dummy();
                break;
            case VISIONTYPE_3D_INSP:
                return new FrameGrabber3D_Dummy();
                break;
            case VISIONTYPE_NGRV_INSP:
                return new Framegrabber2D_Dummy();
                break;
            case VISIONTYPE_SIDE_INSP:
                return new Framegrabber2D_Dummy();
                break;
            case VISIONTYPE_SWIR_INSP: //kircheis_SWIR
                return new Framegrabber2D_Dummy();
                break;
            case VISIONTYPE_TR:
                return new Framegrabber2D_Dummy();
                break;
            default:
                ::AfxMessageBox(_T("Vision type is invalid"), MB_ICONERROR | MB_OK);
                ::exit(0);
        }
    }

    return new Framegrabber2D_Dummy();
}

FrameGrabber::~FrameGrabber(void)
{
    if (m_impl != nullptr)
        delete m_impl;
}

FrameGrabber& FrameGrabber::GetInstance()
{
    static FrameGrabber singleton;

    return singleton;
}

void FrameGrabber::get_firmware_version(const int& grabber_id, unsigned int& cur_version, unsigned int& need_version)
{
    m_impl->get_firmware_version(grabber_id, cur_version, need_version);
}

void FrameGrabber::get_library_version(unsigned int& cur_version, unsigned int& need_version)
{
    m_impl->get_library_version(cur_version, need_version);
}

unsigned int FrameGrabber::get_grab_image_width(const int& grabber_idx, const int& camera_idx) const
{
    return m_impl->get_grab_image_width(grabber_idx, camera_idx);
}

unsigned int FrameGrabber::get_grab_image_height(const int& grabber_idx, const int& camera_idx) const
{
    return m_impl->get_grab_image_height(grabber_idx, camera_idx);
}

long FrameGrabber::get_grab_image_offset_Y(const int& grabber_idx, const int& camera_idx) const
{
    return m_impl->get_grab_image_offset_Y(grabber_idx, camera_idx);
}

bool FrameGrabber::set_grab_image_offset_Y(const int& grabber_idx, const int& camera_idx, const long& offset)
{
    return m_impl->apply_grab_image_offset_Y(grabber_idx, camera_idx, offset);
}

void FrameGrabber::set_grabber_temperature(const int& grabber_id)
{
    m_impl->set_grabber_temperature(grabber_id);
}

float FrameGrabber::get_grabber_temperature(const int& grabber_id) const
{
    return m_impl->get_grabber_temperature(grabber_id);
}

long FrameGrabber::get_grab_image_height_3D(
    const double& scan_depth_um, const bool& use_binning, const double& height_scaling)
{
    return m_impl->get_grab_image_height_3D(scan_depth_um, use_binning, height_scaling);
}

void FrameGrabber::Convert3DtoGray(const Ipvm::Image32r& zmap, const Ipvm::Image16u& vmap, float heightRangeMin,
    float heightRangeMax, Ipvm::Image8u& o_zmapGray, Ipvm::Image8u& o_vmapGray)
{
    Ipvm::ImageProcessing::ScaleInRangeWithNoise(
        zmap, Ipvm::Rect32s(zmap), Ipvm::k_noiseValue32r, heightRangeMin, heightRangeMax, 0, 1, 255, o_zmapGray);
    Ipvm::ImageProcessing::Scale(vmap, Ipvm::Rect32s(vmap), o_vmapGray);
}

void FrameGrabber::live_on(const HWND& event_listener, const UINT& event_message, const long& livePeriod_ms,
    const int& camera_idx, const int& grabMode)
{
    static const bool isNGRV = SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP;
    if (isNGRV == true)
        m_impl->SetTriggerMode(_enumTriggermode::UplinkTrigger);

    m_impl->live_on(event_listener, event_message, livePeriod_ms, enSideVisionModule(camera_idx), grabMode);
}

void FrameGrabber::live_off()
{
    static const bool isNGRV = SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP;

    m_impl->live_off();

    if (isNGRV == true)
        m_impl->SetTriggerMode(_enumTriggermode::Freerun);
}

void FrameGrabber::get_live_image(Ipvm::Image8u& image)
{
    return m_impl->get_live_image(image);
}

void FrameGrabber::get_live_image(Ipvm::Image8u3& image)
{
    return m_impl->get_live_image(image);
}

void FrameGrabber::get_live_image2(Ipvm::Image8u& image)
{
    return m_impl->get_live_image2(image);
}

void FrameGrabber::get_live_image2(Ipvm::Image8u3& image)
{
    return m_impl->get_live_image2(image);
}

bool FrameGrabber::IsLive() const
{
    return m_impl->IsLive();
}

bool FrameGrabber::StartGrab2D(
    const int& grabber_idx, const int& camera_idx, BYTE** image, const int& grab_num, Ipvm::AsyncProgress* progress)
{
    return m_impl->grab(grabber_idx, camera_idx, image, grab_num, progress);
}

bool FrameGrabber::StartGrab3D(const int grabber_idx, const int camera_idx, const int stitchIndex,
    Ipvm::Image32r& zmapImage, Ipvm::Image16u& vmapImage, float& height_range_min, float& height_range_max,
    Ipvm::AsyncProgress* progress)
{
    return m_impl->grab(
        grabber_idx, camera_idx, stitchIndex, zmapImage, vmapImage, height_range_min, height_range_max, progress);
}

bool FrameGrabber::StartDualGrab3D(const int stitchIndex, VisionImageLot& imageLot, float& height_range_min,
    float& height_range_max, Ipvm::AsyncProgress* progress)
{
    return m_impl->multigrab(stitchIndex, imageLot, height_range_min, height_range_max, progress);
}

bool FrameGrabber::wait_grab_end(const int& grabber_idx, const int& camera_idx, Ipvm::AsyncProgress* progress)
{
    return m_impl->wait_grab_end(grabber_idx, camera_idx, progress);
}

bool FrameGrabber::wait_calc_end(
    const int& grabber_idx, const int& camera_idx, const bool& save_slit_images, Ipvm::AsyncProgress* progress)
{
    return m_impl->wait_calc_end(grabber_idx, camera_idx, save_slit_images, progress);
}

bool FrameGrabber::re_calc_cuda_3D(const int& grabber_idx, const int& camera_idx)
{
    return m_impl->re_calc_cuda_3D(grabber_idx, camera_idx);
}

bool FrameGrabber::set_slitbeam_parameters(const int& grabber_idx, const int& camera_idx,
    const SlitBeam3DParameters& slitbeamPara, const float& height_scaling, const bool& use_height_quantization,
    const bool& use_distortion_compensation)
{
    return m_impl->set_slitbeam_parameters(
        grabber_idx, camera_idx, slitbeamPara, height_scaling, use_height_quantization, use_distortion_compensation);
}

bool FrameGrabber::set_exposure_time(
    const int& grabber_idx, const int& camera_idx, const double& exposureHigh_ms, double& validFramePeriod_ms)
{
    return m_impl->set_exposure_time(grabber_idx, camera_idx, exposureHigh_ms, validFramePeriod_ms);
}

void FrameGrabber::set_live_image_size(
    const int& grabber_idx, const int& camera_idx, const bool& use_binning, const bool& is_full_size)
{
    m_impl->set_live_image_size(grabber_idx, camera_idx, use_binning, is_full_size);
}

long FrameGrabber::get_grab_image_height(
    const double& scan_depth_um, const bool& use_binning, const double& height_scaling) const
{
    return m_impl->get_grab_image_height_3D(scan_depth_um, use_binning, height_scaling);
}

float FrameGrabber::get_height_resolution_px2um(
    const bool& use_binning, const float& height_scaling, const bool& use_ideal_motor_speed) const
{
    return m_impl->get_height_resolution_px2um(use_binning, height_scaling, use_ideal_motor_speed);
}

long FrameGrabber::get_scan_length_px() const
{
    return m_impl->get_scan_length_px();
}

long FrameGrabber::get_scan_count() const
{
    return m_impl->get_scan_count();
}

void FrameGrabber::init_slit_beam_distortion()
{
    m_impl->init_slit_beam_distortion();
}

CString FrameGrabber::Get3DGrabErrorMessage()
{
    return m_impl->Get3DGrabErrorMessage();
}

iGrab3D_Error_LOG FrameGrabber::GetiGrab3D_Error_LOG()
{
    return m_impl->GetiGrab3D_Error_LOG();
}

void FrameGrabber::SetiGrab3D_Error_LOG()
{
    return m_impl->SetiGrab3D_Error_LOG();
}

bool FrameGrabber::Check3DNoise(const unsigned int& grabber_id, const unsigned int& camera_id, BYTE** ppbyImg,
    Ipvm::Image8u& o_image, bool bFullSizeInsp)
{
    return m_impl->Check3DNoise(grabber_id, camera_id, ppbyImg, o_image, bFullSizeInsp);
}

bool FrameGrabber::DoColorConversion(
    Ipvm::Image8u i_imageMono, Ipvm::Image8u3& o_imageColor, const bool i_bIsWhiteBalance, const bool i_bChannelReverse)
{
    return m_impl->DoColorConversion(i_imageMono, o_imageColor, i_bIsWhiteBalance, i_bChannelReverse);
}

bool FrameGrabber::SendResizeImageHost(
    Ipvm::Image8u3 i_Srcimage, Ipvm::Image8u3& o_DstImage, long& o_nImageSizeX, long& o_nImageSizeY)
{
    return m_impl->SendResizeImageToHost(i_Srcimage, o_DstImage, o_nImageSizeX, o_nImageSizeY);
}

bool FrameGrabber::SendResizeImageHost(
    Ipvm::Image8u i_Srcimage, Ipvm::Image8u& o_DstImage, long& o_nImageSizeX, long& o_nImageSizeY)
{
    return m_impl->SendResizeImageToHost(i_Srcimage, o_DstImage, o_nImageSizeX, o_nImageSizeY);
}

bool FrameGrabber::DoSplitImageToRed(Ipvm::Image8u3 i_imageColor, Ipvm::Image8u& o_imageRed)
{
    return m_impl->DoSplitImageToRed(i_imageColor, o_imageRed);
}

bool FrameGrabber::DoSplitImageToGreen(Ipvm::Image8u3 i_imageColor, Ipvm::Image8u& o_imageGreen)
{
    return m_impl->DoSplitImageToGreen(i_imageColor, o_imageGreen);
}

bool FrameGrabber::DoSplitImageToBlue(Ipvm::Image8u3 i_imageColor, Ipvm::Image8u& o_imageBlue)
{
    return m_impl->DoSplitImageToBlue(i_imageColor, o_imageBlue);
}

bool FrameGrabber::OpenCameraComm()
{
    return m_impl->OpenCameraComm(0);
}

bool FrameGrabber::CloseCameraComm()
{
    return m_impl->CloseCameraComm();
}

bool FrameGrabber::SetDigitalGainValue(float i_fInputGain, BOOL bInline) // Camera Gain Control - JHB_NGRV
{
    static const bool isNgrvColorOptics = SystemConfig::GetInstance().IsNgrvColorOptics();
    if (isNgrvColorOptics == false)
        return true;

    return m_impl->SetDigitalGainValue(i_fInputGain, bInline);
}

float FrameGrabber::GetCurrentCameraGain()
{
    static const bool isNgrvColorOptics = SystemConfig::GetInstance().IsNgrvColorOptics();
    if (isNgrvColorOptics == false)
        return 1.0f;

    return m_impl->GetCurrentCameraGain();
}

CString FrameGrabber::GetstrVersioninfo(const UINT i_FirmwareVersion, const UINT i_LibraryVersion)
{
    return m_impl->get_version_info(i_FirmwareVersion, i_LibraryVersion);
}