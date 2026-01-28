//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Framegrabber2D_iGrabXE.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedBuildConfiguration/iGrabXE/iFsCtrlCmd.h"
#include "../../SharedBuildConfiguration/iGrabXE/iManX.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IGG_FW_VERSION_XE 0x00010409 //iGrabXE

//CPP_7_________________________________ Implementation body
//
Framegrabber2D_iGrabXE::Framegrabber2D_iGrabXE(void)
    : grab_buffers(nullptr)
    , grab_counts(0)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	const CString path_xcf_file = get_camera_param_save_path();
    const LPCTSTR resource_name = get_camera_param_resourceID(SystemConfig::GetInstance().GetVisionType());

    FrameGrabber_Base::ExtractResourceFile(_T("XCF"), resource_name, path_xcf_file);
    FrameGrabber_Base::SavesettingFile(path_xcf_file);

    init_grabber_info();
    init_camera_info(path_xcf_file);
}

Framegrabber2D_iGrabXE::~Framegrabber2D_iGrabXE()
{
    if (get_grabber_num() > 0)
    {
        for (long idx = 0; idx < get_grabber_num(); idx++)
            ::Ix_CloseSystem(get_grabber_id(idx));
    }
}

void Framegrabber2D_iGrabXE::init_grabber_info()
{
    UINT libraryVersion = 0;
    ::Ix_GetLibraryVersion(libraryVersion);

    if (libraryVersion != IGG_FW_VERSION_XE)
    {
		SystemConfig::GetInstance().Set_is_iGrabFirmware_and_LibraryVersion_Mismatch(true);
    }

	set_library_version(libraryVersion);

	if (set_grabber_num(::Ix_DeviceList()) == false)
    {
        ::AfxMessageBox(_T("Failed to open iGrab"), MB_ICONERROR | MB_OK);
        ::exit(0);
    }

    for (int grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
    {
        const int& id_grabber = get_grabber_id(grabber_idx);

        if (::Ix_OpenSystem(id_grabber) == FALSE)
        {
            ::AfxMessageBox(_T("Failed to open iGrab"), MB_ICONERROR | MB_OK);
            ::exit(0);
        }
        else
        {
            unsigned int firmware_version(0);
            ::Ix_GetFirmwareVersion(id_grabber, firmware_version);
            set_firmware_version(id_grabber, firmware_version);

            if (firmware_version < IGG_FW_VERSION_XE
                || (firmware_version != IGG_FW_VERSION_XE && firmware_version < IGG_FW_VERSION_XE))
            {
                //mc_다른곳에서 MSG를 띄울것이다
                SystemConfig::GetInstance().Set_is_iGrabFirmware_and_LibraryVersion_Mismatch(true);
            }
        }

        BYTE bytePlug(0);
        if (!Ix_GetChannelPlugAssign(id_grabber, 0, bytePlug))
        {
            CString str;
            str.Format(_T("Ix_GetChannelPlugAssign return false"));
            ::AfxMessageBox(str, MB_ICONERROR | MB_OK);
            ::exit(0);
        }

        BYTE byteSetPlug(0);
        byteSetPlug = 0x0F; // 0000 1111b;
        if (bytePlug != byteSetPlug)
        {
            // Pollux 50M, Dalsa 67M 2D Vision 의 경우 4채널 사용
            // 0000 1111
            if (!Ix_SetChannelPlugAssign(id_grabber, 0, byteSetPlug))
            {
                CString str;
                str.Format(_T("Ix_SetChannelPlugAssign 실패\n"));
                ::AfxMessageBox(str, MB_ICONERROR | MB_OK);
                ::exit(0);
            }
        }

        if (!Ix_OpenChannel(id_grabber, 0))
        {
            CString str;
            str.Format(_T("Ix_OpenChannel 실패\n"));
            ::AfxMessageBox(str, MB_ICONERROR | MB_OK);
            ::exit(0);
        }
    }
}

void Framegrabber2D_iGrabXE::init_camera_info(const CString& camera_param_file_path)
{
    if (camera_param_file_path.IsEmpty() == true)
    {
        CString str("");
        str.Format(_T("Cannot open camera set param file."));
        ::AfxMessageBox(str, MB_ICONERROR | MB_OK);
        ::exit(0);
    }

    if (!IsIcfExist(camera_param_file_path))
    {
        CString str;
        str.Format(_T("Cannot find \"%s\" file."), LPCTSTR(camera_param_file_path));
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
            if (::Ix_OpenXCF(id_graaber, id_channel, id_icf, CStringA(camera_param_file_path).GetBuffer()) == FALSE)
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

            ::Ix_GetXCFParam(id_grabber, id_channel, id_icf, XCF_CTL_HORIZONTAL_SIZE, &width);
            ::Ix_GetXCFParam(id_grabber, id_channel, id_icf, XCF_CTL_VERTICAL_SIZE, &height);

            set_grab_image_width(grabber_idx, camera_idx, width);
            set_grab_image_height(grabber_idx, camera_idx, height);
        }
    }

    live_buffer_mono->Create(width, height);
}

BOOL Framegrabber2D_iGrabXE::IsIcfExist(LPCTSTR szPathName)
{
	CFile file;
	if (file.Open(szPathName, CFile::modeRead))
	{
		file.Close();

		return true;
	}

	return false;
}

bool Framegrabber2D_iGrabXE::grab(
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
        return STATUS_GRAB_START_SUCCESS
            == ::Ix_MultiGrabProcess(id_grabber, id_camera, image, grab_num, grab_num, MTG_GRB_ISSUE);
    }

    return false;
}

bool Framegrabber2D_iGrabXE::wait_grab_end(
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

    DWORD dwRet = ::Ix_MultiGrabWait(id_grabber, id_camera);

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

bool Framegrabber2D_iGrabXE::LiveCallback(const int& grabber_id, const int& camera_id)
{
    if (is_valid_index(grabber_id, camera_id) == false)
        return false;

    BYTE* pbyBuffer = live_buffer_mono->GetMem();

    return WAIT_OBJECT_0 == ::Ix_MultiGrabProcess(grabber_id, camera_id, &pbyBuffer, 1, 1);
}

bool Framegrabber2D_iGrabXE::SetTriggerMode(int nMode)
{
    UNREFERENCED_PARAMETER(nMode);

    return true;
}
