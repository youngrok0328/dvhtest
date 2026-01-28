//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Framegrabber2D_iGrabG.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedBuildConfiguration/iGrabG2/iManG.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IGG_LIB_VERSION \
    0x00020103 //mc_LibraryVersion이 변경될때마다 값을 변경하여 준다 [기존에 iGrabX에는 Define이 있었지만, 삭제되어 여기에서 관리]
#define IGG_FW_VERSION_G2C 0x00020209 //iGrabG2C
#define IGG_FW_VERSION_G2 0x00010103 //iGrabG2

//CPP_7_________________________________ Implementation body
//
Framegrabber2D_iGrabG::Framegrabber2D_iGrabG(void)
    : grab_buffers(nullptr)
    , grab_counts(0)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    const CString path_icf_file = get_camera_param_save_path();
    const LPCTSTR resource_name = get_camera_param_resourceID(SystemConfig::GetInstance().GetVisionType());

    FrameGrabber_Base::ExtractResourceFile(_T("ICF"), resource_name, path_icf_file);
    FrameGrabber_Base::SavesettingFile(path_icf_file);

    init_grabber_info();

    init_camera_info(path_icf_file);
}

Framegrabber2D_iGrabG::~Framegrabber2D_iGrabG()
{
    if (get_grabber_num() > 0)
    {
        for (long idx = 0; idx < get_grabber_num(); idx++)
            ::Ig_CloseSystem(get_grabber_id(idx));
    }
}

void Framegrabber2D_iGrabG::init_grabber_info()
{
    unsigned int library_version(0);
    ::Ig_GetLibraryVersion(library_version);

    if (library_version != IGG_LIB_VERSION)
    {
        //mc_다른곳에서 MSG를 띄울것이다
        SystemConfig::GetInstance().Set_is_iGrabFirmware_and_LibraryVersion_Mismatch(true);
    }

    set_library_version(library_version);

    if (set_grabber_num(::Ig_DeviceList()) == false)
    {
        ::AfxMessageBox(_T("Failed to open iGrab"), MB_ICONERROR | MB_OK);
        ::exit(0);
    }

    for (int grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
    {
        const int& id_grabber = get_grabber_id(grabber_idx);

        if (::Ig_OpenSystem(id_grabber) == FALSE)
        {
            ::AfxMessageBox(_T("Failed to open iGrab"), MB_ICONERROR | MB_OK);
            ::exit(0);
        }
        else
        {
            unsigned int firmware_version(0);
            ::Ig_GetFirmwareVersion(id_grabber, firmware_version);
            set_firmware_version(id_grabber, firmware_version);

            if (firmware_version < IGG_FW_VERSION_G2
                || (firmware_version != IGG_FW_VERSION_G2 && firmware_version < IGG_FW_VERSION_G2C))
            {
                //mc_다른곳에서 MSG를 띄울것이다
                SystemConfig::GetInstance().Set_is_iGrabFirmware_and_LibraryVersion_Mismatch(true);
            }
        }
    }
}

void Framegrabber2D_iGrabG::init_camera_info(const CString& camera_param_file_path)
{
    if (camera_param_file_path.IsEmpty() == true)
    {
        CString str("");
        str.Format(_T("Cannot open camera set param file."));
        ::AfxMessageBox(str, MB_ICONERROR | MB_OK);
        ::exit(0);
    }

    //camera & icf set
    int channel_num(1), id_icf(-1);
    for (int grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
    {
        //Channel ID set [iGrabG2C는 CL type이므로 chaanel이 무조건 1EA다 따라서 0번 chaanel을 사용]
        const int id_graaber = get_grabber_id(grabber_idx);
        for (int id_channel = 0; id_channel < channel_num; id_channel++)
        {
            if (::Ig_OpenICF(id_graaber, id_channel, id_icf, CStringA(camera_param_file_path).GetBuffer()) == FALSE)
            {
                CString str;
                str.Format(_T("Cannot open \"%s\" file."), LPCTSTR(camera_param_file_path));
                ::AfxMessageBox(str, MB_ICONERROR | MB_OK);
                ::exit(0);
            }
            else
            {
                set_camera_icf(id_graaber, id_channel, id_icf);
            }
        }
    }

    //size
    unsigned int width(0), height(0);
    for (long grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
    {
        const int id_grabber = get_grabber_id(grabber_idx);
        const int connect_camera_num = get_camera_num(id_grabber);

        for (long camera_idx = 0; camera_idx < connect_camera_num; camera_idx++)
        {
            const int id_channel = get_camera_id(id_grabber, camera_idx);
            const int id_icf = get_camera_icf_id(id_grabber, id_channel);

            ::Ig_GetICFParam(id_grabber, id_channel, id_icf, ICF_CTL_HORIZONTAL_SIZE, &width);
            ::Ig_GetICFParam(id_grabber, id_channel, id_icf, ICF_CTL_VERTICAL_SIZE, &height);

            set_grab_image_width(grabber_idx, camera_idx, width);
            set_grab_image_height(grabber_idx, camera_idx, height);
        }
    }

    live_buffer_mono->Create(width, height);
}

bool Framegrabber2D_iGrabG::grab(
    const int& grabber_idx, const int& camera_idx, BYTE** image, const int& grab_num, Ipvm::AsyncProgress* /*progress*/)
{
    grab_buffers = image;
    grab_counts = grab_num;

    if (is_valid_index(grabber_idx, camera_idx) == false)
        return false;

    const int& id_grabber = get_grabber_id(grabber_idx);
    const int& id_camera = get_camera_id(id_grabber, camera_idx);
    const int& id_icf = get_camera_icf_id(id_grabber, id_camera);

    if (id_icf > 0)
    {
        return IGSTATUS_GRAB_START_SUCCESS
            == ::Ig_MultiGrabProcess(id_grabber, id_camera, image, grab_num, grab_num, MTG_GRB_ISSUE);
    }

    return false;
}

bool Framegrabber2D_iGrabG::wait_grab_end(
    const int& grabber_idx, const int& camera_idx, Ipvm::AsyncProgress* /*progress*/)
{
    if (is_valid_index(grabber_idx, camera_idx) == false)
        return false;

    const int& id_grabber = get_grabber_id(grabber_idx);
    const int& id_camera = get_camera_id(id_grabber, camera_idx);
    const int& id_icf = get_camera_icf_id(id_grabber, id_camera);

    if (id_icf < 0)
    {
        return false;
    }

    DWORD dwRet = ::Ig_MultiGrabWait(id_grabber, id_camera);

    if (dwRet != WAIT_OBJECT_0)
    {
        for (long i = 0; i < grab_counts; i++)
        {
            // Grab Fail일 경우 이미지를 128로 채운다, 0으로 채울 경우 Grab Fail과 조명을 설정하지 않았을 때의 이미지를 구분할 수 없음 - 2023.07.12_JHB
            memset(grab_buffers[i], 128, live_buffer_mono->GetWidthBytes() * live_buffer_mono->GetSizeY());
        }

        return false;
    }

    return true;
}

bool Framegrabber2D_iGrabG::LiveCallback(const int& grabber_id, const int& camera_id)
{
    if (is_valid_index(grabber_id, camera_id) == false)
        return false;

    BYTE* pbyBuffer = live_buffer_mono->GetMem();

    return WAIT_OBJECT_0 == ::Ig_MultiGrabProcess(grabber_id, camera_id, &pbyBuffer, 1, 1);
}

bool Framegrabber2D_iGrabG::SetTriggerMode(int nMode)
{
    UNREFERENCED_PARAMETER(nMode);

    return true;
}

void Framegrabber2D_iGrabG::set_grabber_temperature(const int& grabber_id)
{
    double val(-1.);

    if (::Ig_GetTemperature(grabber_id, &val) == TRUE)
        apply_grabber_temperature(grabber_id, (float)val);
}

void Framegrabber2D_iGrabG::get_firmware_version(
    const int& grabber_id, unsigned int& cur_version, unsigned int& need_version)
{
    cur_version = get_use_firmware_version(grabber_id);

    if (get_use_firmware_version(grabber_id) < IGG_FW_VERSION_G2
        && get_use_firmware_version(grabber_id)
            < IGG_FW_VERSION_G2C) //필요로 하는 G2 F/W보다 낮고 && G2C F/W보다 낮으면 G2로 판단한다
        need_version = IGG_FW_VERSION_G2;
    else
        need_version = IGG_FW_VERSION_G2C; // 그외는 무조건 G2C를 사용할 것이기 때문에 상관없다
}

void Framegrabber2D_iGrabG::get_library_version(unsigned int& cur_version, unsigned int& need_version)
{
    cur_version = get_use_library_version();
    need_version = IGG_LIB_VERSION;
}