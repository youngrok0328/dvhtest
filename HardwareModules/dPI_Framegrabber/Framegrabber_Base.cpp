//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Framegrabber_Base.h"

//CPP_2_________________________________ This project's headers
#include "Resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"

// mc_임시방편
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
//
//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/AlgorithmGpu/ImageProcessingGpu.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Image8u3.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Widget/AsyncProgress.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
FrameGrabber_Base::FrameGrabber_Base(void)
    : library_version(0)
    , m_eCurVisionModule(enSideVisionModule::SIDE_VISIONMODULE_FRONT)
    , hwnd_event_listener(NULL)
    , ui_event_message(0)
    , live_flag(::CreateEvent(nullptr, TRUE, FALSE, nullptr))
    , live_thread(INVALID_HANDLE_VALUE)
    , live_thread_kill(::CreateEvent(nullptr, FALSE, FALSE, nullptr))
    , live_period_ms(200)
    , live_buffer_mono(new Ipvm::Image8u)
    , live_buffer_color(new Ipvm::Image8u3)
    , camera_param_path("")
    , m_fNormal_Channel_GainRed(1.f)
    , m_fNormal_Channel_GainGreen(1.f)
    , m_fNormal_Channel_GainBlue(1.f)
    , m_fReverse_Channel_GainRed(1.f)
    , m_fReverse_Channel_GainGreen(1.f)
    , m_fReverse_Channel_GainBlue(1.f)

    , live_buffer_mono2(new Ipvm::Image8u)
    , live_buffer_color2(new Ipvm::Image8u3)
    , m_grabMode(en3DVisionGrabMode::VISION_3D_GRABMODE_SPECULAR)
{
    HANDLE handle = ::AfxBeginThread(LiveThread, this)->m_hThread;

    ::DuplicateHandle(
        ::GetCurrentProcess(), handle, ::GetCurrentProcess(), &live_thread, 0, FALSE, DUPLICATE_SAME_ACCESS);

    set_camera_param_save_path(DynamicSystemPath::get(DefineFolder::Temp), SystemConfig::GetInstance().GetVisionType());
}

FrameGrabber_Base::~FrameGrabber_Base(void)
{
    ::SetEvent(live_thread_kill);

    ::WaitForSingleObject(live_thread, 5000);

    ::CloseHandle(live_thread_kill);
    ::CloseHandle(live_thread);
    ::CloseHandle(live_flag);

    delete live_buffer_mono;
    delete live_buffer_color;

    delete live_buffer_mono2;
    delete live_buffer_color2;
}

void FrameGrabber_Base::set_camera_param_save_path(const CString& path, const int& vision_type)
{
    CString camera_param_file_name("");
    switch (vision_type)
    {
        case VISIONTYPE_2D_INSP:
        {
            if (SystemConfig::GetInstance().Get2DVisionCameraType() == CAMERATYPE_LAON_PEOPLE_CXP25M)
            {
                camera_param_file_name = _T("\\LPMVC-CL025M.icf");
            }
            else if (SystemConfig::GetInstance().Get2DVisionCameraType() == CAMERATYPE_DALSA_GENIE_NANO_CXP_67M)
            {
                camera_param_file_name = _T("\\DALSA-GENIE-NANO-CXP67M.xcf");
            }
            else
            {
                camera_param_file_name = _T("\\LPMVC-CL025M.icf");
            }
        }
        break;
        case VISIONTYPE_3D_INSP:
        {
            if (SystemConfig::GetInstance().Get3DVisionCameraType() == CAMERATYPE_VIEWWORKS_12MX)
            {
                camera_param_file_name = _T("\\vc_12mx2-0.5.1.xml");
            }
            else if (SystemConfig::GetInstance().Get3DVisionCameraType() == CAMERATYPE_MIKROTRON_EOSENS_21CXP2)
            {
                camera_param_file_name = _T("\\MIKROTRON_21CXP.xcf");
            }
            else
            {
                camera_param_file_name = _T("\\vc_12mx2-0.5.1.xml");
            }
        }
        break;
        case VISIONTYPE_NGRV_INSP:
        {
            camera_param_file_name = _T("vc-65mx-0.2.1.xcf");
        }
        break;
        case VISIONTYPE_SIDE_INSP:
        {
            camera_param_file_name = _T("boA8100_16cm.xcf");
        }
        break;
        case VISIONTYPE_SWIR_INSP:
        {
            camera_param_file_name = _T("fxo992MCX.xcf");
        }
        break;
        case VISIONTYPE_TR:
        {
            camera_param_file_name = _T("STC-CMB4MCL.icf");
        }
        break;
        default:
            break;
    }

    camera_param_path = path + camera_param_file_name;

}

CString FrameGrabber_Base::get_camera_param_save_path() const
{
    return camera_param_path;
}

LPCTSTR FrameGrabber_Base::get_camera_param_resourceID(const int& vision_type)
{
    switch (vision_type)
    {
        case VISIONTYPE_2D_INSP:
        {
            if (SystemConfig::GetInstance().Get2DVisionCameraType() == CAMERATYPE_LAON_PEOPLE_CXP25M)
            {
                return MAKEINTRESOURCE(IDR_ICF_LPMVC_CL025M);
            }
            else if (SystemConfig::GetInstance().Get2DVisionCameraType() == CAMERATYPE_DALSA_GENIE_NANO_CXP_67M)
            {
                return MAKEINTRESOURCE(IDR_XCF_DALSA_GENIE_NANO_CXP67M);
            }
            else
            {
                return MAKEINTRESOURCE(IDR_ICF_LPMVC_CL025M);
            }
        }
        break;
        case VISIONTYPE_3D_INSP:
        {
            if (SystemConfig::GetInstance().Get3DVisionCameraType() == CAMERATYPE_VIEWWORKS_12MX)
            {
                return MAKEINTRESOURCE(IDR_XML_VC_12MX2_0_5_1);
            }
            else if (SystemConfig::GetInstance().Get3DVisionCameraType() == CAMERATYPE_MIKROTRON_EOSENS_21CXP2)
            {
                return MAKEINTRESOURCE(IDR_XCF_MIKROTRON_21CXP);
            }
            else
            {
                return MAKEINTRESOURCE(IDR_XML_VC_12MX2_0_5_1);
            }
        }
        break;
        case VISIONTYPE_NGRV_INSP:
        {
            MAKEINTRESOURCE(IDR_XCF_VC_65MX_0_2_1);
        }
        break;
        case VISIONTYPE_SIDE_INSP:
        {
            MAKEINTRESOURCE(IDR_XCF_BOA8100_16CM);
        }
        break;
        case VISIONTYPE_SWIR_INSP:
        {
            MAKEINTRESOURCE(IDR_XCF_FXO992MCX);
        }
        break;
        case VISIONTYPE_TR:
        {
            MAKEINTRESOURCE(IDR_ICF_STC_CMB4MCL);
        }
        break;
        default:
            break;
    }

    return _T("");
}

void FrameGrabber_Base::get_firmware_version(
    const int& grabber_id, unsigned int& cur_version, unsigned int& need_version)
{
    UNREFERENCED_PARAMETER(grabber_id);
    UNREFERENCED_PARAMETER(cur_version);
    UNREFERENCED_PARAMETER(need_version);

    return;
}

void FrameGrabber_Base::get_library_version(unsigned int& cur_version, unsigned int& need_version)
{
    UNREFERENCED_PARAMETER(cur_version);
    UNREFERENCED_PARAMETER(need_version);

    return;
}

CString FrameGrabber_Base::get_version_info(const UINT& i_FirmwareVersion, const UINT& i_LibraryVersion) const
{
    if (i_FirmwareVersion == 0x00 || i_LibraryVersion == 0x00)
        return _T("");

    CString strFirmwareVersion(""), strLibraryVersion("");

    UINT nDataBit = 8;
    long nDataParsingCount = 3; //이거 자동으로 계산하는거는 Test해보고 해보자 일단 고정값으로 고정

    for (long nindex = nDataParsingCount - 1; nindex >= 0; nindex--) //거꾸로 읽어야 하네 ^^
    {
        UINT nBitShift = nindex * nDataBit;

        if (((i_FirmwareVersion >> nBitShift) & 0xff) != 0x00) //00 이 아니기 때문에
        {
            strFirmwareVersion.AppendFormat(_T("%02x."), ((i_FirmwareVersion >> nBitShift) & 0xff));
        }
        else if (((i_FirmwareVersion >> nBitShift) & 0xff) == 0x00
            && nindex != nDataParsingCount - 1) //첫번쨰 자리가 00일수도 있으니 이는 무시한다
        {
            strFirmwareVersion.AppendFormat(_T("00."));
        }

        if (((i_LibraryVersion >> nBitShift) & 0xff) != 0x00) //00 이 아니기 때문에
        {
            strLibraryVersion.AppendFormat(_T("%02x."), ((i_LibraryVersion >> nBitShift) & 0xff));
        }
        else if (((i_LibraryVersion >> nBitShift) & 0xff) == 0x00
            && nindex != nDataParsingCount - 1) //첫번쨰 자리가 00일수도 있으니 이는 무시한다
        {
            strLibraryVersion.AppendFormat(_T("00."));
        }
    }

    //맨우측 끝자리에 "."를 삭제한다
    strFirmwareVersion.Delete(strFirmwareVersion.GetLength() - 1, 1);
    strLibraryVersion.Delete(strLibraryVersion.GetLength() - 1, 1);

    strFirmwareVersion.MakeUpper(); //소문자도 나오네 ^^...

    //F/W는 문자열내에 F가 없으면 맨앞에 F를 붙여준다
    if (strFirmwareVersion.Find(_T("F")) == -1)
        strFirmwareVersion.Insert(0, _T("F"));

    CString strReturnVal("");

    strReturnVal.AppendFormat(_T("%s,  %s"), (LPCTSTR)strFirmwareVersion, (LPCTSTR)strLibraryVersion);

    return strReturnVal;
}

int FrameGrabber_Base::get_grabber_num() const
{
    return (int)grabber_info.size();
}

int FrameGrabber_Base::get_grabber_id(const int& grabber_idx) const
{
    return grabber_info[grabber_idx].get_graaber_id();
}

int FrameGrabber_Base::get_camera_num(const int& graaber_id) const
{
    return grabber_info[graaber_id].get_connect_camera_num();
}

int FrameGrabber_Base::get_camera_id(const int& grabber_id, const int& camera_idx) const
{
    return grabber_info[grabber_id].get_connect_channel_id(camera_idx);
}

int FrameGrabber_Base::get_camera_icf_id(const int& graaber_id, const int& camera_id) const
{
    return grabber_info[graaber_id].get_connect_icf_id(camera_id);
}

bool FrameGrabber_Base::set_grab_image_width(const int& grabber_idx, const int& camera_idx, const unsigned int& width)
{
    if (is_valid_index(grabber_idx, camera_idx) == false)
    {
        popup_function_error(_T("set_grab_image_width"));
        return false;
    }

    const int& grabber_id = get_grabber_id(grabber_idx);
    const int& camera_id = get_camera_id(grabber_id, camera_idx);

    grab_image_widths[grabber_id][camera_id] = width;

    return true;
}

bool FrameGrabber_Base::set_grab_image_height(const int& grabber_idx, const int& camera_idx, const unsigned int& height)
{
    if (is_valid_index(grabber_idx, camera_idx) == false)
    {
        popup_function_error(_T("set_grab_image_height"));
        return false;
    }

    const int& grabber_id = get_grabber_id(grabber_idx);
    const int& camera_id = get_camera_id(grabber_id, camera_idx);

    grab_image_heights[grabber_id][camera_id] = height;

    return true;
}

bool FrameGrabber_Base::set_grab_image_offset_Y(const int& grabber_idx, const int& camera_idx, const long& offset)
{
    if (is_valid_index(grabber_idx, camera_idx) == false)
    {
        popup_function_error(_T("set_grab_image_offset_Y"));
        return false;
    }

    const int& grabber_id = get_grabber_id(grabber_idx);
    const int& camera_id = get_camera_id(grabber_id, camera_idx);

    grab_image_offsets_Y[grabber_id][camera_id] = offset;

    return true;
}

bool FrameGrabber_Base::apply_grab_image_offset_Y(
    const int& /*grabber_idx*/, const int& /*camera_idx*/, const long& /*offset*/)
{
    return true;
}

unsigned int FrameGrabber_Base::get_grab_image_width(const int& grabber_idx, const int& camera_idx)
{
    if (is_valid_index(grabber_idx, camera_idx) == false)
        return 0;

    const int& grabber_id = get_grabber_id(grabber_idx);
    const int& camera_id = get_camera_id(grabber_id, camera_idx);

    return grab_image_widths[grabber_id][camera_id];
}

unsigned int FrameGrabber_Base::get_grab_image_height(const int& grabber_idx, const int& camera_idx)
{
    if (is_valid_index(grabber_idx, camera_idx) == false)
        return 0;

    const int& grabber_id = get_grabber_id(grabber_idx);
    const int& camera_id = get_camera_id(grabber_id, camera_idx);

    return grab_image_heights[grabber_id][camera_id];
}

int FrameGrabber_Base::get_grab_image_offset_Y(const int& grabber_idx, const int& camera_idx)
{
    if (is_valid_index(grabber_idx, camera_idx) == false)
        return 0;

    const int& grabber_id = get_grabber_id(grabber_idx);
    const int& camera_id = get_camera_id(grabber_id, camera_idx);

    return grab_image_offsets_Y[grabber_id][camera_id];
}

void FrameGrabber_Base::set_library_version(const unsigned int& library_version_cur)
{
    library_version = library_version_cur;
}

unsigned int FrameGrabber_Base::get_use_library_version() const
{
    return library_version;
}

void FrameGrabber_Base::set_firmware_version(const int& grabber_id, const unsigned int& firmware_version)
{
    grabber_info[grabber_id].set_firmware_version(firmware_version);
}

unsigned int FrameGrabber_Base::get_use_firmware_version(const int& grabber_id) const
{
    return grabber_info[grabber_id].get_firmware_version();
}

bool FrameGrabber_Base::ExtractResourceFile(LPCTSTR type, LPCTSTR name, LPCTSTR targetPath)
{
    HINSTANCE hInst = ::AfxGetResourceHandle();

    HRSRC hResource = ::FindResource(hInst, name, type);

    if (hResource == NULL)
    {
        return false;
    }

    DWORD dwSize = ::SizeofResource(hInst, hResource);

    if (dwSize == 0)
    {
        return false;
    }

    HGLOBAL objResource = ::LoadResource(hInst, hResource);

    if (objResource == NULL)
    {
        return false;
    }

    wchar_t* memResource = (wchar_t*)::LockResource(objResource);

    CFile file;

    if (file.Open(targetPath, CFile::modeCreate | CFile::modeWrite))
    {
        file.Write(memResource, dwSize);
    }

    ::UnlockResource(objResource);

    ::FreeResource(objResource);

    return HANDLE(file) != INVALID_HANDLE_VALUE;
}

bool FrameGrabber_Base::SavesettingFile(const LPCTSTR& targetPath)
{
    CFile file;
    if (file.Open(targetPath, CFile::modeRead))
    {
        Ipvm::AsyncProgress progress(L"Connecting camera");

        file.Close();

        return true;
    }
    else
    {
        CString strError;
        strError.Format(_T("Cannot open file : \"%s\""), (LPCTSTR)targetPath);

        ::AfxMessageBox(strError, MB_ICONERROR);
        ::exit(0);
    }

    return false;
}

void FrameGrabber_Base::live_on(const HWND& hwndEventListener, const UINT& uiEventMessage, const long& livePeriod_ms,
    const enSideVisionModule i_eSideVisionModule, const int& grabMode)
{
    ASSERT(::IsWindow(hwndEventListener));
    ASSERT(uiEventMessage >= WM_USER);

    hwnd_event_listener = hwndEventListener;
    ui_event_message = uiEventMessage;
    live_period_ms = livePeriod_ms;
    m_eCurVisionModule = i_eSideVisionModule;
    m_grabMode = grabMode;

    //이거는 별도로 빼야할듯
    SystemConfig::GetInstance().GetColorGains(m_fNormal_Channel_GainRed, m_fNormal_Channel_GainGreen,
        m_fNormal_Channel_GainBlue, m_fReverse_Channel_GainRed, m_fReverse_Channel_GainGreen,
        m_fReverse_Channel_GainBlue);
    //

    ::SetEvent(live_flag);
}

void FrameGrabber_Base::live_off()
{
    ::ResetEvent(live_flag);

    hwnd_event_listener = NULL;
    ui_event_message = 0;
}

void FrameGrabber_Base::get_live_image(Ipvm::Image8u& image)
{
    image.Create(live_buffer_mono->GetSizeX(), live_buffer_mono->GetSizeY());

    Ipvm::ImageProcessing::Copy(*live_buffer_mono, Ipvm::Rect32s(*live_buffer_mono), image);
}

void FrameGrabber_Base::get_live_image(Ipvm::Image8u3& image)
{
    image.Create(live_buffer_color->GetSizeX(), live_buffer_color->GetSizeY());

    Ipvm::ImageProcessing::Copy(*live_buffer_color, Ipvm::Rect32s(*live_buffer_color), image);
}

void FrameGrabber_Base::get_live_image2(Ipvm::Image8u& image)
{
    image.Create(live_buffer_mono2->GetSizeX(), live_buffer_mono2->GetSizeY());

    Ipvm::ImageProcessing::Copy(*live_buffer_mono2, Ipvm::Rect32s(*live_buffer_mono2), image);
}

void FrameGrabber_Base::get_live_image2(Ipvm::Image8u3& image)
{
    image.Create(live_buffer_color2->GetSizeX(), live_buffer_color2->GetSizeY());

    Ipvm::ImageProcessing::Copy(*live_buffer_color2, Ipvm::Rect32s(*live_buffer_color2), image);
}

bool FrameGrabber_Base::IsLive() const
{
    return ::WaitForSingleObject(live_flag, 0) == WAIT_OBJECT_0;
}

bool FrameGrabber_Base::grab(const int& /*grabber_idx*/, const int& /*camera_idx*/, BYTE** /*image*/,
    const int& /*grab_num*/, Ipvm::AsyncProgress* /*progress*/)
{
    return true;
}

bool FrameGrabber_Base::grab(const int /*grabber_idx*/, const int /*camera_idx*/
    ,
    const int /*stitchIndex*/
    ,
    Ipvm::Image32r& /*zmapImage*/, Ipvm::Image16u& /*vmapImage*/
    ,
    float& /*height_range_min*/, float& /*height_range_max*/
    ,
    Ipvm::AsyncProgress* /*progress*/)
{
    return true;
}

bool FrameGrabber_Base::multigrab(
    const int /*stitchIndex*/
    ,
    VisionImageLot& imageLot
    ,
    float& /*height_range_min*/, float& /*height_range_max*/
    ,
    Ipvm::AsyncProgress* /*progress*/)
{
    return true;
}

bool FrameGrabber_Base::wait_grab_end(
    const int& /*grabber_idx*/, const int& /*camera_idx*/, Ipvm::AsyncProgress* /*progress*/)
{
    return true;
}

bool FrameGrabber_Base::wait_calc_end(const int& /*grabber_idx*/, const int& /*camera_idx*/,
    const bool& /*save_slit_images*/, Ipvm::AsyncProgress* /*progress*/)
{
    return true;
}

bool FrameGrabber_Base::re_calc_cuda_3D(const int& grabber_idx, const int& camera_idx) //kircheis_3DCalcRetry
{
    return true;
}

bool FrameGrabber_Base::set_slitbeam_parameters(const int& /*grabber_idx*/, const int& /*camera_idx*/,
    const SlitBeam3DParameters& /*slitbeamPara*/, const float& /*height_scaling*/,
    const bool& /*use_height_quantization*/, const bool& /*use_distortion_compensation*/)
{
    return true;
}

bool FrameGrabber_Base::set_exposure_time(const int& /*grabber_idx*/, const int& /*camera_idx*/,
    const double& /*exposureHigh_ms*/, double& /*validFramePeriod_ms*/)
{
    return true;
}

void FrameGrabber_Base::set_live_image_size(
    const int& /*grabber_idx*/, const int& /*camera_idx*/, const bool& /*use_binning*/, const bool& /*is_full_size*/)
{
    return;
}

long FrameGrabber_Base::get_grab_image_height_3D(
    const double& /*scan_depth_um*/, const bool& /*use_binning*/, const double& /*height_scaling*/) const
{
    return 0;
}

float FrameGrabber_Base::get_height_resolution_px2um(
    const bool& /*use_binning*/, const float& /*height_scaling*/, const bool& /*use_ideal_motor_speed*/) const
{
    return 0.f;
}

long FrameGrabber_Base::get_scan_length_px() const
{
    return 0;
}

long FrameGrabber_Base::get_scan_count() const
{
    return 0;
}

void FrameGrabber_Base::init_slit_beam_distortion()
{
    return;
}

bool FrameGrabber_Base::OpenCameraComm(const int& /*camera_id*/)
{
    return true;
}

bool FrameGrabber_Base::CloseCameraComm()
{
    return true;
}

bool FrameGrabber_Base::SetDigitalGainValue(float /*i_fInputGain*/, BOOL /*bInline*/) // Digital gain Contorl - JHB_NGRV
{
    return true;
}

float FrameGrabber_Base::GetCurrentCameraGain()
{
    return 0.f;
}

UINT FrameGrabber_Base::LiveThread(LPVOID pParam)
{
    return ((FrameGrabber_Base*)pParam)->LiveThread();
}

UINT FrameGrabber_Base::LiveThread()
{
    HANDLE handles[2] = {live_thread_kill, live_flag};

    while (1)
    {
        DWORD dwRet = ::WaitForMultipleObjects(sizeof(handles) / sizeof(HANDLE), handles, FALSE, INFINITE);

        if (dwRet != (WAIT_OBJECT_0 + 1))
        {
            return 0;
        }

        int idx_grabber = 0;
        int idx_camera = 0;
        bool success = false;
        if (SystemConfig::GetInstance().IsDual3DGrabMode())
        {
            if (m_grabMode == VISION_3D_GRABMODE_SPECULAR)
            {
                success = LiveCallback(idx_grabber, idx_camera);
            }
            else if (m_grabMode == VISION_3D_GRABMODE_DEFUSED)
            {
                idx_grabber = 1;
                idx_camera = 0;
                success = LiveCallback(idx_grabber, idx_camera);
            }
            else if (m_grabMode == VISION_3D_GRABMODE_INTEGRATED)
            {
                bool bCam1success = false;
                bool bCam2success = false;
          
                idx_grabber = 0;
                idx_camera = 0;
                bCam1success = LiveCallback(idx_grabber, idx_camera);
                
                idx_grabber = 1;
                idx_camera = 0;
                bCam2success = LiveCallback(idx_grabber, idx_camera);

                if (bCam1success == true && bCam2success == true)
                    success = true;
            }
        }
        else
        {
            success = LiveCallback(m_eCurVisionModule);
        }

        if (success == false)
        {
            live_buffer_mono->FillZero();
            live_buffer_color->FillZero();

            if (SystemConfig::GetInstance().IsDual3DGrabMode())
            {
                live_buffer_mono2->FillZero();
                live_buffer_color2->FillZero();
            }
        }

        if (SystemConfig::GetInstance().IsNGRVInspectionMode() == true && success == true)
        {
            Conversion_NGRV_InspectionMode_buf();
        }
        else if (SystemConfig::GetInstance().IsNGRVInspectionMode() == false && success == true)
        {
            Conversion_NGRV_Mode_buf(SystemConfig::GetInstance().m_bUseBayerPatternGPU);
        }

        //if (success != false && SystemConfig::GetInstance().m_bUseBayerPatternGPU) //	Bayer Pattern Work - 2021.01.28
        //{
        //    live_buffer_color->Create(live_buffer_mono->GetSizeX(), live_buffer_mono->GetSizeY());

        //    if (SystemConfig::GetInstance().IsReverseChannel()
        //        == TRUE) // JHB_NGRV - 2022.01.17 : Reverse Channel을 사용할 경우, 체크하여 Color Gain 값을 다르게 주어야 함
        //    {
        //        if (Ipvm::ImageProcessingGpu::DemosaicBilinear(*live_buffer_mono, Ipvm::Rect32s(*live_buffer_color),
        //                Ipvm::BayerPattern::e_gbrg, m_fReverse_Channel_GainRed, m_fReverse_Channel_GainGreen,
        //                m_fReverse_Channel_GainBlue, *live_buffer_color)
        //            != Ipvm::Status::e_ok)
        //        {
        //            live_buffer_color->FillZero();
        //        }
        //    }
        //    else
        //    {
        //        if (Ipvm::ImageProcessingGpu::DemosaicBilinear(*live_buffer_mono, Ipvm::Rect32s(*live_buffer_color),
        //                Ipvm::BayerPattern::e_gbrg, m_fNormal_Channel_GainRed, m_fNormal_Channel_GainGreen,
        //                m_fNormal_Channel_GainBlue, *live_buffer_color)
        //            != Ipvm::Status::e_ok)
        //        {
        //            live_buffer_color->FillZero();
        //        }
        //    }
        //}
        //else if (success != false && SystemConfig::GetInstance().m_bUseBayerPatternGPU == false)
        //{
        //    live_buffer_color->Create(live_buffer_mono->GetSizeX(), live_buffer_mono->GetSizeY());
        //}

        if (::IsWindow(hwnd_event_listener) && ui_event_message >= WM_USER)
        {
            ::SendMessage(hwnd_event_listener, ui_event_message, success, 0);
        }
        else
        {
            hwnd_event_listener = NULL;
            ui_event_message = 0;
        }

        Sleep(live_period_ms);
    }
}

void FrameGrabber_Base::Conversion_NGRV_InspectionMode_buf()
{
    Ipvm::Image8u dummy_red(live_buffer_mono->GetSizeX(), live_buffer_mono->GetSizeY());
    Ipvm::Image8u dummy_blue(live_buffer_mono->GetSizeX(), live_buffer_mono->GetSizeY());

    if (SystemConfig::GetInstance().IsReverseChannel() == TRUE)
    {
        if (Ipvm::ImageProcessingGpu::DemosaicBilinear(*live_buffer_mono, Ipvm::Rect32s(*live_buffer_mono),
                Ipvm::BayerPattern::e_gbrg, m_fReverse_Channel_GainRed, m_fReverse_Channel_GainGreen,
                m_fReverse_Channel_GainBlue, dummy_red, *live_buffer_mono, dummy_blue)
            != Ipvm::Status::e_ok)
        {
            live_buffer_mono->FillZero();
        }
    }
    else
    {
        if (Ipvm::ImageProcessingGpu::DemosaicBilinear(*live_buffer_mono, Ipvm::Rect32s(*live_buffer_mono),
                Ipvm::BayerPattern::e_gbrg, m_fNormal_Channel_GainRed, m_fNormal_Channel_GainGreen,
                m_fNormal_Channel_GainBlue, dummy_red, *live_buffer_mono, dummy_blue)
            != Ipvm::Status::e_ok)
        {
            live_buffer_mono->FillZero();
        }
    }
}

void FrameGrabber_Base::Conversion_NGRV_Mode_buf(const BOOL UseBayerPatternGPU)
{
    live_buffer_color->Create(live_buffer_mono->GetSizeX(), live_buffer_mono->GetSizeY());

    if (UseBayerPatternGPU == TRUE)
    {
        // JHB_NGRV - 2022.01.17 : Reverse Channel을 사용할 경우, 체크하여 Color Gain 값을 다르게 주어야 함
        if (SystemConfig::GetInstance().IsReverseChannel() == TRUE)
        {
            if (Ipvm::ImageProcessingGpu::DemosaicBilinear(*live_buffer_mono, Ipvm::Rect32s(*live_buffer_color),
                    Ipvm::BayerPattern::e_gbrg, m_fReverse_Channel_GainRed, m_fReverse_Channel_GainGreen,
                    m_fReverse_Channel_GainBlue, *live_buffer_color)
                != Ipvm::Status::e_ok)
            {
                live_buffer_color->FillZero();
            }
        }
        else
        {
            if (Ipvm::ImageProcessingGpu::DemosaicBilinear(*live_buffer_mono, Ipvm::Rect32s(*live_buffer_color),
                    Ipvm::BayerPattern::e_gbrg, m_fNormal_Channel_GainRed, m_fNormal_Channel_GainGreen,
                    m_fNormal_Channel_GainBlue, *live_buffer_color)
                != Ipvm::Status::e_ok)
            {
                live_buffer_color->FillZero();
            }
        }
    }
}

CString FrameGrabber_Base::Get3DGrabErrorMessage()
{
    CString StrNULL;
    StrNULL.Empty();
    return 0;
}

iGrab3D_Error_LOG FrameGrabber_Base::GetiGrab3D_Error_LOG()
{
    iGrab3D_Error_LOG siGrab3D_Error_LogDummy;

    return siGrab3D_Error_LogDummy;
}

void FrameGrabber_Base::SetiGrab3D_Error_LOG()
{
    return;
}

bool FrameGrabber_Base::Check3DNoise(const unsigned int& grabber_id, const unsigned int& camera_id, BYTE** ppbyImg,
    Ipvm::Image8u& o_image, bool bFullSizeInsp)
{
    UNREFERENCED_PARAMETER(grabber_id);
    UNREFERENCED_PARAMETER(camera_id);
    UNREFERENCED_PARAMETER(ppbyImg);
    UNREFERENCED_PARAMETER(o_image);
    UNREFERENCED_PARAMETER(bFullSizeInsp);

    return true;
}

bool FrameGrabber_Base::DoColorConversion(
    Ipvm::Image8u i_imageMono, Ipvm::Image8u3& o_imageColor, const bool i_bIsWhiteBalance, const bool i_bChannelReverse)
{
    if (i_bIsWhiteBalance)
    {
        if (Ipvm::ImageProcessingGpu::DemosaicBilinear(
                i_imageMono, Ipvm::Rect32s(i_imageMono), Ipvm::BayerPattern::e_gbrg, 1.f, 1.f, 1.f, o_imageColor)
            != Ipvm::Status::e_ok)
        {
            o_imageColor.FillZero();
            return FALSE;
        }
    }
    else
    {
        //{{ Frame 8 일 경우 RGB Gain을 다르게 가져가야함
        if (i_bChannelReverse == true)
        {
            if (Ipvm::ImageProcessingGpu::DemosaicBilinear(i_imageMono, Ipvm::Rect32s(i_imageMono),
                    Ipvm::BayerPattern::e_gbrg, m_fReverse_Channel_GainRed, m_fReverse_Channel_GainGreen,
                    m_fReverse_Channel_GainBlue, o_imageColor)
                != Ipvm::Status::e_ok)
            {
                o_imageColor.FillZero();
                return FALSE;
            }
        }
        else
        {
            if (Ipvm::ImageProcessingGpu::DemosaicBilinear(i_imageMono, Ipvm::Rect32s(i_imageMono),
                    Ipvm::BayerPattern::e_gbrg, m_fNormal_Channel_GainRed, m_fNormal_Channel_GainGreen,
                    m_fNormal_Channel_GainBlue, o_imageColor)
                != Ipvm::Status::e_ok)
            {
                o_imageColor.FillZero();
                return FALSE;
            }
        }
    }

    return TRUE;
}

bool FrameGrabber_Base::SendResizeImageToHost(
    Ipvm::Image8u i_Srcimage, Ipvm::Image8u& o_DstImage, long& o_nImageSizeX, long& o_nImageSizeY)
{
    long nResizeScale = 2;

    // Reszie image Size 계산
    o_nImageSizeX = ((i_Srcimage.GetSizeX() / nResizeScale + 3) / 4) * 4;
    o_nImageSizeY = i_Srcimage.GetSizeY() / nResizeScale;

    // image reszie시 for문 연산을 IPP 연산을 하도록 한다.
    o_DstImage.Create(o_nImageSizeX, o_nImageSizeY);
    Ipvm::ImageProcessing::ResizeLinearInterpolation(i_Srcimage, o_DstImage);

    return TRUE;
}

bool FrameGrabber_Base::SendResizeImageToHost(
    Ipvm::Image8u3 i_Srcimage, Ipvm::Image8u3& o_DstImage, long& o_nImageSizeX, long& o_nImageSizeY)
{
    long nResizeScale = 2;

    // Reszie image Size 계산
    o_nImageSizeX = ((i_Srcimage.GetSizeX() / nResizeScale + 3) / 4) * 4;
    o_nImageSizeY = i_Srcimage.GetSizeY() / nResizeScale;

    // image reszie시 for문 연산을 IPP 연산을 하도록 한다.
    o_DstImage.Create(o_nImageSizeX, o_nImageSizeY);
    Ipvm::ImageProcessing::ResizeLinearInterpolation(i_Srcimage, o_DstImage);

    return TRUE;
}

bool FrameGrabber_Base::DoSplitImageToRed(Ipvm::Image8u3 i_imageColor, Ipvm::Image8u& o_imageRed)
{
    Ipvm::Image8u imageGreen;
    Ipvm::Image8u imageBlue;

    imageGreen.Create(i_imageColor.GetSizeX(), i_imageColor.GetSizeY());
    imageBlue.Create(i_imageColor.GetSizeX(), i_imageColor.GetSizeY());

    if (Ipvm::ImageProcessing::SplitRGB(i_imageColor, Ipvm::Rect32s(i_imageColor), o_imageRed, imageGreen, imageBlue)
        != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    return TRUE;
}

bool FrameGrabber_Base::DoSplitImageToGreen(Ipvm::Image8u3 i_imageColor, Ipvm::Image8u& o_imageGreen)
{
    Ipvm::Image8u imageRed;
    Ipvm::Image8u imageBlue;

    imageRed.Create(i_imageColor.GetSizeX(), i_imageColor.GetSizeY());
    imageBlue.Create(i_imageColor.GetSizeX(), i_imageColor.GetSizeY());

    if (Ipvm::ImageProcessing::SplitRGB(i_imageColor, Ipvm::Rect32s(i_imageColor), imageRed, o_imageGreen, imageBlue)
        != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    return TRUE;
}

bool FrameGrabber_Base::DoSplitImageToBlue(Ipvm::Image8u3 i_imageColor, Ipvm::Image8u& o_imageBlue)
{
    Ipvm::Image8u imageRed;
    Ipvm::Image8u imageGreen;

    imageRed.Create(i_imageColor.GetSizeX(), i_imageColor.GetSizeY());
    imageGreen.Create(i_imageColor.GetSizeX(), i_imageColor.GetSizeY());

    if (Ipvm::ImageProcessing::SplitRGB(i_imageColor, Ipvm::Rect32s(i_imageColor), imageRed, imageGreen, o_imageBlue)
        != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    return TRUE;
}

void FrameGrabber_Base::apply_grabber_temperature(const int& grabber_id, const float& temperature)
{
    grabber_info[grabber_id].set_grabber_temperature(temperature);
}

void FrameGrabber_Base::set_grabber_temperature(const int& /*grabber_id*/)
{
    return;
}

float FrameGrabber_Base::get_grabber_temperature(const int& grabber_id) const
{
    return grabber_info[grabber_id].get_grabber_temperature();
}

void FrameGrabber_Base::init_grabber_info()
{
    return;
}

void FrameGrabber_Base::init_camera_info(const CString& /*camera_param_file_path*/)
{
    return;
}

bool FrameGrabber_Base::set_grabber_num(const long& graaber_num)
{
    if (graaber_num < 0)
        return false;

    grabber_info.resize(graaber_num);

    for (long graaber_idx = 0; graaber_idx < graaber_num; graaber_idx++)
    {
        grabber_info[graaber_idx].init();
        grabber_info[graaber_idx].set_grabber_id(graaber_idx);
    }

    return true;
}

void FrameGrabber_Base::set_camera_icf(const int& graaber_id, const int& camera_id, const int& icf_id)
{
    grabber_info[graaber_id].set_camera_icf(camera_id, icf_id);
}

bool FrameGrabber_Base::is_valid_index(const int& grabber_idx, const int& camera_idx)
{
    if (grabber_idx > get_grabber_num() || grabber_idx < 0)
    {
        CString error("");
        error.Format(_T("is_valid_index : Invalid grabber_idx input : %d, max : %d"), grabber_idx, get_grabber_num());
        popup_function_error(error);
        error.Empty();
        return false;
    }

    const int& grabber_id = get_grabber_id(grabber_idx);

    if (camera_idx > get_camera_num(grabber_id) || camera_idx < 0)
    {
        CString error("");
        error.Format(
            _T("is_valid_index : Invalid camera_idx input : %d, max : %d"), camera_idx, get_camera_num(grabber_id));
        popup_function_error(error);
        error.Empty();
        return false;
    }

    return true;
}

void FrameGrabber_Base::popup_function_error(const CString& name_function)
{
    CString error_msg = name_function;
    error_msg += _T(" Error");
    ::AfxMessageBox(error_msg);
    error_msg.Empty();
}