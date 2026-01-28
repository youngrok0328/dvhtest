//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Framegrabber2D_iGrabXQ.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedBuildConfiguration/iGrabXQ/iFsCtrlCmd.h"
#include "../../SharedBuildConfiguration/iGrabXQ/iManX.h"
#include "../../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h> //kircheis_SWIR
#include <Ipvm/AlgorithmGpu/ImageProcessingGpu.h> //kircheis_SWIR
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Widget/AsyncProgress.h>

//CPP_5_________________________________ Standard library headers
#include <vector>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define VIEWORKS_65MX_IMAGE_SIZE_X 9344
#define VIEWORKS_65MX_IMAGE_SIZE_Y 7000

#define BASLER_47M_IMAGE_SIZE_X 8192
#define BASLER_47M_IMAGE_SIZE_Y 2000
#define BASLER_47M_IMAGE_FULL_SIZE_Y 5468

#define IF_FW_VERSION_FIRMWARE_MIN 0x000105 // iGrabXQ Board는 버전이 항상 두 자리다.... - 2023.06.29_JHB
//#define IF_FW_VERSION_NEW_B_TYPE	0x000201

// 주소 값 읽어오는 방법 연구소 문의후 없앨 예정....
#define ADR_BASLER_47M_ACQ_START 0x100004a0
#define ADR_BASLER_47M_ACQ_STOP 0x100004ac
#define ADR_BASLER_47M_OFFSETY 0x1000329c
#define ADR_BASLER_47M_HEIGHT 0x1000327c

//CustomGrabEvent
#define MAX_CUSTOM_GRAB_EVENT 1

//CPP_7_________________________________ Implementation body
//
Framegrabber2D_iGrabXQ::Framegrabber2D_iGrabXQ(void)
    : m_ppbyGrabBuffers(nullptr)
    , m_ppbyColorGrabBuffers(nullptr)
    , m_sGrabCounts(0)
    , m_hActiveDev(INVALID_HANDLE_VALUE)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    camera_serials.clear();

    //camera type을 인자로 받아야함
    const CString path_xcf_file = get_camera_param_save_path();
    const LPCTSTR resource_name = get_camera_param_resourceID(SystemConfig::GetInstance().GetVisionType());

    FrameGrabber_Base::ExtractResourceFile(_T("XCF"), resource_name, path_xcf_file);
    FrameGrabber_Base::SavesettingFile(path_xcf_file);

    init_grabber_info();

    init_camera_info(path_xcf_file);

    init_camera_serial();

    init_camera_default_param(SystemConfig::GetInstance().GetVisionType());

    //static const bool isNGRV_Inspection
    //    = SystemConfig::GetInstance()
    //          .IsNGRVInspectionMode(); //kircheis_SWIR//Camera가 NGRV용 이고 VisionType이 2D Vision이면 NGRV Camera로 영상 찍는거다.

    //if (isNGRV_Inspection == true)
    //{
    //    m_imageDemosiacBufForSingle.Create(VIEWORKS_65MX_IMAGE_SIZE_X, VIEWORKS_65MX_IMAGE_SIZE_Y);
    //}

    //// iGrabXQ Board는 NGRV/Side 공유한다 - 2024.03.06_JHB
    //if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP || isNGRV_Inspection) //kircheis_SWIR
    //{
    //    CheckCameraInfoNGRV();
    //    SetVerticalSizeY(true, false, false);
    //}
    //else if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP)
    //{
    //    CheckCameraInfoSide();
    //    SetVerticalSizeY(false, 2000, true);
    //}

    //// CAM Serial Number
    //if (GetDeviceSerialNumber(m_cameraSerialNumber) == false)
    //{
    //    ::AfxMessageBox(_T("Cannot read camera serial number"), MB_ICONERROR | MB_OK);
    //    ::exit(0);
    //}

    // Camera Gain Test - JHB
    CalcTime(_T("Connect Start"));
    if (!GetCurrentGainValue(m_fCurrentCameraGain))
    {
        ::AfxMessageBox(_T("Cannot read camera gain value"), MB_ICONERROR | MB_OK);
        ::exit(0);
    }
    CalcTime(_T("Get Gain value Done"));

    //// boA8100-16cm Camera 사용 시 주소 값에 상관없이 Acquisition On/Off 함수 사용
    //// 모든 Camera 관련 Parameter를 설정 후에 Acquisition Start를 줘야한다... 그 전까지는 무조건 Acqusition Stop으로 사용...
    //if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP)
    //{
    //    SetSideCameraAcquisition(TRUE);
    //}
}

Framegrabber2D_iGrabXQ::~Framegrabber2D_iGrabXQ(void)
{
    if (get_grabber_num() > 0)
    {
        for (int grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
        {
            const int& id_grabber = get_grabber_id(grabber_idx);
            for (int camera_idx = 0; camera_idx < get_camera_num(grabber_idx); camera_idx++)
            {
                const int& id_camera = get_camera_id(id_grabber, camera_idx);

                if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP)
                {
                    HANDLE handle = ::ctrlOpen(id_camera);
                    BOOL bStartVal = ctrlSetAcqStop(handle);

                    UNREFERENCED_PARAMETER(bStartVal);
                }

                ::Ix_CloseChannel(id_grabber, id_camera);
            }

            ::Ix_CloseSystem(id_grabber);
        }
    }
}

void Framegrabber2D_iGrabXQ::init_grabber_info()
{
    unsigned int libraryVersion(0);
    ::Ix_GetLibraryVersion(libraryVersion);

    if (libraryVersion != IF_LIB_VERSION)
    {
        //mc_다른곳에서 MSG를 띄울것이다
        SystemConfig::GetInstance().Set_is_iGrabFirmware_and_LibraryVersion_Mismatch(true);
    }

    set_library_version(libraryVersion);

    if (set_grabber_num(::Ix_DeviceList()) == false)
    {
        ::AfxMessageBox(_T("Failed to open iGrabXQ"), MB_ICONERROR | MB_OK);
        ::exit(0);
    }

    //연결된 grabber ID를 set
    for (int grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
    {
        const int& id_grabber = get_grabber_id(grabber_idx);
        if (::Ix_OpenSystem(grabber_idx) == FALSE)
        {
            CString strID;
            strID.Format(_T("Failed to open framegrabber : Sys = %d"), grabber_idx);
            ::AfxMessageBox(strID);
            strID.Empty();
            ::exit(0);
        }
        else
        {
            unsigned int firmware_version(0);
            ::Ix_GetFirmwareVersion(id_grabber, firmware_version);
            set_firmware_version(id_grabber, firmware_version);

            if (firmware_version < IF_FW_VERSION_FIRMWARE_MIN)
            {
                //mc_다른곳에서 MSG를 띄울것이다
                SystemConfig::GetInstance().Set_is_iGrabFirmware_and_LibraryVersion_Mismatch(true);
            }
        }
    }
}

void Framegrabber2D_iGrabXQ::init_camera_info(const CString& camera_param_file_path)
{
    if (camera_param_file_path.IsEmpty() == true)
    {
        CString str("");
        str.Format(_T("Cannot open camera set param file."));
        ::AfxMessageBox(str, MB_ICONERROR | MB_OK);
        ::exit(0);
    }

    long open_channel_num(0); //Open된 channel이 하나도 없으면 잘못된 상황이다 바로 종료한다
    for (int grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
    {
        const int& id_graaber = get_grabber_id(grabber_idx);
        for (int camera_idx = 0; camera_idx < MAX_CAMERA_NUMBER; camera_idx++)
        {
            //const int& id_camera = get_camera_id(id_graaber, camera_idx);
            if (check_is_valid_camera_channel(id_graaber, camera_idx) == false)
            {
                //사용하지 않는 channel이니 닫는다
                if (::Ix_CloseChannel(id_graaber, camera_idx) == FALSE)
                {
                    ::AfxMessageBox(_T("Failed to close camera handle"));
                    exit(0);
                }

                continue;
            }

            int id_icf(-1);

            // 통신 시도하기 전에 Freerun 모드로 가 있게 한다.
            ::Ix_OpenXCF(id_graaber, camera_idx, id_icf, CStringA(camera_param_file_path).GetBuffer());
            ::Ix_SetXCFParam(id_graaber, camera_idx, id_icf, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);

            //Camera & icf set
            set_camera_icf(id_graaber, camera_idx, id_icf);

            open_channel_num++;
        }
    }

    if (open_channel_num == 0) //한개도 Open하지 못한상황이다
    {
        CString error_msg(_T("Failed to open framegrabber : all Channel"));
        ::AfxMessageBox(error_msg);
        error_msg.Empty();
        ::exit(0);
    }
}

bool Framegrabber2D_iGrabXQ::check_is_valid_camera_channel(const int& id_graaber, const int& id_channel)
{
    if (::Ix_OpenChannel(id_graaber, id_channel) == FALSE)
    {
        CString strChannel("");
        strChannel.Format(_T("Fail to open Channel"));
        ::AfxMessageBox(strChannel);
        strChannel.Empty();
        //::exit(0);

        return false;
    }
    else
    {
        BYTE plug_channel(0x00); //plug값 받아서 쓰자 그냥
        if (::Ix_GetChannelPlugAssign(id_graaber, id_channel, plug_channel) == TRUE && plug_channel != 0x00)
        {
            return true;
        }
        else //할당이 안된상황
            return false;
    }

    return true;
}

bool Framegrabber2D_iGrabXQ::init_camera_serial()
{
    for (int grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
    {
        const int& id_grabber = get_grabber_id(grabber_idx);
        for (int camera_idx = 0; camera_idx < get_camera_num(id_grabber); camera_idx++)
        {
            const int& id_camera = get_camera_id(id_grabber, camera_idx);

            CString device_serial("");
            if (GetDeviceSerialNumber(id_camera, device_serial) == false)
            {
                ::AfxMessageBox(_T("Cannot read camera serial number"), MB_ICONERROR | MB_OK);
                ::exit(0);
            }

            camera_serials[id_grabber][id_camera] = device_serial;
        }
    }

    return true;
}

bool Framegrabber2D_iGrabXQ::init_camera_default_param(const int& camera_type)
{
    for (int grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
    {
        const int& id_grabber = get_grabber_id(grabber_idx);
        for (long camera_idx = 0; camera_idx < get_camera_num(id_grabber); camera_idx++)
        {
            const int& id_camera = get_camera_id(id_grabber, camera_idx);

            BYTE plug(0x00);
            switch (camera_type)
            {
                case VISIONTYPE_NGRV_INSP: //NGRV
                {
                    plug = 0x0f;
                }
                break;
                case VISIONTYPE_SIDE_INSP: //Side
                {
                    plug = id_camera == 0 ? 0x01 : 0x02; //임시방편
                }
                break;
                case VISIONTYPE_SWIR_INSP:
                {
                    plug = 0x01;
                }
                break;
                case VISIONTYPE_2D_INSP:
                {
                    plug = 0x0f;
                }
                break;
                default:
                    break;
            }

            if (::Ix_SetChannelPlugAssign(id_grabber, id_camera, plug) == FALSE)
            {
                CString strChannelAssign("");
                strChannelAssign.Format(_T("Fail to get Channel Assignment Info."));
                ::AfxMessageBox(strChannelAssign);
                strChannelAssign.Empty();
                ::exit(0);
            }

            //Parameter 변경전에 acq. stop 해준다.[추가해야함]

            const int& id_icf = get_camera_icf_id(id_grabber, id_camera);

            switch (camera_type)
            {
                case VISIONTYPE_NGRV_INSP:
                {
                    if (!SetCameraAcqusitionStart(id_camera))
                    {
                        ::AfxMessageBox(_T("Acquisition Start Error"));
                        ::exit(0);
                    }

                    ::Ix_SetCustomGrabEvent(id_grabber, id_camera, MAX_CUSTOM_GRAB_EVENT);

                    SetVerticalSizeY(id_grabber, id_camera, id_icf, true, false, false);
                }
                break;
                case VISIONTYPE_SIDE_INSP:
                {
                    SetSideCameraAcquisition(id_camera, FALSE);

                    SetVerticalSizeY(id_grabber, id_camera, id_icf, false, 2000, true);
                }
                break;
                case VISIONTYPE_SWIR_INSP:
                {
                    Ipvm::AsyncProgress progress(_T("Modify camera parameters"));

                    unsigned int width(0), height(0);
                    ::Ix_GetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_HORIZONTAL_SIZE, &width);
                    ::Ix_GetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_VERTICAL_SIZE, &height);

                    ::Ix_SetXCFParam(
                        id_grabber, id_camera, id_icf, XCF_CTL_TRIGGER_MODE, _enumTriggermode::UplinkTrigger);

                    set_grab_image_width(grabber_idx, camera_idx, width);
                    set_grab_image_height(grabber_idx, camera_idx, height);

                    live_buffer_mono->Create(
                        get_grab_image_width(grabber_idx, camera_idx), get_grab_image_height(grabber_idx, camera_idx));
                }
                break;
                case VISIONTYPE_2D_INSP:
                {
                    Ipvm::AsyncProgress progress(_T("Modify camera parameters"));

                    unsigned int width(0), height(0);
                    ::Ix_GetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_HORIZONTAL_SIZE, &width);
                    ::Ix_GetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_VERTICAL_SIZE, &height);

                    ::Ix_SetXCFParam(
                        id_grabber, id_camera, id_icf, XCF_CTL_TRIGGER_MODE, _enumTriggermode::UplinkTrigger);

                    set_grab_image_width(grabber_idx, camera_idx, width);
                    set_grab_image_height(grabber_idx, camera_idx, height);

                    live_buffer_mono->Create(
                        get_grab_image_width(grabber_idx, camera_idx), get_grab_image_height(grabber_idx, camera_idx));
                }
                break;
                default:
                    break;
            }
        }
    }

    return true;
}

void Framegrabber2D_iGrabXQ::CheckCameraInfoNGRV()
{
    //   /*{{ 순서도
    //1. Grab Board의 Library Version 체크
    //2. 카메라에 할당 되어있는 SYSTEM ID 체크 및 해당 ID로 SYSTEM Open
    //3. 사용할 Channel을 할당(X 및 XQ Board 일 때는 Channel을 할당 해주어야 함)
    //4. Channel Open
    //5. XCF 파일 Open
    //6. Firmware Version 체크
    //}}*/

    //   //{{ Library Version Check
    //   UINT libraryVersion = 0;
    //   ::Ix_GetLibraryVersion(libraryVersion);

    //   if (libraryVersion != IF_LIB_VERSION)
    //   {
    //       //CString strNotice;
    //       //strNotice.Format(_T("iGrabXQ library version mismatch!!\r\n\r\nExpected version : %X\r\nLoaded version : %X\r\n\r\nProgram terminated!!!"), IF_LIB_VERSION, libraryVersion);
    //       //::AfxMessageBox(strNotice, MB_OK | MB_ICONERROR);
    //       //::exit(0);

    //       //mc_다른곳에서 MSG를 띄울것이다
    //       SystemConfig::GetInstance().Set_is_iGrabFirmware_and_LibraryVersion_Mismatch(true);
    //   }

    //   m_libraryVersion = libraryVersion;
    //   //m_libraryVersion.Format(_T("%X"), libraryVersion);
    //   //}}

    //   //	먼저 Open되어있는 SYS_ID가 존재하면 삭제
    //   if (m_iSYS_ID >= 0)
    //   {
    //       ::Ix_CloseSystem(m_iSYS_ID);
    //       m_iSYS_ID = -1;
    //   }

    //   int nSysID = 0;

    //   if (!::Ix_OpenSystem(nSysID))
    //   {
    //       CString strID;
    //       strID.Format(_T("Fail to open Framegrabber : Sys = %d"), nSysID);
    //       ::AfxMessageBox(strID);
    //       ::exit(0);
    //   }

    //   m_iSYS_ID = nSysID;

    //   if (!::Ix_SetChannelPlugAssign(m_iSYS_ID, 0, 0x0f))
    //   {
    //       CString strChannelAssign;
    //       strChannelAssign.Format(_T("Fail to get Channel Assignment Info."));
    //       ::AfxMessageBox(strChannelAssign);
    //       ::exit(0);
    //   }

    //   if (!::Ix_OpenChannel(m_iSYS_ID, 0))
    //   {
    //       CString strChannel;
    //       strChannel.Format(_T("Fail to open Channel"));
    //       ::AfxMessageBox(strChannel);
    //       ::exit(0);
    //   }

    //   // XCF 파일 열기
    //   const CString strXCFPathName = DynamicSystemPath::get(DefineFolder::Temp) + _T("vc-65mx-0.2.1.xcf");

    //   FrameGrabber_Base::ExtractResourceFile(_T("XCF"), MAKEINTRESOURCE(IDR_XCF_VC_65MX_0_2_1), strXCFPathName);

    //   if (!IsXcfExist(strXCFPathName))
    //   {
    //       CString str;
    //       str.Format(_T("Cannot find \"%s\" file."), LPCTSTR(strXCFPathName));
    //       ::AfxMessageBox(str, MB_ICONERROR | MB_OK);
    //       ::exit(0);
    //   }

    //   if (::Ix_OpenXCF(m_iSYS_ID, 0, m_iICF_ID, CStringA(strXCFPathName).GetBuffer()) == FALSE)
    //   {
    //       CString str;
    //       str.Format(_T("Cannot open \"%s\" file."), LPCTSTR(strXCFPathName));
    //       ::AfxMessageBox(str, MB_ICONERROR | MB_OK);
    //       ::exit(0);
    //   }

    //   // 통신 시도하기 전에 Freerun 모드로 설정
    //   ::Ix_SetXCFParam(m_iSYS_ID, 0, m_iICF_ID, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);

    //   //{{ Firmware Version Check
    //   UINT firmwareVersion = 0;
    //   ::Ix_GetFirmwareVersion(m_iSYS_ID, firmwareVersion);

    //   m_firmwareVersion = firmwareVersion;

    //   //m_firmwareVersion.Format(_T("%X"), firmwareVersion);

    //   if (firmwareVersion < IF_FW_VERSION_FIRMWARE_MIN)
    //   {
    //       SystemConfig::GetInstance().Set_is_iGrabFirmware_and_LibraryVersion_Mismatch(true);
    //   }

    //   //}}

    //   //{{ XML 파일 열기
    //   const CString strXMLPathName = DynamicSystemPath::get(DefineFolder::Temp) + _T("vc_65mx-0.2.1.xml");

    //   FrameGrabber_Base::ExtractResourceFile(_T("XML"), MAKEINTRESOURCE(IDR_XML_VC_65MX_0_2_1), strXMLPathName);
    //   //}}

    //   CFile fileXML;
    //   if (fileXML.Open(strXMLPathName, CFile::modeRead))
    //   {
    //       Ipvm::AsyncProgress progress(L"Connecting camera");

    //       fileXML.Close();
    //   }
    //   else
    //   {
    //       CString strError;
    //       strError.Format(_T("Cannot open XML file : \"%s"), (LPCTSTR)strXMLPathName);

    //       ::AfxMessageBox(strError, MB_ICONERROR);
    //       ::exit(0);
    //   }

    //   if (!SetCameraAcqusitionStart())
    //   {
    //       ::AfxMessageBox(_T("Acquisition Start Error"));
    //       ::exit(0);
    //   }

    //   ::Ix_SetCustomGrabEvent(m_iSYS_ID, 0, MAX_CUSTOM_GRAB_EVENT);

    //   static const bool isNGRV_Inspection = SystemConfig::GetInstance().IsNGRVInspectionMode(); //kircheis_SWIR

    //   if (isNGRV_Inspection) //kircheis_SWIR
    //   {
    //       // NGRV 검사 모드는 Trigger Mode를 UplinkTrigger로 미리 설정한다.
    //       ::Ix_SetXCFParam(m_iSYS_ID, 0, m_iICF_ID, XCF_CTL_TRIGGER_MODE, _enumTriggermode::UplinkTrigger);
    //   }
}

void Framegrabber2D_iGrabXQ::CheckCameraInfoSide()
{
    //   /*{{ 순서도
    //1. Grab Board의 Library Version 체크
    //2. 카메라에 할당 되어있는 SYSTEM ID 체크 및 해당 ID로 SYSTEM Open
    //3. 사용할 Channel을 할당(X 및 XQ Board 일 때는 Channel을 할당 해주어야 함)
    //4. Channel Open
    //5. XCF 파일 Open
    //6. Trigger Mode/Type 설정
    //7. Firmware Version 체크
    //}}*/

    //   //Cam Channel, iCF ID Init.
    //   for (long nidx = 0; nidx < SIDE_CAMERA_NUM; nidx++)
    //   {
    //       m_iChannel_ID_Side[nidx] = -1;
    //       m_iICF_ID_Side[nidx] = -1;
    //   }
    //   //{{ Library Version Check
    //   UINT libraryVersion = 0;
    //   ::Ix_GetLibraryVersion(libraryVersion);

    //   if (libraryVersion != IF_LIB_VERSION)
    //   {
    //       //CString strNotice;
    //       //strNotice.Format(_T("iGrabXQ library version mismatch!!\r\n\r\nExpected version : %X\r\nLoaded version : %X\r\n\r\nProgram terminated!!!"), IF_LIB_VERSION, libraryVersion);
    //       //::AfxMessageBox(strNotice, MB_OK | MB_ICONERROR);
    //       //::exit(0);

    //       //mc_다른곳에서 MSG를 띄울것이다
    //       SystemConfig::GetInstance().Set_is_iGrabFirmware_and_LibraryVersion_Mismatch(true);
    //   }

    //   m_libraryVersion = libraryVersion;
    //   //m_libraryVersion.Format(_T("%X"), libraryVersion);
    //   //}}

    //   //	먼저 Open되어있는 SYS_ID가 존재하면 삭제
    //   if (m_iSYS_ID >= 0)
    //   {
    //       ::Ix_CloseSystem(m_iSYS_ID);
    //       m_iSYS_ID = -1;
    //   }

    //   int nSysID = 0;

    //   if (!::Ix_OpenSystem(nSysID))
    //   {
    //       CString strID;
    //       strID.Format(_T("Fail to open Framegrabber : Sys = %d"), nSysID);
    //       ::AfxMessageBox(strID);
    //       ::exit(0);
    //   }

    //   m_iSYS_ID = nSysID;

    //   // XCF 파일 열기
    //   const CString strXCFPathName = DynamicSystemPath::get(DefineFolder::Temp) + _T("boA8100_16cm.xcf");

    //   FrameGrabber_Base::ExtractResourceFile(_T("XCF"), MAKEINTRESOURCE(IDR_XCF_BOA8100_16CM), strXCFPathName);

    //   if (!IsXcfExist(strXCFPathName))
    //   {
    //       CString str;
    //       str.Format(_T("Cannot find \"%s\" file."), LPCTSTR(strXCFPathName));
    //       ::AfxMessageBox(str, MB_ICONERROR | MB_OK);
    //       ::exit(0);
    //   }

    //   BYTE byChannelPlug(0x01);
    //   for (long nCamidx = 0; nCamidx < SIDE_CAMERA_NUM; nCamidx++)
    //   {
    //       if (!::Ix_SetChannelPlugAssign(m_iSYS_ID, nCamidx, byChannelPlug))
    //       {
    //           CString strChannelAssign;
    //           strChannelAssign.Format(_T("Fail to get Channel : %d Assignment Info."), nCamidx);
    //           ::AfxMessageBox(strChannelAssign);
    //           ::exit(0);
    //       }

    //       if (!::Ix_OpenChannel(m_iSYS_ID, nCamidx))
    //       {
    //           CString strChannel;
    //           strChannel.Format(_T("Fail to open Channel : %d"), nCamidx);
    //           ::AfxMessageBox(strChannel);
    //           ::exit(0);
    //       }

    //       m_iChannel_ID_Side[nCamidx] = nCamidx;

    //       byChannelPlug++;

    //       if (::Ix_OpenXCF(
    //               m_iSYS_ID, m_iChannel_ID_Side[nCamidx], m_iICF_ID_Side[nCamidx], CStringA(strXCFPathName).GetBuffer())
    //           == FALSE)
    //       {
    //           CString str;
    //           str.Format(_T("Cannot open \"%s\" file."), LPCTSTR(strXCFPathName));
    //           ::AfxMessageBox(str, MB_ICONERROR | MB_OK);
    //           ::exit(0);
    //       }

    //       ::Ix_SetXCFParam(m_iSYS_ID, m_iChannel_ID_Side[nCamidx], m_iICF_ID_Side[nCamidx], XCF_CTL_TRIGGER_MODE,
    //           _enumTriggermode::UplinkTrigger);
    //   }

    //   //{{ Firmware Version Check
    //   UINT firmwareVersion = 0;
    //   ::Ix_GetFirmwareVersion(m_iSYS_ID, firmwareVersion);

    //   m_firmwareVersion = firmwareVersion;

    //   if (firmwareVersion < IF_FW_VERSION_FIRMWARE_MIN)
    //   {
    //       SystemConfig::GetInstance().Set_is_iGrabFirmware_and_LibraryVersion_Mismatch(true);
    //   }

    //   //}}

    //   //{{ XML 파일 열기
    //   const CString strXMLPathName = DynamicSystemPath::get(DefineFolder::Temp) + _T("boA8100_16cm.xml");

    //   FrameGrabber_Base::ExtractResourceFile(_T("XML"), MAKEINTRESOURCE(IDR_XML_BOA8100_16CM), strXMLPathName);
    //   //}}

    //   CFile fileXML;
    //   if (fileXML.Open(strXMLPathName, CFile::modeRead))
    //   {
    //       Ipvm::AsyncProgress progress(L"Connecting camera");

    //       fileXML.Close();
    //   }
    //   else
    //   {
    //       CString strError;
    //       strError.Format(_T("Cannot open XML file : \"%s"), (LPCTSTR)strXMLPathName);

    //       ::AfxMessageBox(strError, MB_ICONERROR);
    //       ::exit(0);
    //   }

    //   SetSideCameraAcquisition(FALSE);
}

bool Framegrabber2D_iGrabXQ::GetDeviceSerialNumber(const int& camera_id, CString& serialNumber)
{
    Ipvm::AsyncProgress progress(_T("Get camera serial number"));

    // Camera Open...
    HANDLE handle = ::ctrlOpen(camera_id);

    // Register address
    ULONG ulDeviceSerialNumberAddress = 0x20B0;
    //ULONG ulUserBaseAddress = 0x10000000;
    //ULONG ulTrigBaseAddress = ulUserBaseAddress + 0x00020000;
    //ULONG ulAddressAcqStart = ulTrigBaseAddress + 0x00001084;
    //ULONG ulAddressAcqStop = ulTrigBaseAddress + 0x000010C4;

    // Communication Status Check
    char deviceSerialNumberBuffer[40];
    memset(deviceSerialNumberBuffer, 0, sizeof(deviceSerialNumberBuffer));

    BYTE ret1 = ::ctrlCmdMsg(handle, enumMemoryRead, 16, ulDeviceSerialNumberAddress, nullptr, 1000);

    BYTE byAckCode = 0;
    ULONG ulSize = 0;
    BYTE ret2 = ::ctrlAckMsg(handle, byAckCode, ulSize, (BYTE*)deviceSerialNumberBuffer);

    ULONG* replyMem = (ULONG*)deviceSerialNumberBuffer;

    ULONG memSize = ulSize / 4;
    for (ULONG i = 0; i < memSize; i++)
        replyMem[i] = XQSWAP(replyMem[i]);

    serialNumber = deviceSerialNumberBuffer;

    // Close Device...
    BYTE ret3 = static_cast<BYTE>(::ctrlClose(handle));

    UNREFERENCED_PARAMETER(ret1); //ret 계열 변수는 디버깅을 위한 변수라 없애지 않고 남겨둔다.
    UNREFERENCED_PARAMETER(ret2);
    UNREFERENCED_PARAMETER(ret3);

    return true;
}

bool Framegrabber2D_iGrabXQ::grab(
    const int& grabber_idx, const int& camera_idx, BYTE** image, const int& grab_num, Ipvm::AsyncProgress* progress)
{
    if (is_valid_index(grabber_idx, camera_idx) == false)
        return false;

    const int& id_grabber = get_grabber_id(grabber_idx);
    const int& id_camera = get_camera_id(id_grabber, camera_idx);
    const int& icf_id = get_camera_icf_id(id_grabber, id_camera);

    if (progress)
    {
        progress->Post(L"Grab Start");
    }

    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP)
    {
        ::Ix_SetXCFParam(id_grabber, id_camera, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::UplinkTrigger);
    }

    m_ppbyGrabBuffers = image;
    m_sGrabCounts = grab_num;

    DWORD dwRet = ::Ix_MultiGrabProcess(
        id_grabber, id_camera, m_ppbyGrabBuffers, m_sGrabCounts, m_sGrabCounts, MTG_GRB_ISSUE, 1500);

    return STATUS_GRAB_START_SUCCESS == dwRet;
}

bool Framegrabber2D_iGrabXQ::wait_grab_end(const int& grabber_idx, const int& camera_idx, Ipvm::AsyncProgress* progress)
{
    if (is_valid_index(grabber_idx, camera_idx) == false)
        return false;

    const int& id_grabber = get_grabber_id(grabber_idx);
    const int& id_camera = get_camera_id(id_grabber, camera_idx);
    const int& icf_id = get_camera_icf_id(id_grabber, id_camera);

    if (progress)
    {
        progress->Post(_T("wait for Grab end"));
    }

    DWORD dwRet = ::Ix_MultiGrabWait(id_grabber, id_camera);

    //영상 획득이 끝나면  Freerun으로 변경하여  불필요한  트리거 발생을 방지하자.
    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP)
    {
        ::Ix_SetXCFParam(id_grabber, id_camera, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);
    }

    if (dwRet != WAIT_OBJECT_0)
    {
        for (long i = 0; i < m_sGrabCounts; i++)
        {
            memset(m_ppbyGrabBuffers[i], 0, live_buffer_mono->GetWidthBytes() * live_buffer_mono->GetSizeY());
        }

        return false;
    }

    return true;
}

//bool Framegrabber2D_iGrabXQ::WaitGrabEnd(Ipvm::AsyncProgress* progress)
//{
//    static const bool isNGRV_Inspection = SystemConfig::GetInstance().IsNGRVInspectionMode(); //kircheis_SWIR
//
//    if (progress)
//    {
//        progress->Post(_T("wait for Grab end"));
//    }
//
//    if (m_iICF_ID < 0)
//    {
//        return false;
//    }
//
//    DWORD dwRet = ::Ix_MultiGrabWait(m_iSYS_ID, 0);
//
//    //영상 획득이 끝나면  Freerun으로 변경하여  불필요한  트리거 발생을 방지하자.
//    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP)
//    {
//        ::Ix_SetXCFParam(m_iSYS_ID, 0, m_iICF_ID, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);
//    }
//    else if (isNGRV_Inspection == true && dwRet == WAIT_OBJECT_0) //kircheis_SWIR
//    {
//        Ipvm::Image8u imageGrabbed;
//        imageGrabbed.Create(m_liveBuffer->GetSizeX(), m_liveBuffer->GetSizeY());
//        BYTE* pImageGrabbed = (BYTE*)imageGrabbed.GetMem();
//
//        long imageSize = m_liveBuffer->GetWidthBytes() * m_liveBuffer->GetSizeY();
//        for (long idx = 0; idx < m_sGrabCounts; idx++)
//        {
//            memcpy(pImageGrabbed, m_ppbyGrabBuffers[idx], imageSize);
//            // Grab된 이미지를 Green Channel Demosaic 처리한다. (imageGrabbed -> m_imageDemosiacBufForSingle -> m_ppbyGrabBuffers)
//
//            memcpy(m_ppbyGrabBuffers[idx], pImageGrabbed, imageSize);
//        }
//    }
//
//    if (dwRet != WAIT_OBJECT_0)
//    {
//        for (long i = 0; i < m_sGrabCounts; i++)
//        {
//            memset(m_ppbyGrabBuffers[i], 0, m_liveBuffer->GetWidthBytes() * m_liveBuffer->GetSizeY());
//        }
//
//        switch (dwRet)
//        {
//            case STATUS_GRAB_TIMEOUT:
//                //			::AfxMessageBox(_T("STATUS_GRAB_TIMEOUT"));
//                break;
//
//            case STATUS_GRAB_ALREADY_RUNNING:
//                //			::AfxMessageBox(_T("STATUS_GRAB_ALREADY_RUNNING"));
//                break;
//
//            case STATUS_GRAB_DMA_OVER_FAIL:
//                //			::AfxMessageBox(_T("STATUS_GRAB_DMA_OVER_FAIL"));
//                break;
//
//            case STATUS_GRAB_FAIL:
//                //			::AfxMessageBox(_T("STATUS_GRAB_FAIL"));
//                break;
//        }
//
//        return false;
//    }
//
//    return true;
//}

bool Framegrabber2D_iGrabXQ::LiveCallback(const int& graaber_id, const int& camera_id)
{
    BYTE* pbyBuffer = live_buffer_mono->GetMem();

    DWORD retGrab = Ix_MultiGrabProcess(graaber_id, camera_id, &pbyBuffer, 1, 1);

    if (retGrab == STATUS_GRAB_SUCCESS)
    {
        return TRUE;
    }
    else
    {
        switch (retGrab)
        {
            case STATUS_GRAB_TIMEOUT:
                ::AfxMessageBox(_T("STATUS_GRAB_TIMEOUT"));
                break;

            case STATUS_GRAB_ALREADY_RUNNING:
                //			::AfxMessageBox(_T("STATUS_GRAB_ALREADY_RUNNING"));
                break;

            case STATUS_GRAB_DMA_OVER_FAIL:
                ::AfxMessageBox(_T("STATUS_GRAB_DMA_OVER_FAIL"));
                break;

            case STATUS_GRAB_FAIL:
                ::AfxMessageBox(_T("STATUS_GRAB_FAIL"));
                break;
        }

        ::Ix_GrabStop(graaber_id, camera_id);

        return false;
    }

    return true;
}

bool Framegrabber2D_iGrabXQ::SetTriggerMode(int nMode)
{
    //mc_임시방편 NGRV는 무조건 0이니까..
    const int& id_grabber = get_grabber_id();
    const int& id_camera = get_camera_id(id_grabber);
    const int& id_icf = get_camera_icf_id(id_grabber, id_camera);

    switch (nMode)
    {
        case _enumTriggermode::Freerun:
            ::Ix_SetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);
            break;

        case _enumTriggermode::Trigger:
            ::Ix_SetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Trigger);
            break;

        case _enumTriggermode::UplinkTrigger:
            ::Ix_SetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_TRIGGER_MODE, _enumTriggermode::UplinkTrigger);
            break;

        default:
            ::Ix_SetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);
            return false;
            break;
    }

    return true;
}

bool Framegrabber2D_iGrabXQ::SetCameraAcqusitionStart(const int& camera_id)
{
    Ipvm::AsyncProgress progress(L"Camera Acqusition control");

    // Camera Open...
    HANDLE handle = ::ctrlOpen(camera_id);

    // Register address
    //ULONG ulDeviceSerialNumberAddress = 0x20B0;
    ULONG ulUserBaseAddress = 0x10000000;
    ULONG ulTrigBaseAddress = ulUserBaseAddress + 0x00020000;
    ULONG ulAddressAcqStart = ulTrigBaseAddress + 0x00001084;
    //ULONG ulAddressAcqStop = ulTrigBaseAddress + 0x000010C4;

    BYTE command = 1;
    BYTE ret1 = ::ctrlCmdMsg(handle, enumMemoryWrite, 4, ulAddressAcqStart, &command, 1000);

    BYTE byAckCode = 0;
    ULONG ulSize = 0;
    BYTE ret2 = ::ctrlAckMsg(handle, byAckCode, ulSize, nullptr);

    // Close Device...
    BOOL ret3 = ::ctrlClose(handle);

    UNREFERENCED_PARAMETER(ret1); //ret 계열 변수는 디버깅을 위한 변수라 없애지 않고 남겨둔다.
    UNREFERENCED_PARAMETER(ret2);
    UNREFERENCED_PARAMETER(ret3);

    return true;
}

bool Framegrabber2D_iGrabXQ::SetSideCameraAcquisition(const int& camera_id, BOOL i_bAcquisition)
{
    ULONG ulInputAddress;
    // Camera Open...
    HANDLE handle = ::ctrlOpen(camera_id);

    //ULONG ulBaseAddress = 0x100001dc;

    if (i_bAcquisition == TRUE)
    {
        ulInputAddress = ADR_BASLER_47M_ACQ_START;
    }
    else
    {
        ulInputAddress = ADR_BASLER_47M_ACQ_STOP;
    }

    BYTE command = 1;
    BYTE ret1 = ::ctrlCmdMsg(handle, enumMemoryWrite, 4, ulInputAddress, &command, 1000);

    BYTE byAckCode = 0;
    ULONG ulSize = 0;
    BYTE ret2 = ::ctrlAckMsg(handle, byAckCode, ulSize, nullptr);

    // Close Device...
    BOOL ret3 = ::ctrlClose(handle);

    UNREFERENCED_PARAMETER(ret1); //ret 계열 변수는 디버깅을 위한 변수라 없애지 않고 남겨둔다.
    UNREFERENCED_PARAMETER(ret2);
    UNREFERENCED_PARAMETER(ret3);

    return true;
}

bool Framegrabber2D_iGrabXQ::GetCurrentGainValue(float& o_fCurrentGain) // Camera Gain Test - JHB
{
    Ipvm::AsyncProgress progress(_T("Get camera gain value"));

    DevelopmentLog::AddLog(DevelopmentLog::Type::Develop, _T("Camera Connect Start"));

    HexToFloat H2F;

    if (OpenCameraComm(0))
    {
        DevelopmentLog::AddLog(DevelopmentLog::Type::Develop, _T("Camera Connection Correctly"));

        CalcTime(_T("Connect Correctly"));

        memset(H2F.m_nHex, 0, sizeof(H2F.m_nHex));

        //BYTE byAckCode = 0;
        ReadFloat(Gain_Address, Gain_Length, (FLOAT*)H2F.m_nHex);

        o_fCurrentGain = H2F.m_fValue;

        if (o_fCurrentGain
            != SystemConfig::GetInstance()
                .m_fDefault_Camera_Gain) // 만약 Default 설정과 현재 불러온 Gain 값이 다르면, Default 값으로 돌려준다 - JHB_NGRV
        {
            o_fCurrentGain = SystemConfig::GetInstance().m_fDefault_Camera_Gain;

            WriteFloat(Gain_Address, Gain_Length, o_fCurrentGain);
        }
    }
    else
    {
        DevelopmentLog::AddLog(DevelopmentLog::Type::Develop, _T("Camera Connection Fail"));
        CalcTime(_T("Connect Fail"));
        return FALSE;
    }

    DevelopmentLog::AddLog(DevelopmentLog::Type::Develop, _T("Gain : %02f"), o_fCurrentGain);

    return CloseCameraComm();
}

bool Framegrabber2D_iGrabXQ::SetVerticalSizeY(const int& id_grabber, const int& id_camera, const int& id_icf,
    const bool i_bFullSizeY, const long i_nScanSizeY, const bool i_bSideVIsion)
{
    Ipvm::AsyncProgress progress(_T("Modify camera parameters"));

    UINT uiImageFullSizeY = BASLER_47M_IMAGE_FULL_SIZE_Y;

    if (i_bSideVIsion
        == false) //(i_bSideVIsion != true) //조건문을 이렇게 꼬지말자. 가독성 떨어진다. "아니다"라는 조건은 "==  false"로 하자.. "== false"와 "!=true"는 결과는 같지만 사람 머리는 2번 계산해야 한다.
    {
        UINT nWidth, nHeight;

        ::Ix_GetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_HORIZONTAL_SIZE, &nWidth);
        ::Ix_GetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_VERTICAL_SIZE, &nHeight);

        ::Ix_SetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_TRIGGER_MODE, _enumTriggermode::UplinkTrigger);

        set_grab_image_width(id_grabber, id_camera, nWidth);
        set_grab_image_height(id_grabber, id_camera, nHeight);
    }
    else
    {
        set_grab_image_height(id_camera, i_bFullSizeY ? uiImageFullSizeY : i_nScanSizeY);

        // 이미지 사이즈 설정
        for (long nCamidx = 0; nCamidx < SIDE_CAMERA_NUM; nCamidx++)
        {
            UINT uGrabImageHeight(0);
            UINT uWidth(0);

            ::Ix_SetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_VERTICAL_SIZE, get_grab_image_height(id_camera));

            Sleep(10);

            ::Ix_GetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_HORIZONTAL_SIZE, &uWidth);
            ::Ix_GetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_VERTICAL_SIZE, &uGrabImageHeight);

            if (CAST_UINT(get_grab_image_height(id_camera) != uGrabImageHeight))
            {
                return false;
            }

            set_grab_image_width(id_grabber, id_camera, uWidth);

            ::Ix_SetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_TRIGGER_MODE, _enumTriggermode::UplinkTrigger);
        }

        // Side는 Camera Connection이 2개가 필요하기 때문에 - 2024.07.12_JHB Side Vision
        if (apply_grab_image_offset_Y(0, enSideVisionModule::SIDE_VISIONMODULE_FRONT,
                SystemConfig::GetInstance().GetSideVisionFrontCameraOffsetY())
            == false)
            return false;

        if (apply_grab_image_offset_Y(0, enSideVisionModule::SIDE_VISIONMODULE_REAR,
                SystemConfig::GetInstance().GetSideVisionRearCameraOffsetY())
            == false)
            return false;

        //if (SetCameraOffsetY(enSideVisionModule::SIDE_VISIONMODULE_FRONT,
        //        SystemConfig::GetInstance().GetSideVisionFrontCameraOffsetY())
        //    == false)
        //    return false;

        //if (SetCameraOffsetY(enSideVisionModule::SIDE_VISIONMODULE_REAR,
        //        SystemConfig::GetInstance().GetSideVisionRearCameraOffsetY())
        //    == false)
        //    return false;
    }

    live_buffer_mono->Create(get_grab_image_width(id_camera), get_grab_image_height(id_camera));
    live_buffer_mono2->Create(
        get_grab_image_width(id_grabber, id_camera), get_grab_image_height(id_grabber, id_camera));

    return true;
}

bool Framegrabber2D_iGrabXQ::SetDigitalGainValue(float i_fInputGain, BOOL bInline)
{
    if (bInline)
    {
        WriteFloat(Gain_Address, Gain_Length, i_fInputGain);

        return TRUE;
    }
    else
    {
        if (OpenCameraComm(0))
        {
            WriteFloat(Gain_Address, Gain_Length, i_fInputGain);
        }
        else
        {
            return FALSE;
        }
    }

    m_fCurrentCameraGain = i_fInputGain; // Set Digital Gain 값 저장 - JHB_2023.02.07

    return CloseCameraComm();
}

float Framegrabber2D_iGrabXQ::GetCurrentCameraGain()
{
    return m_fCurrentCameraGain;
}

bool Framegrabber2D_iGrabXQ::OpenCameraComm(const int& camera_id)
{
    if (m_hActiveDev != INVALID_HANDLE_VALUE)
    {
        ctrlClose(m_hActiveDev);
        Sleep(100);
    }

    m_hActiveDev = ctrlOpen(camera_id);

    if (m_hActiveDev == INVALID_HANDLE_VALUE)
        return FALSE;

    return TRUE;
}

bool Framegrabber2D_iGrabXQ::CloseCameraComm()
{
    if (m_hActiveDev == INVALID_HANDLE_VALUE)
        return FALSE;

    BOOL bRet = ctrlClose(m_hActiveDev);

    m_hActiveDev = INVALID_HANDLE_VALUE;

    return bRet;
}

BYTE Framegrabber2D_iGrabXQ::_SendCmd(
    BOOL RW, INTERFACE_TYPE ifType, ULONG nLength, ULONG nAddr, BYTE* pSendData, BYTE* pRecvData)
{
    BYTE RetMsg = STATUS_CTRLCMD_FAIL;
    BYTE RetAck = STATUS_CTRLCMD_FAIL;
    BYTE nAckCode = 0;
    BYTE pAckData[256];
    ULONG nRecvLength = 0;

    if (m_hActiveDev == INVALID_HANDLE_VALUE)
        return STATUS_CTRLCMD_HANDLE_FAIL;

    RetMsg = ctrlCmdMsg(m_hActiveDev, CAST_BYTE(RW), nLength, nAddr, pSendData, 1000);

    if (RetMsg == STATUS_CTRLCMD_SUCCESS)
    {
        RetAck = ctrlAckMsg(m_hActiveDev, nAckCode, nRecvLength, pAckData, TRUE);
        if (RetAck == STATUS_CTRLCMD_SUCCESS)
        {
            ULONG* pTempBuffer = (ULONG*)pAckData;
            if (ifType == _IString)
            {
                ULONG bufferSize = nRecvLength / 4;
                for (ULONG i = 0; i < bufferSize; i++)
                    pTempBuffer[i] = XQSWAP(pTempBuffer[i]);
            }
            if (pRecvData != NULL)
                memcpy(pRecvData, pAckData, nRecvLength);
        }
        else
        {
            return STATUS_CTRLCMD_ACK_ERROR;
        }
    }
    else
        return STATUS_CTRLCMD_FAIL;

    return STATUS_CTRLCMD_SUCCESS;
}

BYTE Framegrabber2D_iGrabXQ::WriteInteger(ULONG nAddr, ULONG nLength, UINT32 nSendData)
{
    if (m_hActiveDev == INVALID_HANDLE_VALUE)
        return STATUS_CTRLCMD_HANDLE_FAIL;

    return _SendCmd(TRUE, _IInteger, nLength, nAddr, (BYTE*)&nSendData, NULL);
}

BYTE Framegrabber2D_iGrabXQ::WriteString(ULONG nAddr, ULONG nLength, BYTE* pSendData)
{
    if (m_hActiveDev == INVALID_HANDLE_VALUE)
        return STATUS_CTRLCMD_HANDLE_FAIL;

    return _SendCmd(TRUE, _IString, nLength, nAddr, pSendData, NULL);
}

BYTE Framegrabber2D_iGrabXQ::WriteDouble(ULONG nAddr, ULONG nLength, DOUBLE fSendData)
{
    if (m_hActiveDev == INVALID_HANDLE_VALUE)
        return STATUS_CTRLCMD_HANDLE_FAIL;

    return _SendCmd(TRUE, _IInteger, nLength, nAddr, (BYTE*)&fSendData, NULL);
}

BYTE Framegrabber2D_iGrabXQ::WriteFloat(ULONG nAddr, ULONG nLength, FLOAT fSendData)
{
    if (m_hActiveDev == INVALID_HANDLE_VALUE)
        return STATUS_CTRLCMD_HANDLE_FAIL;

    return _SendCmd(TRUE, _IInteger, nLength, nAddr, (BYTE*)&fSendData, NULL);
}

BYTE Framegrabber2D_iGrabXQ::ReadInteger(ULONG nAddr, ULONG nLength, UINT32* pRecvData)
{
    if (m_hActiveDev == INVALID_HANDLE_VALUE)
        return STATUS_CTRLCMD_HANDLE_FAIL;

    return _SendCmd(FALSE, _IInteger, nLength, nAddr, NULL, (BYTE*)pRecvData);
}

BYTE Framegrabber2D_iGrabXQ::ReadString(ULONG nAddr, ULONG nLength, BYTE* pRecvData)
{
    if (m_hActiveDev == INVALID_HANDLE_VALUE)
        return STATUS_CTRLCMD_HANDLE_FAIL;

    return _SendCmd(FALSE, _IString, nLength, nAddr, NULL, (BYTE*)pRecvData);
}

BYTE Framegrabber2D_iGrabXQ::ReadDouble(ULONG nAddr, ULONG nLength, DOUBLE* pRecvData)
{
    if (m_hActiveDev == INVALID_HANDLE_VALUE)
        return STATUS_CTRLCMD_HANDLE_FAIL;

    return _SendCmd(FALSE, _IInteger, nLength, nAddr, NULL, (BYTE*)pRecvData);
}

BYTE Framegrabber2D_iGrabXQ::ReadFloat(ULONG nAddr, ULONG nLength, FLOAT* pRecvData)
{
    if (m_hActiveDev == INVALID_HANDLE_VALUE)
        return STATUS_CTRLCMD_HANDLE_FAIL;

    return _SendCmd(FALSE, _IInteger, nLength, nAddr, NULL, (BYTE*)pRecvData);
}

void Framegrabber2D_iGrabXQ::CalcTime(CString i_strFileName)
{
    CString saveFileName;
    saveFileName.Format(_T("D:\\Calc_CamConnection_Time.csv"));

    SYSTEMTIME cur_time;
    GetLocalTime(&cur_time);

    CString strTime;
    strTime.Format(_T("%s : [% 02d:% 02d : % 02d. % 03d]"), (LPCTSTR)i_strFileName, cur_time.wHour, cur_time.wMinute,
        cur_time.wSecond, cur_time.wMilliseconds);

    FILE* fp = nullptr;
    _tfopen_s(&fp, saveFileName, _T("a"));
    if (fp == nullptr)
    {
        return;
    }
    fprintf(fp, "%S\n", (LPCTSTR)strTime);

    fclose(fp);
}

void Framegrabber2D_iGrabXQ::get_firmware_version(
    const int& grabber_id, unsigned int& cur_version, unsigned int& need_version)
{
    cur_version = get_use_firmware_version(grabber_id);
    need_version = IF_FW_VERSION_FIRMWARE_MIN;
}

void Framegrabber2D_iGrabXQ::get_library_version(unsigned int& cur_version, unsigned int& need_version)
{
    cur_version = get_use_library_version();
    need_version = IF_LIB_VERSION;
}

bool Framegrabber2D_iGrabXQ::apply_grab_image_offset_Y(
    const int& grabber_idx, const int& camera_idx, const long& offset)
{
    const int& id_grabber = get_grabber_id(grabber_idx);
    const int& id_camera = get_camera_id(id_grabber, camera_idx);

    // Register address
    //ULONG ulAddressHeight = ADR_BASLER_47M_HEIGHT;
    ULONG ulAddressOffsetY = ADR_BASLER_47M_OFFSETY;
    ULONG ulAddressAcqStart = ADR_BASLER_47M_ACQ_START;
    ULONG ulAddressAcqStop = ADR_BASLER_47M_ACQ_STOP;

    UINT uiImageFullSizeY = BASLER_47M_IMAGE_FULL_SIZE_Y;

    // Communication Status Check
    long nCommand = 1;

    // 1. Camera Open
    HANDLE handle = ::ctrlOpen(id_camera);

    BYTE ret = 0;
    // 2. Parameter 설정 전 Acq Stop
    bool bStopVal = ctrlSetAcqStop(handle); //Grabber Stop I/O [Basler Camera 한정]
    ret = ::ctrlCmdMsg(handle, enumMemoryWrite, sizeof(long), ulAddressAcqStop, (BYTE*)&nCommand, 1000);
    Sleep(300); //mc_500ms 까지는 사용해도 괜찮을듯

    // Write OffsetY
    set_grab_image_offset_Y(id_camera, offset);

    if (CAST_UINT(get_grab_image_height(id_camera) + get_grab_image_offset_Y(id_camera)) >= uiImageFullSizeY)
        set_grab_image_offset_Y(id_camera, 0);

    const int& offst_Y = get_grab_image_offset_Y(id_camera);
    ret = ::ctrlCmdMsg(handle, enumMemoryWrite, sizeof(offst_Y), ulAddressOffsetY, (BYTE*)&offst_Y, 1000);
    Sleep(300); //mc_500ms 까지는 사용해도 괜찮을듯

    bool bStartVal = ctrlSetAcqStart(handle); //Grabber Start I/O [Basler Camera 한정]
    ret = ::ctrlCmdMsg(handle, enumMemoryWrite, sizeof(long), ulAddressAcqStart, (BYTE*)&nCommand, 1000);

    // Close Device...
    ret = CAST_BYTE(::ctrlClose(handle));

    UNREFERENCED_PARAMETER(ret); //ret 계열 변수는 디버깅을 위한 변수라 없애지 않고 남겨둔다.
    UNREFERENCED_PARAMETER(bStartVal);
    UNREFERENCED_PARAMETER(bStopVal);

    return true;
}