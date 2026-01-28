//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "FrameGrabber3D_iGrabXE.h"

//CPP_2_________________________________ This project's headers
#include "SlitbeamDistortionData.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../InformationModule/dPI_DataBase/SlitBeam3DParameters.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedBuildConfiguration/iGrabXE/iFsCtrlCmd.h"
#include "../../SharedBuildConfiguration/iGrabXE/iManX.h"
#include "../../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/AlgorithmGpu/ImageProcessingGpu.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image16u.h>
#include <Ipvm/Base/Image32r.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/ImageFile.h>
#include <Ipvm/Base/LineEq32r.h>
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/SbtCore/SbtCoreGpu.h>
#include <Ipvm/SbtCore/SbtCorePara.h>
#include <Ipvm/Widget/AsyncProgress.h>
#include <afxmt.h>

//CPP_5_________________________________ Standard library headers
#include <fstream>
#include <thread>
#include <vector>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IF_LIB_VERSION \
    0x00990406 //mc_LibraryVersion이 변경될때마다 값을 변경하여 준다 [기존에 iGrabX에는 Define이 있었지만, 삭제되어 여기에서 관리]
#define IF_FW_VERSION_MIN 0x0000f333

#define DEFAULT_IMAGE_SIZE_X 4096
#define DEFAULT_IMAGE_SIZE_Y 200
#define DEFAULT_IMAGE_FULL_SIZE_Y 3072

#define SWAP(x) ((x & 0xff000000) >> 24) | ((x & 0x000000ff) << 24) | ((x & 0x00ff0000) >> 8) | ((x & 0x0000ff00) << 8)

//CPP_7_________________________________ Implementation body
//
static const float g_miminumExposureLow_ms = 0.01f;

static CCriticalSection g_cs;

enum _BootstrapInterface
{
    _IInteger,
    _IString,
    _IEnumerate
};

BYTE CmdSet(HANDLE hDev, UINT nAccType, UINT RegAddr, BYTE* pCmdData, INT nLength, BYTE* pReplyData, ULONG Interface,
    DWORD dwTimeout)
{
    DWORD dwRet = STATUS_CTRLCMD_HANDLE_FAIL;

    for (int nTry = 0; nTry < 3; nTry++) //3
    {
        if (STATUS_CTRLCMD_SUCCESS
            == (dwRet = CAST_DWORD(ctrlCmdMsg(hDev, CAST_BYTE(nAccType), nLength, RegAddr, pCmdData, dwTimeout))))
        {
            BYTE nAckCode = 0;
            ULONG nReplySize = 0;

            if (STATUS_CTRLCMD_SUCCESS
                == (dwRet = CAST_DWORD(ctrlAckMsg(hDev, nAckCode, nReplySize, pReplyData, TRUE))))
            {
                ULONG* pUlong = (ULONG*)pReplyData;
                if (Interface == _IString)
                {
                    ULONG swapSize = nReplySize / 4;
                    for (ULONG i = 0; i < swapSize; i++)
                        pUlong[i] = SWAP(pUlong[i]);
                }
            }
        }
        Sleep(100);

        if (dwRet == STATUS_CTRLCMD_SUCCESS)
            break;
    }

    return CAST_BYTE(dwRet);
}

bool WritePort(HANDLE hDev, CString strCmd)
{
    const unsigned int COMM_WRITE_DATA = 0x80104;

    BYTE dataBuff[256] = {
        0,
    };
    CStringA astrCmd(strCmd);

    size_t nCommLength = astrCmd.GetLength();

    for (int i = 0; i < nCommLength; i++)
        dataBuff[i] = astrCmd.GetAt(i);
    dataBuff[nCommLength] = '\r';

    nCommLength += 1;
    size_t numRegisterWritten = (nCommLength / 4) * 4 + ((nCommLength % 4 == 0) ? 0 : 4) + 4;
    //---------------------------------------------------------------
    ULONG dataBuff1[256] = {
        0,
    };
    dataBuff1[0] = CAST_ULONG(nCommLength);

    memcpy(&dataBuff1[1], dataBuff, nCommLength);

    for (int i = 0; i < numRegisterWritten / 4; i++)
        dataBuff1[i] = SWAP(dataBuff1[i]);

    int nReadBytes;
    BYTE Ret = CmdSet(hDev, enumMemoryWrite, COMM_WRITE_DATA, (BYTE*)dataBuff1, static_cast<INT>(numRegisterWritten),
        (BYTE*)&nReadBytes, _BootstrapInterface::_IInteger, 1000);

    return Ret == STATUS_CTRLCMD_SUCCESS;
}

bool ReadPort(HANDLE hDev, CString& strRes)
{
    //int   DbgSize = 0;
    //ULONG DbgBuff[256];

    const unsigned int COMM_READ_LENGTH = 0x80000;
    const unsigned int COMM_READ_DATA = 0x80004;

    int nReadBytes = 0;
    size_t dataLength = 4;

    BOOL Ret = CmdSet(hDev, enumMemoryRead, COMM_READ_LENGTH, NULL, static_cast<INT>(dataLength), (BYTE*)&nReadBytes,
        _BootstrapInterface::_IInteger, 1000);
    //Ret =ctrlDbgAckMsg(hDev, DbgSize, DbgBuff);

    if (Ret != STATUS_CTRLCMD_SUCCESS)
    {
        return false;
    }

    dataLength = (nReadBytes);
    size_t numRegistersRead = (dataLength / 4) * 4 + ((dataLength % 4 == 0) ? 0 : 4);

    BYTE Buffer[4096] = {
        0,
    };

    Ret = CmdSet(
        hDev, enumMemoryRead, COMM_READ_DATA, NULL, static_cast<INT>(numRegistersRead), (BYTE*)Buffer, _IInteger, 1000);

    if (Ret != STATUS_CTRLCMD_SUCCESS)
    {
        return false;
    }

    //swap.
    UINT* pBuff = (UINT*)Buffer;
    for (int i = 0; i < numRegistersRead / 4; i++)
        pBuff[i] = SWAP(pBuff[i]);

    strRes = (char*)pBuff;

    return true;
}

void SolveBlackSunProblem(const int& camera_id)
{
    Ipvm::AsyncProgress progress(L"Camera sensor control");

    HANDLE hDev = ::ctrlOpen(camera_id);

    if (hDev == INVALID_HANDLE_VALUE)
    {
        ::AfxMessageBox(_T("Cannot connect to camera"), MB_ICONERROR);
        ::exit(0);
    }

    //--------------------------------------
    // Write Command "wcis 102 0x256e"
    //--------------------------------------
    CString strRes;

    if (WritePort(hDev, _T("aar vieworks")) == FALSE)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPort(hDev, strRes) == FALSE)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (WritePort(hDev, _T("wcis 102 0x256e")) == false)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPort(hDev, strRes) == false)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    //--------------------------------------
    // Read Command "rcis 102"
    //--------------------------------------
    if (WritePort(hDev, _T("rcis 102")) == false)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPort(hDev, strRes) == false)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    ::ctrlClose(hDev);
}

long GetCurBloValue(CString strRes)
{
    long nStart = strRes.Find('\n');
    CString strValue;
    char ch;
    BOOL bStartNumber = FALSE;
    for (long n = nStart; n < strRes.GetLength(); n++)
    {
        ch = static_cast<char>(strRes.GetAt(n));
        if (ch >= '0' && ch <= '9')
        {
            bStartNumber = TRUE;
            strValue += ch;
        }
        else if (bStartNumber)
            break;
    }
    return ((long)_ttoi(strValue));
}

void CameraAnalogOffsetControl(
    const int& camera_id, const long& offset1, const long& offset2, const CString& camera_serial)
{
    Ipvm::AsyncProgress progress(L"Camera offset control");

    HANDLE hDev = ::ctrlOpen(camera_id);

    if (hDev == INVALID_HANDLE_VALUE)
    {
        ::AfxMessageBox(_T("Cannot connect to camera"), MB_ICONERROR);
        ::exit(0);
    }

    //--------------------------------------
    // Read Command "gblo"/"gblo2" //현재 카메라의 Analog Gain 을 읽어 수치화
    //--------------------------------------
    CString strRes, strCommand;
    long nCurOffset1(0), nCurOffset2(0);

    if (WritePort(hDev, _T("aar vieworks")) == FALSE)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPort(hDev, strRes) == FALSE)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (WritePort(hDev, _T("gblo")) == false)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPort(hDev, strRes) == false)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    nCurOffset1 = GetCurBloValue(strRes);

    if (WritePort(hDev, _T("gblo2")) == false)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPort(hDev, strRes) == false)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    nCurOffset2 = GetCurBloValue(strRes);

    //읽은 Analog Gain을 기존에 저장된 값이 없으면 System.ini 에 저장하여 Ref로 삼는다.
    CString str3DCameraSN = ::SystemConfig::GetInstance().m_str3DCameraSN;
    //현재 사용하는 카메라 시리얼과  System.ini에 있는 카메라 시리얼이 다르면 오리진 초기화
    if (str3DCameraSN != camera_serial && camera_serial.IsEmpty() == false)
    {
        ::SystemConfig::GetInstance().m_n3DCameraAnalogOffset1Origin = 0;
        ::SystemConfig::GetInstance().m_n3DCameraAnalogOffset2Origin = 0;
        ::SystemConfig::GetInstance().m_str3DCameraSN = camera_serial;
    }

    if (::SystemConfig::GetInstance().m_n3DCameraAnalogOffset1Origin == 0)
        ::SystemConfig::GetInstance().m_n3DCameraAnalogOffset1Origin = nCurOffset1;
    if (::SystemConfig::GetInstance().m_n3DCameraAnalogOffset2Origin == 0)
        ::SystemConfig::GetInstance().m_n3DCameraAnalogOffset2Origin = nCurOffset2;

    ::SystemConfig::GetInstance().Save3DCameraAnalogOffset();

    //--------------------------------------
    // write Command "sblo"/"sblo2" //Ref에 Offset을 적용하여 이를 Camera로 던진다.
    //--------------------------------------
    long nTargetOffset1 = ::SystemConfig::GetInstance().m_n3DCameraAnalogOffset1Origin + offset1;
    long nTargetOffset2 = ::SystemConfig::GetInstance().m_n3DCameraAnalogOffset2Origin + offset2;

    strCommand.Format(_T("sblo %d"), nTargetOffset1);
    if (WritePort(hDev, strCommand) == false)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPort(hDev, strRes) == false)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    strCommand.Format(_T("sblo2 %d"), nTargetOffset2);
    if (WritePort(hDev, strCommand) == false)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPort(hDev, strRes) == false)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    ::ctrlClose(hDev);
}

void TurnOnOffPrnuCorrection(const int& camera_id, bool on)
{
    Ipvm::AsyncProgress progress(L"Camera sensor control : change PRNU setting");

    HANDLE hDev = ::ctrlOpen(camera_id);

    if (hDev == INVALID_HANDLE_VALUE)
    {
        ::AfxMessageBox(_T("Cannot connect to camera"), MB_ICONERROR);
        ::exit(0);
    }

    //--------------------------------------
    // Write Command "sprnu 1"
    //--------------------------------------
    CString strRes;

    if (WritePort(hDev, _T("aar vieworks")) == FALSE)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPort(hDev, strRes) == FALSE)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (on)
    {
        if (WritePort(hDev, _T("sprnu 1")) == false)
        {
            ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
            ::exit(0);
        }
    }
    else
    {
        if (WritePort(hDev, _T("sprnu 0")) == false)
        {
            ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
            ::exit(0);
        }
    }

    if (ReadPort(hDev, strRes) == false)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    //--------------------------------------
    // Read Command "rprnu"
    //--------------------------------------
    if (WritePort(hDev, _T("rprnu")) == false)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPort(hDev, strRes) == false)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    ::ctrlClose(hDev);
}

void TurnOnOffDsnuCorrection(const int& camera_id, bool on)
{
    Ipvm::AsyncProgress progress(L"Camera sensor control : change DSNU setting");

    HANDLE hDev = ::ctrlOpen(camera_id);

    if (hDev == INVALID_HANDLE_VALUE)
    {
        ::AfxMessageBox(_T("Cannot connect to camera"), MB_ICONERROR);
        ::exit(0);
    }

    //--------------------------------------
    // Write Command "set sdsnu 1"
    //--------------------------------------
    CString strRes;

    if (WritePort(hDev, _T("aar vieworks")) == FALSE)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPort(hDev, strRes) == FALSE)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (on)
    {
        if (WritePort(hDev, _T("sdsnu 1")) == false)
        {
            ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
            ::exit(0);
        }
    }
    else
    {
        if (WritePort(hDev, _T("sdsnu 0")) == false)
        {
            ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
            ::exit(0);
        }
    }

    if (ReadPort(hDev, strRes) == false)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    //--------------------------------------
    // Read Command "rdsnu"
    //--------------------------------------
    if (WritePort(hDev, _T("rdsnu")) == false)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPort(hDev, strRes) == false)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    ::ctrlClose(hDev);
}

FrameGrabber3D_iGrabXE::FrameGrabber3D_iGrabXE()
    : grabber_id_use_grab(0)
    , camera_id_use_grab(0)
    , m_profileNumber(0)
    , m_zmapImage(new Ipvm::Image32r)
    , m_vmapImage(new Ipvm::Image32r)
    , m_vmapResizeTempImage(new Ipvm::Image16u)
    , m_pCore(NULL)
    , m_resultZmapImage(nullptr)
    , m_resultVmapImage(nullptr)
    , m_resultStitchIndex(0)
    , m_scanImageCount(0)
    , m_distortionData(new SlitbeamDistortionData(DEFAULT_IMAGE_SIZE_X))
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    camera_serials.clear();
    custom_grab_event_counts.clear();
    dsnu_images.clear();
    dsnu_binning_images.clear();
    m_use_binning.clear();

    // ViewWorks 카메라는 파워 온 후에 Acquisition Start 모드로 들어가게 해야한다.
    const CString path_xml_file = get_camera_param_save_path();
    const LPCTSTR resource_name = get_camera_param_resourceID(SystemConfig::GetInstance().GetVisionType());

    FrameGrabber_Base::ExtractResourceFile(_T("XML"), resource_name, path_xml_file);
    FrameGrabber_Base::SavesettingFile(path_xml_file);

    init_grabber_info();

    init_camera_info(path_xml_file);

    init_dsnu_image_buf(DEFAULT_IMAGE_SIZE_X, DEFAULT_IMAGE_FULL_SIZE_Y); //나중에 변수로 받든지 말든지

    init_camera_serial();

    init_image_size_default(DEFAULT_IMAGE_SIZE_X, DEFAULT_IMAGE_SIZE_Y); //나중에 변수로 받든지 말든지

    init_camera_default_param();

    apply_camera_additional_param();

    apply_camera_dsnu();

    //임시로 만든다
    //SystemConfig가 camera대수 만큼 고려되어 있지 않아서 일단은 하나로 set한다

    for (int grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
    {
        const int& id_grabber = get_grabber_id(grabber_idx);
        for (int camera_idx = 0; camera_idx < get_camera_num(id_grabber); camera_idx++)
        {
            const int& id_camera = get_camera_id(id_grabber, camera_idx);
            const int& id_icf = get_camera_icf_id(id_grabber, id_camera);

            //Scale set을 다른방법으로 고려해야 함
            SystemConfig::GetInstance().SetScale3D(
                get_height_resolution_px2um(false, SystemConfig::GetInstance().m_slitbeamHeightScaling));

            ChangeVerticalSize(
                id_grabber, id_camera, id_icf, false, false, get_grab_image_height(grabber_idx, camera_idx));
        }
    }

    //

    //GPU는 1개로 한정한다
    ////===========================================///////
    ////================== Mem Alloc  ================//////
    m_pCore = new Ipvm::SbtCoreGpu(0);
    m_pCoreParam = new Ipvm::SbtCorePara;
}

FrameGrabber3D_iGrabXE::~FrameGrabber3D_iGrabXE(void)
{
    if (get_grabber_num() > 0)
    {
        for (long grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
        {
            const int& grabber_id = get_grabber_id(grabber_idx);
            for (long camera_idx = 0; camera_idx < get_camera_num(grabber_idx); camera_idx++)
            {
                const int& camera_id = get_camera_id(grabber_id, camera_idx);
                ::Ix_CloseChannel(grabber_id, camera_id);
            }

            ::Ix_CloseSystem(grabber_id);
        }
    }

    delete m_vmapResizeTempImage;
    delete m_zmapImage;
    delete m_vmapImage;
    delete m_distortionData;

    delete m_pCore;
    delete m_pCoreParam;
}

void FrameGrabber3D_iGrabXE::init_grabber_info()
{
    UINT libraryVersion(0);
    ::Ix_GetLibraryVersion(libraryVersion);

    if (libraryVersion != IF_LIB_VERSION)
    {
        //mc_다른곳에서 MSG를 띄울것이다
        SystemConfig::GetInstance().Set_is_iGrabFirmware_and_LibraryVersion_Mismatch(true);
    }

    set_library_version(libraryVersion);

    if (set_grabber_num(::Ix_DeviceList()) == false)
    {
        ::AfxMessageBox(_T("Failed to open iGrabXE"), MB_ICONERROR | MB_OK);
        ::exit(0);
    }

    for (int grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
    {
        const int& id_grabber = get_grabber_id(grabber_idx);
        if (::Ix_OpenSystem(grabber_idx) == FALSE)
        {
            CString strID("");
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

            if (firmware_version < IF_FW_VERSION_MIN)
            {
                //mc_다른곳에서 MSG를 띄울것이다
                SystemConfig::GetInstance().Set_is_iGrabFirmware_and_LibraryVersion_Mismatch(true);
            }
        }
    }
}

void FrameGrabber3D_iGrabXE::init_camera_info(const CString& camera_param_file_path)
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
        const int id_graaber = get_grabber_id(grabber_idx);
        for (int camera_idx = 0; camera_idx < MAX_CAMERA_NUMBER; camera_idx++)
        {
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
            ::Ix_NewXCF(id_graaber, camera_idx, id_icf);

            if (id_icf < 0)
            {
                ::Ix_CloseChannel(id_graaber, camera_idx);
                continue;
            }

            ::Ix_SetXCFParam(id_graaber, camera_idx, id_icf, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);

            //Camera & icf set
            set_camera_icf(id_graaber, camera_idx, id_icf);

            open_channel_num++;
        }
    }

    if (open_channel_num == 0) //한개도 Open하지 못한상황이다 종료한다
    {
        CString error_msg(_T("Failed to open framegrabber : all Channel"));
        ::AfxMessageBox(error_msg);
        error_msg.Empty();
        ::exit(0);
    }

    ////몇 Channel을 물었는지 확인해보고 없으면 Full로 물게 해줘야함
    //BYTE byChannel = 0x00;
    //if (!::Ix_GetChannelPlugAssign(m_iSYS_ID, m_iChannel_ID, byChannel))
    //{
    //    ::AfxMessageBox(_T("Failed to open framegrabber : Fail to Open Camera Link Channel, Please Check ")
    //                    _T("iGrabXE_Control Program Setting"));
    //}
    //else
    //{
    //    if (byChannel != 0xff)
    //    {
    //        Ipvm::AsyncProgress progress(L"Setting Camera Link Channel");
    //        Ix_SetChannelPlugAssign(m_iSYS_ID, m_iChannel_ID, 0xff);
    //    }
    //}
    //
}

bool FrameGrabber3D_iGrabXE::check_is_valid_camera_channel(const int& id_graaber, const int& camera_idx)
{
    if (::Ix_OpenChannel(id_graaber, camera_idx) == FALSE) //이게 fase면 grabber가 문제있는 상황
    {
        return false;
    }
    else
    {
        BYTE plug_channel(0x00);
        if (::Ix_GetChannelPlugAssign(id_graaber, camera_idx, plug_channel) == TRUE && plug_channel != 0x00)
        {
            return true;
        }
        else //할당이 안된상황
            return false;
    }

    return true;
}

bool FrameGrabber3D_iGrabXE::init_dsnu_image_buf(const int32_t& width, const int32_t& height)
{
    if (width < 0 || height < 0)
    {
        popup_function_error(_T("init_dsnu_image_buf"));
        return false;
    }

    for (int grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
    {
        const int& id_grabber = get_grabber_id(grabber_idx);
        for (int camera_idx = 0; camera_idx < get_camera_num(id_grabber); camera_idx++)
        {
            const int& id_camera = get_camera_id(id_grabber, camera_idx);

            Ipvm::Image8u dsnu_img(width, height);
            dsnu_img.FillZero();

            dsnu_images[id_grabber][id_camera] = dsnu_img;

            Ipvm::Image8u dsnu_binning_img(width, height / 2);
            dsnu_binning_img.FillZero();

            dsnu_binning_images[id_grabber][id_camera] = dsnu_binning_img;
        }
    }

    return true;
}

bool FrameGrabber3D_iGrabXE::init_camera_serial()
{
    long retry_count(3);

    for (int grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
    {
        const int& id_grabber = get_grabber_id(grabber_idx);
        for (int camera_idx = 0; camera_idx < get_camera_num(id_grabber); camera_idx++)
        {
            CString camera_serial("");
            const int& id_camera = get_camera_id(id_grabber, camera_idx);
            for (long try_idx = 0; try_idx < retry_count; try_idx++)
            {
                if (get_device_serial(id_camera, camera_serial) == true || camera_serial.IsEmpty() == false)
                {
                    break;
                }
                else
                {
                    Sleep(10);

                    if (try_idx == retry_count - 1) //3번했는데 실패하면 바로종료
                    {
                        ::AfxMessageBox(
                            _T("Cannot read camera serial number\nPlease restart the Vision S/W, after checking ")
                            _T("the CXP Cable."),
                            MB_ICONERROR | MB_OK);
                        ::exit(0);
                    }
                }
            }

            camera_serials[id_grabber][id_camera] = camera_serial;
        }
    }

    return true;
}

bool FrameGrabber3D_iGrabXE::get_device_serial(const int& camera_id, CString& serialNumber)
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

    ULONG swapSize = ulSize / 4;
    for (ULONG i = 0; i < swapSize; i++)
        replyMem[i] = SWAP(replyMem[i]);

    serialNumber = deviceSerialNumberBuffer;

    // Close Device...
    BYTE ret3 = CAST_BYTE(::ctrlClose(handle));

    UNREFERENCED_PARAMETER(ret1); //ret 계열 변수들은 디버깅용인 관계로 삭제하지 못함
    UNREFERENCED_PARAMETER(ret2);
    UNREFERENCED_PARAMETER(ret3);

    if (serialNumber.IsEmpty() == true)
        return false;

    return true;
}

bool FrameGrabber3D_iGrabXE::init_image_size_default(const unsigned int& width, const unsigned int& height)
{
    if (width < 0 || height < 0)
    {
        popup_function_error(_T("init_image_size_default"));
        return false;
    }

    for (long grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
    {
        const int& id_grabber = get_grabber_id(grabber_idx);
        for (long camera_idx = 0; camera_idx < get_camera_num(id_grabber); camera_idx++)
        {
            set_grab_image_width(grabber_idx, camera_idx, width);
            set_grab_image_height(grabber_idx, camera_idx, height);
            set_grab_image_offset_Y(grabber_idx, camera_idx, 0);
        }
    }

    return true;
}

bool FrameGrabber3D_iGrabXE::init_camera_default_param()
{
    for (int grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
    {
        const int& id_grabber = get_grabber_id(grabber_idx);
        for (int camera_idx = 0; camera_idx < get_camera_num(id_grabber); camera_idx++)
        {
            const int& id_camera = get_camera_id(id_grabber, camera_idx);
            const int& id_icf = get_camera_icf_id(id_grabber, id_camera);

            //Parameter 변경전에 acq. stop 해준다.[추가해야함]

            ::Ix_SetXCFParam(
                id_grabber, id_camera, id_icf, XCF_CTL_HORIZONTAL_SIZE, get_grab_image_width(grabber_idx, camera_idx));
            ::Ix_SetXCFParam(
                id_grabber, id_camera, id_icf, XCF_CTL_VERTICAL_SIZE, get_grab_image_height(grabber_idx, camera_idx));
            ::Ix_SetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_TRIGGER_TYPE, _enumTriggerType::HardwareTrigger);
            ::Ix_SetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_PAGE_TRIGGER, FALSE);
            ::Ix_SetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_IMAGE_BIT, _enumImagebit::Image8bit);
            ::Ix_SetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_CAM_MODE, _enumCameraMode::AreaScan);

            double validFramePeriod_ms(0.);
            set_exposure_time(grabber_idx, camera_idx, 2., validFramePeriod_ms);

            int cur_cust_event_num(0), max_cust_event_num(0);
            ::Ix_GetCustomGrabEvent(id_grabber, id_camera, &cur_cust_event_num, &max_cust_event_num);
            ::Ix_SetCustomGrabEvent(id_grabber, id_camera, max_cust_event_num);
            custom_grab_event_counts[id_grabber][id_camera] = max_cust_event_num;

            if (max_cust_event_num < 30)
            {
                CString str("");
                str.Format(_T("Library version : %X\r\nFirmware version : %X\r\n\r\nCustom event number is less than ")
                           _T("required ")
                           _T("value.\r\n\r\nRequired value : %d\r\nCurrent value : %d"),
                    get_use_library_version(), get_use_firmware_version(id_grabber), 30, max_cust_event_num);
                ::AfxMessageBox(str, MB_ICONERROR | MB_OK);
                str.Empty();

                ::exit(0);
            }
        }
    }

    return true;
}

bool FrameGrabber3D_iGrabXE::apply_camera_additional_param()
{
    for (int grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
    {
        const int& id_grabber = get_grabber_id(grabber_idx);
        for (int camera_idx = 0; camera_idx < get_camera_num(id_grabber); camera_idx++)
        {
            const int& id_camera = get_camera_id(id_grabber, camera_idx);
            const CString& camera_serial = camera_serials[id_grabber][id_camera];

            //analogoffset도 카메라 개수만큼 되어야 한다
            long n3DCameraAnalogOffset1 = ::SystemConfig::GetInstance().m_n3DCameraAnalogOffset1;
            long n3DCameraAnalogOffset2 = ::SystemConfig::GetInstance().m_n3DCameraAnalogOffset2;

            SolveBlackSunProblem(id_camera);
            CameraAnalogOffsetControl(id_camera, n3DCameraAnalogOffset1, n3DCameraAnalogOffset2, camera_serial);
            TurnOnOffPrnuCorrection(id_camera, true);
            TurnOnOffDsnuCorrection(id_camera, false);
        }
    }

    return true;
}

bool FrameGrabber3D_iGrabXE::apply_camera_dsnu()
{
    for (int grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
    {
        const int& id_grabber = get_grabber_id(grabber_idx);
        for (int camera_idx = 0; camera_idx < get_camera_num(id_grabber); camera_idx++)
        {
            const int& id_camera = get_camera_id(id_grabber, camera_idx);

            auto& dsnu_image = dsnu_images[id_grabber][id_camera];
            auto& dsnu_binning_image = dsnu_binning_images[id_grabber][id_camera];
            const CString& camera_serial = camera_serials[id_grabber][id_camera];

            if (load_dsnu_image(camera_serial, dsnu_image, dsnu_binning_image) == false)
            {
                const int& id_icf = get_camera_icf_id(id_grabber, id_camera);
                make_dsnu_image(id_grabber, id_camera, id_icf, custom_grab_event_counts[id_grabber][id_camera],
                    dsnu_image, dsnu_binning_image, camera_serial);
            }

            //이것도 camera개수만큼 돌아야할듯
            if (m_distortionData->reloadProfile() == false)
            {
                DevelopmentLog::AddLog(DevelopmentLog::Type::Warning, _T("Cannot open slitbeam distortion profile."));
                DevelopmentLog::AddLog(DevelopmentLog::Type::Warning, _T("Calibrate optics distortion."));
            }
        }
    }

    return true;
}

//2023.02.23_DEV. Debug Log
static unsigned int g_cb_call_count = 0;
static unsigned int g_cb_sum_count = 0;
static unsigned int g_hard_intr_count_s = 0; //시작 (start)
static unsigned int g_soft_intr_count_s = 0; //시작 (start)
static unsigned int g_hard_intr_count_e = 0; //끝 (end)
static unsigned int g_soft_intr_count_e = 0; //끝 (end)
//

bool FrameGrabber3D_iGrabXE::grab(const int grabber_idx, const int camera_idx, const int stitchIndex,
    Ipvm::Image32r& zmapImage, Ipvm::Image16u& vmapImage, float& height_range_min, float& height_range_max,
    Ipvm::AsyncProgress* progress)
{
    if (is_valid_index(grabber_idx, camera_idx) == false)
        return false;

    const int& id_grabber = get_grabber_id(grabber_idx);
    const int& id_camera = get_camera_id(id_grabber, camera_idx);
    const int& icf_id = get_camera_icf_id(id_grabber, id_camera);

    grabber_id_use_grab = id_grabber;
    camera_id_use_grab = id_camera;

    if (progress)
    {
        progress->Post(L"Grab start");
    }

    ResetiGrab3D_Error_LOG();

    m_resultZmapImage = &zmapImage;
    m_resultVmapImage = &vmapImage;
    m_resultStitchIndex = stitchIndex;
    height_range_min = -0.5f * m_pCore->GetHeightRange_um(*m_pCoreParam);
    height_range_max = 0.5f * m_pCore->GetHeightRange_um(*m_pCoreParam);

    m_zmapImage->FillZero();
    m_vmapImage->FillZero();

    auto coreResult = m_pCore->ProcessAsync(NULL, *m_zmapImage, *m_vmapImage); //kircheis_SDK127

    if (coreResult != Ipvm::Status::e_ok)
    {
        m_strErrorMessage = Ipvm::ToString(coreResult);
        SetiGrab3D_Error_LOG();
        return false;
    }

    ::Ix_SetXCFParam(id_grabber, id_camera, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::UplinkTrigger);

    //2023.02.23_DEV. Debug Log
    g_cb_call_count = 0;
    g_cb_sum_count = 0;
    g_hard_intr_count_s = 0;
    g_soft_intr_count_s = 0;
    g_hard_intr_count_e = 0;
    g_soft_intr_count_e = 0;
    //

    //쓰레드로 올림
    HANDLE hThread = GetCurrentThread();
    m_dwCurrentThreadPriority = GetThreadPriority(hThread);
    SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);

    m_scanImageCount = 0;

    const DWORD dwCalcTimeOut = SystemConfig::GetInstance().GetGrabTimeoutTime();

    DWORD ret = ::Ix_MultiGrabProcess(id_grabber, id_camera, NULL, 0, m_profileNumber, MtgWaitOp::MTG_GRB_ISSUE,
        dwCalcTimeOut, ScanEnd, this); //mc_23.05.30

    //2023.02.23_DEV. Debug Log
    Ix_DbgHardIntrCount(id_grabber, id_camera, g_hard_intr_count_s); // 항상 0이 나와야됨.
    Ix_DbgSoftIntrCount(id_grabber, id_camera, g_soft_intr_count_s); // 항상 0이 나와야됨.

    m_siGrab3D_Error_LOG.m_hard_intr_count_s = g_hard_intr_count_s;
    m_siGrab3D_Error_LOG.m_soft_intr_count_s = g_soft_intr_count_s;
    //

    //2023.05.30_DEV. Debug Log
    m_siGrab3D_Error_LOG.m_dwDbgMultGrabProcessReturnVal = ret;
    //

    if (ret != STATUS_GRAB_START_SUCCESS)
    {
        ::Ix_SetXCFParam(id_grabber, id_camera, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);

        m_strErrorMessage = _T("MultiGrabProcess Fail");

        SetiGrab3D_Error_LOG();
    }

    return ret;
}

bool FrameGrabber3D_iGrabXE::wait_grab_end(const int& grabber_idx, const int& camera_idx, Ipvm::AsyncProgress* progress)
{
    if (is_valid_index(grabber_idx, camera_idx) == false)
        return false;

    const int& id_grabber = get_grabber_id(grabber_idx);
    const int& id_camera = get_camera_id(id_grabber, camera_idx);
    const int& icf_id = get_camera_icf_id(id_grabber, id_camera);

    m_strErrorMessage.Empty();
    if (progress)
    {
        progress->Post(_T("Wait for grab end"));
    }

    //{{mc_2023.03.14_H/W Info
    //m_siGrab3D_Error_LOG.m_fCPU_Temperature = SystemConfig::GetInstance().Get_CPU_InfoData(eCPUInfoDataType::eCPUInfoDataType_CPU_Temperature);
    m_siGrab3D_Error_LOG.m_fPC_UsagePhysicalMemory = SystemConfig::GetInstance().Get_UsageMemoryData(
        eMemoryType::eMemoryType_PC, eMemoryDataType::eMemoryDataType_PhysicalMemory);
    m_siGrab3D_Error_LOG.m_fiPack_UsagePhysicalMemory = SystemConfig::GetInstance().Get_UsageMemoryData(
        eMemoryType::eMemoryType_iPack, eMemoryDataType::eMemoryDataType_PhysicalMemory);
    //}}

    //여기!! 여기서 풀기!
    DWORD dwRet = ::Ix_MultiGrabWait(id_grabber, id_camera);

    HANDLE hThread = GetCurrentThread();
    SetThreadPriority(hThread, m_dwCurrentThreadPriority);

    ::Ix_SetXCFParam(id_grabber, id_camera, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);

    //2023.03.08_DEV. Debug Log Test
    DWORD dwState(CAST_DWORD(-1));
    ::Ix_DbgRunState(id_grabber, id_camera, dwState);
    m_siGrab3D_Error_LOG.m_dwDbgRunState = dwState;
    //

    //2023.03.14_ATM Log Test
    if (dwState != -1)
    {
        //2023.02.23_DEV. Debug Log
        Ix_DbgHardIntrCount(id_grabber, id_camera, g_hard_intr_count_e);
        Ix_DbgSoftIntrCount(id_grabber, id_camera, g_soft_intr_count_e);

        m_siGrab3D_Error_LOG.m_hard_intr_count_e = g_hard_intr_count_e;
        m_siGrab3D_Error_LOG.m_soft_intr_count_e = g_soft_intr_count_e;
        //
    }
    //

    if (dwRet == STATUS_GRAB_SUCCESS)
    {
        return true;
    }

    m_pCore->Abort();

    //2023.02.23_DEV. Debug Log
    Ix_DbgHardIntrCount(id_grabber, id_camera, g_hard_intr_count_e);
    Ix_DbgSoftIntrCount(id_grabber, id_camera, g_soft_intr_count_e);

    m_siGrab3D_Error_LOG.m_hard_intr_count_e = g_hard_intr_count_e;
    m_siGrab3D_Error_LOG.m_soft_intr_count_e = g_soft_intr_count_e;
    //

    //2023.03.08_DEV. Debug Log
    DWORD dwStateFail(CAST_DWORD(-1));
    ::Ix_DbgRunState(id_grabber, id_camera, dwStateFail);
    m_siGrab3D_Error_LOG.m_dwDbgRunState = dwState;
    //

    //2023.06.13_MultGrabWait return val.
    m_siGrab3D_Error_LOG.m_dwMultGrabWaitReturnVal = dwRet;

    CString strMessage("");

    switch (dwRet)
    {
        case STATUS_GRAB_TIMEOUT:
            //strMessage.Format(_T("STATUS_GRAB_TIMEOUT ==> Grab count : %04d/%04d"), m_scanImageCount, GetScanLength_px());
            strMessage = _T("STATUS_GRAB_TIMEOUT");
            break;
        case STATUS_GRAB_ALREADY_RUNNING:
            //strMessage.Format(_T("STATUS_GRAB_ALREADY_RUNNING ==> Grab count : %04d/%04d"), m_scanImageCount, GetScanLength_px());
            strMessage = _T("STATUS_GRAB_ALREADY_RUNNING");
            break;
        case STATUS_GRAB_DMA_OVER_FAIL:
            //strMessage.Format(_T("STATUS_GRAB_DMA_OVER_FAIL ==> Grab count : %04d/%04d"), m_scanImageCount, GetScanLength_px());
            strMessage = _T("STATUS_GRAB_DMA_OVER_FAIL");
            break;
        case STATUS_GRAB_FAIL:
            //strMessage.Format(_T("STATUS_GRAB_FAIL ==> Grab count : %04d/%04d"), m_scanImageCount, GetScanLength_px());
            strMessage = _T("STATUS_GRAB_FAIL");
            break;
        case STATUS_GRAB_FIFO_OVERFLOW:
            //	strMessage.Format(_T("STATUS_GRAB_FIFO_OVERFLOW ==> Grab count : %d"), m_scanImageCount);
            strMessage = _T("STATUS_GRAB_FIFO_OVERFLOW");
            break;
        default:
            strMessage.Format(_T("%d"), dwRet);
            break;
    }
    m_strErrorMessage = strMessage;
    //::AfxMessageBox(strMessage);

    SetiGrab3D_Error_LOG();

    return false;
}

bool FrameGrabber3D_iGrabXE::wait_calc_end(
    const int& grabber_idx, const int& camera_idx, const bool& save_slit_images, Ipvm::AsyncProgress* progress)
{
    if (is_valid_index(grabber_idx, camera_idx) == false)
        return false;

    if (save_slit_images == true)
    {
        if (progress)
        {
            progress->Post(L"Scanning : Save raw images");
        }

        const unsigned int& width = get_grab_image_width(grabber_idx, camera_idx);
        const unsigned int& height = get_grab_image_height(grabber_idx, camera_idx);
        Save2DImage(width, height, m_resultStitchIndex);
    }

    if (progress)
    {
        progress->Post(_T("Wait for calculation end"));
    }

    auto coreResult = m_pCore->Wait();

    if (progress)
    {
        progress->Post(_T("Wait for post processing done"));
    }

    PostWaitCalcEndWork(); //kircheis_3DCalcRetry

    if (coreResult != Ipvm::Status::e_ok)
    {
        m_strErrorMessage = Ipvm::ToString(coreResult);
        SetiGrab3D_Error_LOG();
        return false;
    }

    return true;
}

bool FrameGrabber3D_iGrabXE::re_calc_cuda_3D(const int& grabber_idx, const int& camera_idx) //kircheis_3DCalcRetry
{
    DWORD nCount = 0;
    long nBufCnt = (long)m_scanBuffers.size();

    Sleep(300);
    m_zmapImage->FillZero();
    m_vmapImage->FillZero();
    auto coreResult = m_pCore->ProcessAsync(NULL, *m_zmapImage, *m_vmapImage); //kircheis_SDK127

    if (coreResult != Ipvm::Status::e_ok)
    {
        m_strErrorMessage = Ipvm::ToString(coreResult);
        SetiGrab3D_Error_LOG();
        return false;
    }

    for (long nIdx = 0; nIdx < nBufCnt; nIdx++)
    {
        nCount++;
        m_scanImageCount = (long)nCount;
        Ipvm::SbtCore::CallbackAfterFrameUpdated(nCount, m_pCore);
    }

    auto coreWaitResult = m_pCore->Wait();

    PostWaitCalcEndWork(); //kircheis_3DCalcRetry

    if (coreWaitResult != Ipvm::Status::e_ok)
    {
        m_strErrorMessage = Ipvm::ToString(coreWaitResult);
        SetiGrab3D_Error_LOG();
        return false;
    }

    return true;
}

void FrameGrabber3D_iGrabXE::PostWaitCalcEndWork() //kircheis_3DCalcRetry
{
    const int physicalScanWidth_px = m_zmapImage->GetSizeX();
    const int physicalScanLength_px = m_zmapImage->GetSizeY();

    const int logicalScanLength_px = m_resultZmapImage->GetSizeX();

    const int maxStitchCount = m_resultZmapImage->GetSizeY() / physicalScanWidth_px;

    const int stitchIndex = max(0, min(m_resultStitchIndex, maxStitchCount - 1));

    Ipvm::Rect32s imageRect(
        0, physicalScanWidth_px * stitchIndex, logicalScanLength_px, physicalScanWidth_px * (stitchIndex + 1));

    //const float heightRangeMin = -0.5f * m_pCore->GetHeightRange_um(*m_pCoreParam);
    //const float heightRangeMax = 0.5f * m_pCore->GetHeightRange_um(*m_pCoreParam);

    Ipvm::Image32r childResultZmapImage(*m_resultZmapImage, imageRect);
    Ipvm::Image16u childResultVmapImage(*m_resultVmapImage, imageRect);

    // ZMAP 회전, REISZE 및 DISTORTION 적용
    m_distortionData->applyAndRotateCw90_zmap(*m_zmapImage, childResultZmapImage);

    // VMAP 회전, RESIZE
    if (1)
    {
        if (physicalScanLength_px != logicalScanLength_px)
        {
            Ipvm::ImageProcessingGpu::RotateCw90(*m_vmapImage, 255.f, *m_vmapResizeTempImage);

            Ipvm::ImageProcessing::ResizeNearestInterpolation(*m_vmapResizeTempImage, childResultVmapImage);
        }
        else
        {
            Ipvm::ImageProcessingGpu::RotateCw90(*m_vmapImage, 255.f, childResultVmapImage);
        }
    }
}

bool FrameGrabber3D_iGrabXE::set_exposure_time(
    const int& grabber_idx, const int& camera_idx, const double& exposureHigh_ms, double& validFramePeriod_ms)
{
    // 아래 테이블은 뷰웍스 매뉴얼 참조함.
    //	Line	FPS
    Ipvm::Point32r2 data[] = {
        {500.f, 1000.f / 1782},
        {1000.f, 1000.f / 960},
        {1500.f, 1000.f / 657},
        {2000.f, 1000.f / 499},
        {2500.f, 1000.f / 403},
        {3072.f, 1000.f / 330},
    };

    Ipvm::LineEq32r lineEq;

    Ipvm::DataFitting::FitToLine(sizeof(data) / sizeof(Ipvm::Point32r2), data, lineEq);

    double test[] = {
        lineEq.GetPositionY(data[0].m_x),
        lineEq.GetPositionY(data[1].m_x),
        lineEq.GetPositionY(data[2].m_x),
        lineEq.GetPositionY(data[3].m_x),
        lineEq.GetPositionY(data[4].m_x),
        lineEq.GetPositionY(data[5].m_x),
    };

    const double minimumExposureTime_ms = 0.016;
    const double desirableExposureTime_ms
        = max(minimumExposureTime_ms, exposureHigh_ms * SystemConfig::GetInstance().m_slitbeamIlluminationGain);

    const double scanSpeedSlowDownRatio = SystemConfig::GetInstance().m_slitbeamScanSpeedDownRatio;
    const double defaultFramePeriod_ms
        = CAST_DOUBLE(lineEq.GetPositionY(CAST_FLOAT(get_grab_image_height(grabber_idx, camera_idx))));
    const double ceiledDefaultFramePeriod_ms = ::ceil(defaultFramePeriod_ms * 100 * scanSpeedSlowDownRatio)
        * 0.01; // 엔코더 떨림에 대비해서 10% 정도 더 마진을 준다.

    validFramePeriod_ms = max(ceiledDefaultFramePeriod_ms,
        desirableExposureTime_ms * scanSpeedSlowDownRatio); // 엔코더 떨림에 대비해서 10% 정도 더 마진을 준다.

    CSingleLock lock(&g_cs, TRUE);

    const int& id_grabber = get_grabber_id(grabber_idx);
    const int& id_camera = get_camera_id(id_grabber, camera_idx);
    const int& id_icf = get_camera_icf_id(id_grabber, id_camera);

    ::Ix_SetXCFExposureTime(id_grabber, id_camera, id_icf, float(desirableExposureTime_ms), 0.001f);

    return true;
}

bool FrameGrabber3D_iGrabXE::set_slitbeam_parameters(const int& grabber_idx, const int& camera_idx,
    const SlitBeam3DParameters& slitbeamPara, const float& height_scaling, const bool& use_height_quantization,
    const bool& use_distortion_compensation)
{
    Ipvm::AsyncProgress progress(_T("Setup slitbeam GPU core"));

    const int& id_grabber = get_grabber_id(grabber_idx);
    const int& id_camera = get_camera_id(grabber_idx, camera_idx);
    const int& id_icf = get_camera_icf_id(id_grabber, id_camera);

    const long newGrabImageHeight
        = get_grab_image_height_3D(slitbeamPara.m_scanDepth_um, slitbeamPara.m_cameraVerticalBinning, height_scaling);

    bool bIsSuccessedChangeVerticalSize = false; //kircheis_3DGrabIssue

    if (newGrabImageHeight != get_grab_image_height(grabber_idx, camera_idx)
        || m_use_binning[id_grabber][id_camera] != slitbeamPara.m_cameraVerticalBinning)
    {
        for (long i = 0; i < 3; i++) //kircheis_3DGrabIssue
        {
            if (ChangeVerticalSize(
                    id_grabber, id_camera, id_icf, slitbeamPara.m_cameraVerticalBinning, false, newGrabImageHeight))
            {
                bIsSuccessedChangeVerticalSize = true;
                break;
            }
        }

        set_grab_image_height(grabber_idx, camera_idx, newGrabImageHeight);
        m_use_binning[id_grabber][id_camera] = slitbeamPara.m_cameraVerticalBinning;
    }
    else
    {
        bIsSuccessedChangeVerticalSize = true;
    }

    SystemConfig::GetInstance().SetScale3D(
        get_height_resolution_px2um(slitbeamPara.m_cameraVerticalBinning, height_scaling));

    // 4 의 배수로 맞추어야 한다
    long physicalScanLength_pxTmp = long(slitbeamPara.m_scanLength_mm * 1000. / slitbeamPara.m_scanStep_um + 0.5);
    const long physicalScanLength_px = physicalScanLength_pxTmp - (physicalScanLength_pxTmp % 4);
    //const long frameSize = m_grabImageWidth * m_grabImageHeight;

    const int& width = get_grab_image_width(grabber_idx, camera_idx);
    const int& height = get_grab_image_height(grabber_idx, camera_idx);

    live_buffer_mono->Create(width, height);

    const long oldProfileNumber = m_profileNumber;

    m_scanBuffers.resize(max(physicalScanLength_px, m_scanBuffers.size()));

    for (long idx = 0; idx < physicalScanLength_px; idx++)
    {
        auto& image = m_scanBuffers[idx];

        if (image.GetSizeY() < width)
        {
            image.Create(width, height, true);
        }
    }

    if (physicalScanLength_px != oldProfileNumber)
    {
        m_zmapImage->Create(width, physicalScanLength_px, true);
        m_vmapImage->Create(width, physicalScanLength_px, true);
    }

    m_vmapResizeTempImage->Create(physicalScanLength_px, width);

    m_profileNumber = physicalScanLength_px;

    m_distortionData->ready(slitbeamPara.m_scanDepth_um, slitbeamPara.m_cameraVerticalBinning ? true : false,
        m_zmapImage->GetSizeY(), m_zmapImage->GetSizeX(), use_distortion_compensation);

    Ipvm::Image8u dsnuImage;

    if (m_use_binning[id_grabber][id_camera])
    {
        Ipvm::Rect partialRect(0, get_grab_image_offset_Y(grabber_idx, camera_idx),
            get_grab_image_width(grabber_idx, camera_idx),
            get_grab_image_offset_Y(grabber_idx, camera_idx) + get_grab_image_height(grabber_idx, camera_idx));

        dsnuImage = Ipvm::Image8u(dsnu_binning_images[id_grabber][id_camera], partialRect);
    }
    else
    {
        Ipvm::Rect partialRect(0, get_grab_image_offset_Y(grabber_idx, camera_idx),
            get_grab_image_width(grabber_idx, camera_idx),
            get_grab_image_offset_Y(grabber_idx, camera_idx) + get_grab_image_height(grabber_idx, camera_idx));

        dsnuImage = Ipvm::Image8u(dsnu_images[id_grabber][id_camera], partialRect);
    }

    m_pCoreParam->m_imageSizeX = get_grab_image_width(grabber_idx, camera_idx);
    m_pCoreParam->m_imageSizeY = get_grab_image_height(grabber_idx, camera_idx);
    m_pCoreParam->m_scanLength = physicalScanLength_px;
    m_pCoreParam->m_imageBuffers = m_scanBuffers.data();
    m_pCoreParam->m_imageBufferCount = physicalScanLength_px;
    m_pCoreParam->m_dsnuBuffer = &dsnuImage;
    m_pCoreParam->m_saturationIntensity = 255;
    m_pCoreParam->m_distortionProfile = m_distortionData->getDistortionProfile();
    m_pCoreParam->m_noiseThresholdPre_gv = slitbeamPara.m_noiseThreshold;
    m_pCoreParam->m_noiseThresholdFinal_gv
        = slitbeamPara.m_noiseThreshold_FinalFiltering; //0.5f * slitbeamPara.m_noiseThreshold;
    m_pCoreParam->m_beamThickness_um = slitbeamPara.m_beamThickness_um;
    m_pCoreParam->m_lateralResolution_um_per_px = SystemConfig::GetInstance().Get3DImageVerticalScale_px2um()
        * (slitbeamPara.m_cameraVerticalBinning ? 2.f : 1.f);
    m_pCoreParam->m_scanStep_um = slitbeamPara.m_scanStep_um;
    m_pCoreParam->m_illuminationAngle_deg = SystemConfig::GetInstance().m_slitbeamIlluminationAngle_deg;
    m_pCoreParam->m_cameraAngle_deg = SystemConfig::GetInstance().m_slitbeamCameraAngle_deg;
    m_pCoreParam->m_heightScaling = height_scaling;
    m_pCoreParam->m_useHeightValueQuantization = use_height_quantization;
    m_pCoreParam->m_heightValueQuantum_um = 0.1f;
    m_pCoreParam->m_useSpaceTimeSampling = true;
    m_pCoreParam->m_shearingCompensationMode = Ipvm::SbtCoreShearingCompensationMode::e_weightedAverage;
    m_pCoreParam->m_shearingCompensationDiscardLevel_um = 100.f;
    //m_pCoreParam->m_reversalOptics = SystemConfig::GetInstance().m_slitbeamOpticsReversal ? true : false;
    m_pCoreParam->m_verticalFlippedImage = SystemConfig::GetInstance().m_slitbeamCameraRotation ? false : true;
    m_pCoreParam->m_spaceTimeSamplingScaleFactor = SystemConfig::GetInstance().Get_SpaceTimeSamplingScaleFactor();
    m_pCoreParam->m_forwardScan = false;
    m_pCoreParam->m_noiseValue = Ipvm::k_noiseValue32r;

    auto coreResult = m_pCore->Setup(*m_pCoreParam);

    if (coreResult != Ipvm::Status::e_ok)
    {
        ::AfxMessageBox(CString(Ipvm::ToString(coreResult)));
        return true; //kircheis_3DGrabIssue
    }

    return bIsSuccessedChangeVerticalSize; //kircheis_3DGrabIssue
}

void FrameGrabber3D_iGrabXE::set_live_image_size(
    const int& grabber_idx, const int& camera_idx, const bool& use_binning, const bool& is_full_size)
{
    const int& id_grabber = get_grabber_id(grabber_idx);
    const int& id_camera = get_camera_id(id_grabber, camera_idx);
    const int& id_icf = get_camera_icf_id(id_grabber, id_camera);

    ChangeVerticalSize(
        id_grabber, id_camera, id_icf, use_binning, is_full_size, get_grab_image_height(grabber_idx, camera_idx));
}

long FrameGrabber3D_iGrabXE::get_grab_image_height_3D(
    const double& scan_depth_um, const bool& use_binning, const double& height_scaling) const
{
    const float heightResolution_px2um = get_height_resolution_px2um(use_binning, CAST_FLOAT(height_scaling), true);

    const long grabImageHeight = CAST_LONG(::ceil(scan_depth_um / heightResolution_px2um));

    const long validGrabImageHeight = Ipvm::Image::CalcWidthBytes(grabImageHeight, 8, 4);

    return validGrabImageHeight;
}

bool FrameGrabber3D_iGrabXE::load_dsnu_image(
    const CString& camera_serial, Ipvm::Image8u& dsum_img, Ipvm::Image8u& dsum_binning_img)
{
    Ipvm::Image8u image;

    CString pathName;
    pathName.Format(
        _T("%sDSNU_IMAGE_%s.bmp"), LPCTSTR(DynamicSystemPath::get(DefineFolder::System)), LPCTSTR(camera_serial));

    auto ret = Ipvm::ImageFile::Load(LPCTSTR(pathName), image);

    if (ret != Ipvm::Status::e_ok)
    {
        CString strError;
        strError.Format(_T("Cannot open \"%s\" file"), LPCTSTR(pathName));
        ::AfxMessageBox(strError, MB_ICONERROR | MB_OK);
        return false;
    }

    if (image.GetSizeX() != dsum_img.GetSizeX() || image.GetSizeY() != dsum_img.GetSizeY())
    {
        ::AfxMessageBox(_T("DSNU Image size mismatch!"), MB_ICONERROR | MB_OK);
        return false;
    }

    dsum_img = image;

    for (long y = 0; y < DEFAULT_IMAGE_FULL_SIZE_Y / 2; y++)
    {
        memcpy(dsum_binning_img.GetMem(0, y), dsum_img.GetMem(0, 2 * y), DEFAULT_IMAGE_SIZE_X);
    }

    return true;
}

bool FrameGrabber3D_iGrabXE::make_dsnu_image(const int& grabber_id, const int& camera_id, const int& icf_id,
    int& custom_grab_count, Ipvm::Image8u& dsnu_image, Ipvm::Image8u& dsnu_binning_image, const CString& camera_serial)
{
    Ipvm::AsyncProgress progress(_T("Make DSNU Image"));

    ChangeVerticalSize(grabber_id, camera_id, icf_id, false, true, 0);

    ::Ix_SetCustomGrabEvent(grabber_id, camera_id, 1);
    float exposureHigh_ms = 0.f;
    float exposureLow_ms = 0.f;
    ::Ix_GetXCFExposureTime(grabber_id, camera_id, icf_id, exposureHigh_ms, exposureLow_ms);
    ::Ix_SetXCFExposureTime(grabber_id, camera_id, icf_id, exposureHigh_ms, 90.f);
    ::Ix_SetXCFParam(grabber_id, camera_id, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::UplinkTrigger);
    ::Ix_SetXCFParam(grabber_id, camera_id, icf_id, XCF_CTL_TRIGGER_TYPE, _enumTriggerType::FG_Trigger);

    Ipvm::Image32s sumImage(DEFAULT_IMAGE_SIZE_X, DEFAULT_IMAGE_FULL_SIZE_Y);
    sumImage.FillZero();

    Ipvm::Image8u grabImage(DEFAULT_IMAGE_SIZE_X, DEFAULT_IMAGE_FULL_SIZE_Y);

    const long repeatCount = 1000;
    bool success = true;

    for (long idx = 0; idx < repeatCount; idx++)
    {
        CString str;
        str.Format(_T("Make DSNU Image (%d/%d)"), idx + 1, repeatCount);

        progress.Post(str);

        BYTE* address = grabImage.GetMem();

        if (STATUS_GRAB_SUCCESS
            != ::Ix_MultiGrabProcess(grabber_id, camera_id, &address, 1, 1, MtgWaitOp::MTG_END_WAIT, 1000))
        {
            ::AfxMessageBox(_T("Cannot grab image"), MB_ICONERROR | MB_OK);
            success = false;
            break;
        }

        for (long y = 0; y < DEFAULT_IMAGE_FULL_SIZE_Y; y++)
        {
            const auto* grabMem = grabImage.GetMem(0, y);
            auto* sumMem = sumImage.GetMem(0, y);

            for (long x = 0; x < DEFAULT_IMAGE_SIZE_X; x++)
            {
                sumMem[x] += grabMem[x];
            }
        }
    }

    int nCustEvtNum = 0;
    ::Ix_GetCustomGrabEvent(grabber_id, camera_id, &nCustEvtNum, &custom_grab_count);
    ::Ix_SetCustomGrabEvent(grabber_id, camera_id, custom_grab_count);
    ::Ix_SetXCFParam(grabber_id, camera_id, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);
    ::Ix_SetXCFParam(grabber_id, camera_id, icf_id, XCF_CTL_TRIGGER_TYPE, _enumTriggerType::HardwareTrigger);
    ::Ix_SetXCFExposureTime(grabber_id, camera_id, icf_id, exposureHigh_ms, exposureLow_ms);

    if (success == false)
    {
        return false;
    }

    unsigned long histogram[256];
    memset(histogram, 0, sizeof(histogram));

    for (long y = 0; y < DEFAULT_IMAGE_FULL_SIZE_Y; y++)
    {
        const auto* sumMem = sumImage.GetMem(0, y);
        auto* dsnuMem = dsnu_image.GetMem(0, y);

        for (long x = 0; x < DEFAULT_IMAGE_SIZE_X; x++)
        {
            dsnuMem[x] = BYTE(sumMem[x] / float(repeatCount) + 1.f); // 올림 계산

            histogram[dsnuMem[x]]++;
        }
    }

    if (1)
    {
        typedef std::basic_string<char>::size_type temp_string_size_type;

        std::ofstream outfile(DynamicSystemPath::get(DefineFolder::Temp) + _T("DsnuHistogram.csv"));

        if (outfile.is_open())
        {
            for (long idx = 0; idx < 256; idx++)
            {
                outfile << histogram[idx] << std::endl;
            }
        }
    }

    long brightPixelCount = 0;

    for (long idx = 20; idx < 256; idx++)
    {
        brightPixelCount += histogram[idx];
    }

    if (brightPixelCount > 10)
    {
        ::AfxMessageBox(_T("Too many bright pixels. Retry please."), MB_ICONERROR | MB_OK);
        ::exit(0);
    }

    CString pathName;
    pathName.Format(
        _T("%sDSNU_IMAGE_%s.bmp"), LPCTSTR(DynamicSystemPath::get(DefineFolder::System)), LPCTSTR(camera_serial));

    Ipvm::ImageFile::SaveAsBmp(dsnu_image, pathName);

    for (long y = 0; y < DEFAULT_IMAGE_FULL_SIZE_Y / 2; y++)
    {
        memcpy(dsnu_binning_image.GetMem(0, y), dsnu_image.GetMem(0, 2 * y), DEFAULT_IMAGE_SIZE_X);
    }

    return true;
}

bool FrameGrabber3D_iGrabXE::ChangeVerticalSize(const int& grabber_id, const int& camera_id, const int& icf_id,
    const bool cameraVerticalBinning, const bool fullSizeY, const long scanSizeY)
{
    Ipvm::AsyncProgress progress(_T("Modify camera parameters"));

    int nCustEvtNum(0);
    ::Ix_SetCustomGrabEvent(grabber_id, camera_id, 1);
    ::Ix_GetCustomGrabEvent(grabber_id, camera_id, &nCustEvtNum, &custom_grab_event_counts[grabber_id][camera_id]);

    UINT uiImageFullSizeY = DEFAULT_IMAGE_FULL_SIZE_Y / (cameraVerticalBinning ? 2 : 1);
    set_grab_image_height(grabber_id, camera_id, fullSizeY ? uiImageFullSizeY : scanSizeY);
    m_use_binning[grabber_id][camera_id] = cameraVerticalBinning;

    // Camera Open...
    HANDLE handle = ::ctrlOpen(camera_id);

    // Register address
    ULONG ulUserBaseAddress = 0x10000000;
    ULONG ulIpBaseAddress = ulUserBaseAddress + 0x00010000;
    ULONG ulAddressHeight = ulIpBaseAddress + 0x00000044;
    //ULONG ulAddressMaxHeight = ulIpBaseAddress + 0x00000544;
    ULONG ulAddressOffsetY = ulIpBaseAddress + 0x000000C4;
    ULONG ulAddressBinningY = ulIpBaseAddress + 0x0000001C4;
    ULONG ulTrigBaseAddress = ulUserBaseAddress + 0x00020000;
    ULONG ulAddressAcqStart = ulTrigBaseAddress + 0x00001084;
    ULONG ulAddressAcqStop = ulTrigBaseAddress + 0x000010C4;

    // Communication Status Check
    long nTrue = 1;
    long nFalse = 0;

    BYTE ret = 0;
    DWORD dwTimeOutMS = cameraVerticalBinning ? 4000 : 1000;
    // Acq Stop
    ret = ::ctrlCmdMsg(handle, enumMemoryWrite, sizeof(long), ulAddressAcqStart, (BYTE*)&nFalse, 1000);
    Sleep(10);
    ret = ::ctrlCmdMsg(handle, enumMemoryWrite, sizeof(long), ulAddressAcqStop, (BYTE*)&nTrue, 1000);

    // Write binning
    UINT uiBinning = cameraVerticalBinning ? 2 : 1;
    ret = ::ctrlCmdMsg(handle, enumMemoryWrite, sizeof(uiBinning), ulAddressBinningY, (BYTE*)&uiBinning, dwTimeOutMS);

    Sleep(10);

    if (fullSizeY == true)
    {
        // Write offset
        int change_offset_Y = 0;
        set_grab_image_offset_Y(grabber_id, camera_id, change_offset_Y);

        ret = ::ctrlCmdMsg(handle, enumMemoryWrite, sizeof(get_grab_image_offset_Y(grabber_id, camera_id)),
            ulAddressOffsetY, (BYTE*)&change_offset_Y, dwTimeOutMS);

        set_grab_image_offset_Y(grabber_id, camera_id, change_offset_Y);

        Sleep(10);
        // Write height
        auto height = get_grab_image_height(grabber_id, camera_id);
        ret = ::ctrlCmdMsg(handle, enumMemoryWrite, sizeof(get_grab_image_height(grabber_id, camera_id)),
            ulAddressHeight, (BYTE*)&height, dwTimeOutMS);

        set_grab_image_height(grabber_id, camera_id, height);
    }
    else
    {
        // Write height
        auto height = get_grab_image_height(grabber_id, camera_id);
        //long nCurImageHeight = m_grabImageHeight;
        ret = ::ctrlCmdMsg(handle, enumMemoryWrite, sizeof(long), ulAddressHeight, (BYTE*)&height, dwTimeOutMS);
        if (CAST_UINT(get_grab_image_height(grabber_id, camera_id)) != uiImageFullSizeY
            && ret != STATUS_CTRLCMD_SUCCESS) //kircheis_3DGrabIssue
            return false;

        set_grab_image_height(grabber_id, camera_id, height);

        // Write offset
        int iImageOffsetY = SystemConfig::GetInstance().m_slitbeamCameraOffsetY / (cameraVerticalBinning ? 2 : 1);
        Sleep(10);
        set_grab_image_offset_Y(grabber_id, camera_id,
            (uiImageFullSizeY - get_grab_image_height(grabber_id, camera_id)) / 2 + iImageOffsetY);

        if (CAST_UINT(get_grab_image_height(grabber_id, camera_id) + get_grab_image_offset_Y(grabber_id, camera_id))
            >= uiImageFullSizeY)
            set_grab_image_offset_Y(grabber_id, camera_id, 0);

        auto offset_Y = get_grab_image_offset_Y(grabber_id, camera_id);
        ret = ::ctrlCmdMsg(handle, enumMemoryWrite, sizeof(offset_Y), ulAddressOffsetY, (BYTE*)&offset_Y, dwTimeOutMS);
    }

    // Acq Start
    ret = ::ctrlCmdMsg(handle, enumMemoryWrite, sizeof(long), ulAddressAcqStop, (BYTE*)&nFalse, 1000);
    Sleep(10);
    ret = ::ctrlCmdMsg(handle, enumMemoryWrite, sizeof(long), ulAddressAcqStart, (BYTE*)&nTrue, 1000);

    // Close Device...
    ret = CAST_BYTE(::ctrlClose(handle));

    // 이미지 사이즈 설정
    ::Ix_SetCustomGrabEvent(grabber_id, camera_id, 1);
    ::Ix_SetXCFParam(grabber_id, camera_id, icf_id, XCF_CTL_VERTICAL_SIZE, get_grab_image_height(camera_id));
    //{{//kircheis_3DGrabIssue
    unsigned int nGrabImageHeight = 0;
    Sleep(10);
    ::Ix_GetXCFParam(grabber_id, camera_id, icf_id, XCF_CTL_VERTICAL_SIZE, &nGrabImageHeight);
    if (CAST_UINT(get_grab_image_height(grabber_id, camera_id)) != nGrabImageHeight)
        return false;
    //}}

    ::Ix_GetCustomGrabEvent(grabber_id, camera_id, &nCustEvtNum, &custom_grab_event_counts[grabber_id][camera_id]);
    ::Ix_SetCustomGrabEvent(grabber_id, camera_id, custom_grab_event_counts[grabber_id][camera_id]);

    live_buffer_mono->Create(get_grab_image_width(grabber_id, camera_id), get_grab_image_height(grabber_id, camera_id));

    return true;
}

void FrameGrabber3D_iGrabXE::ScanEnd(void* pUserData, DWORD nCount, void* pParam1, void* pParam2)
{
    UNREFERENCED_PARAMETER(pParam2);

    ((FrameGrabber3D_iGrabXE*)pUserData)->ScanEnd(nCount, (BYTE*)pParam1);
}

void FrameGrabber3D_iGrabXE::ScanEnd(DWORD nCount, BYTE* pDmaBuffer)
{
    //2023.02.23_DEV. Debug Log
    g_cb_call_count++;
    g_cb_sum_count = (int)max(g_cb_sum_count, nCount);

    m_siGrab3D_Error_LOG.m_cb_call_count = g_cb_call_count;
    m_siGrab3D_Error_LOG.m_cb_sum_count = g_cb_sum_count;
    //

    BYTE* pCpyImageBuffer = m_scanBuffers[nCount - 1].GetMem();

    const long frameSize = get_grab_image_width(grabber_id_use_grab, camera_id_use_grab)
        * get_grab_image_height(grabber_id_use_grab, camera_id_use_grab);

    memcpy(pCpyImageBuffer, pDmaBuffer, frameSize);

    m_scanImageCount = nCount;

    Ipvm::SbtCore::CallbackAfterFrameUpdated(nCount, m_pCore);
}

void FrameGrabber3D_iGrabXE::LiveEnd(void* pUserData, DWORD nCount, void* pParam1, void* pParam2)
{
    UNREFERENCED_PARAMETER(pParam2);

    ((FrameGrabber3D_iGrabXE*)pUserData)->LiveEnd(nCount, (BYTE*)pParam1);
}

void FrameGrabber3D_iGrabXE::LiveEnd(DWORD nCount, BYTE* pDmaBuffer)
{
    if (nCount > 1)
    {
        return;
    }

    memcpy(live_buffer_mono->GetMem(), pDmaBuffer, live_buffer_mono->GetWidthBytes() * live_buffer_mono->GetSizeY());
}

struct ThreadArgs
{
    LPCTSTR m_szDirectory;
    long m_width;
    long m_height;
    Ipvm::Image8u* m_imageBuffers;
    long m_threadIndex;
    long m_threadCount;
    long m_totalImageCount;
};

static void ImageSaveThread(ThreadArgs args)
{
    CString strFile;

    for (long idx = 0; idx < args.m_totalImageCount; idx++)
    {
        if (idx % args.m_threadCount != args.m_threadIndex)
        {
            continue;
        }

        strFile.Format(_T("%s\\%05d.png"), args.m_szDirectory, idx);

        Ipvm::ImageFile::SaveAsPng(args.m_imageBuffers[idx], strFile);
    }
}

void FrameGrabber3D_iGrabXE::Save2DImage(
    const unsigned int& image_width, const unsigned int& image_height, const long splitSetIndex)
{
    CString strDirectory;

    strDirectory.Format(
        _T("%sSlitbeamScanImage_%01d\\"), LPCTSTR(DynamicSystemPath::get(DefineFolder::Image)), splitSetIndex);

    ::CreateDirectory(strDirectory, NULL);

    strDirectory += CTime::GetCurrentTime().Format("%Y%m%d_%H%M%S");

    ::CreateDirectory(strDirectory, NULL);

    static const long threadCount = 6;

    ThreadArgs args = {LPCTSTR(strDirectory), CAST_LONG(image_width), CAST_LONG(image_height), m_scanBuffers.data(), 0,
        threadCount, m_profileNumber};

    std::vector<std::thread> threads;

    for (; args.m_threadIndex < threadCount; args.m_threadIndex++)
    {
        threads.emplace_back(ImageSaveThread, args);
    }

    for (auto& thread : threads)
    {
        thread.join();
    }
}

bool FrameGrabber3D_iGrabXE::LiveCallback(const int& graaber_id, const int& camera_id)
{
    CSingleLock lock(&g_cs, TRUE);

    ::Ix_SetCustomGrabEvent(graaber_id, camera_id, 1);
    float exposureHigh_ms = 0.f;
    float exposureLow_ms = 0.f;
    const int& icf_id = get_camera_icf_id(graaber_id, camera_id);
    ::Ix_GetXCFExposureTime(graaber_id, camera_id, icf_id, exposureHigh_ms, exposureLow_ms);
    ::Ix_SetXCFExposureTime(graaber_id, camera_id, icf_id, exposureHigh_ms, 90.f);
    ::Ix_SetXCFParam(graaber_id, camera_id, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::UplinkTrigger);
    ::Ix_SetXCFParam(graaber_id, camera_id, icf_id, XCF_CTL_TRIGGER_TYPE, _enumTriggerType::FG_Trigger);

    auto ret1
        = ::Ix_MultiGrabProcess(graaber_id, camera_id, nullptr, 0, 1, MtgWaitOp::MTG_GRB_ISSUE, 1000, LiveEnd, this);

    if (STATUS_GRAB_START_SUCCESS == ret1)
    {
        auto ret2 = ::Ix_MultiGrabWait(graaber_id, camera_id);

        int nCustEvtNum = 0;
        ::Ix_GetCustomGrabEvent(graaber_id, camera_id, &nCustEvtNum, &custom_grab_event_counts[graaber_id][camera_id]);
        ::Ix_SetCustomGrabEvent(graaber_id, camera_id, custom_grab_event_counts[graaber_id][camera_id]);
        ::Ix_SetXCFParam(graaber_id, camera_id, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);
        ::Ix_SetXCFParam(graaber_id, camera_id, icf_id, XCF_CTL_TRIGGER_TYPE, _enumTriggerType::HardwareTrigger);
        ::Ix_SetXCFExposureTime(graaber_id, camera_id, icf_id, exposureHigh_ms, exposureLow_ms);

        const int offset_Y = get_grab_image_offset_Y(graaber_id, camera_id);
        const int image_height = get_grab_image_height(graaber_id, camera_id);

        if (m_use_binning[graaber_id][camera_id] == true)
        {
            Ipvm::Image8u roiImage(dsnu_binning_images[graaber_id][camera_id],
                Ipvm::Rect32s(0, offset_Y, DEFAULT_IMAGE_SIZE_X, offset_Y + image_height));

            Ipvm::ImageProcessing::Subtract(roiImage, Ipvm::Rect32s(roiImage), 0, *live_buffer_mono);
        }
        else
        {
            Ipvm::Image8u roiImage(dsnu_images[graaber_id][camera_id],
                Ipvm::Rect32s(0, offset_Y, DEFAULT_IMAGE_SIZE_X, offset_Y + image_height));

            Ipvm::ImageProcessing::Subtract(roiImage, Ipvm::Rect32s(roiImage), 0, *live_buffer_mono);
        }

        return STATUS_GRAB_SUCCESS == ret2;
    }
    else
    {
        ::Ix_GrabStop(graaber_id, camera_id);

        int nCustEvtNum = 0;
        ::Ix_GetCustomGrabEvent(graaber_id, camera_id, &nCustEvtNum, &custom_grab_event_counts[graaber_id][camera_id]);
        ::Ix_SetCustomGrabEvent(graaber_id, camera_id, custom_grab_event_counts[graaber_id][camera_id]);
        ::Ix_SetXCFParam(graaber_id, camera_id, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);
        ::Ix_SetXCFParam(graaber_id, camera_id, icf_id, XCF_CTL_TRIGGER_TYPE, _enumTriggerType::HardwareTrigger);
        ::Ix_SetXCFExposureTime(graaber_id, camera_id, icf_id, exposureHigh_ms, exposureLow_ms);

        return false;
    }

    return true;
}

float FrameGrabber3D_iGrabXE::get_height_resolution_px2um(
    const bool& use_binning, const float& height_scaling, const bool& use_ideal_motor_speed) const
{
    UNREFERENCED_PARAMETER(use_ideal_motor_speed);

    const BOOL bUseIdealMotorSpeed = true; // bIsForGrabHeight;//kircheis_IdealMotor
    const float illuminationAngle_deg = SystemConfig::GetInstance().m_slitbeamIlluminationAngle_deg;
    const float cameraAngle_deg = SystemConfig::GetInstance().m_slitbeamCameraAngle_deg;
    float fIdealScaleY = (bUseIdealMotorSpeed ? SystemConfig::GetInstance().m_slitbeamImage_px2um_y
                                              : SystemConfig::GetInstance().Get2DScaleY(
                                                    enSideVisionModule::SIDE_VISIONMODULE_FRONT)); //kircheis_IdealMotor
    const float lateralResolution_px2um = fIdealScaleY * (use_binning ? 2.f : 1.f); //kircheis_IdealMotor

    const float idealHeightResolution_px2um = Ipvm::SbtCore::GetIdealHeightResolution_px2um(
        illuminationAngle_deg, cameraAngle_deg, lateralResolution_px2um);

    float fHeightScale = (bUseIdealMotorSpeed ? 1.f : height_scaling); //kircheis_IdealMotor
    const float heightResolution_px2um = fHeightScale * idealHeightResolution_px2um; //kircheis_IdealMotor

    return heightResolution_px2um;
}

long FrameGrabber3D_iGrabXE::get_scan_length_px() const
{
    return m_profileNumber;
}

long FrameGrabber3D_iGrabXE::get_scan_count() const
{
    return m_scanImageCount;
}

void FrameGrabber3D_iGrabXE::init_slit_beam_distortion()
{
    m_distortionData->reloadProfile();
    m_distortionData->resetSelectDistortionFull();
}

CString FrameGrabber3D_iGrabXE::Get3DGrabErrorMessage()
{
    CString strReturn = m_strErrorMessage;
    m_strErrorMessage.Empty();
    return strReturn;
}

void FrameGrabber3D_iGrabXE::SetiGrab3D_Error_LOG()
{
    m_siGrab3D_Error_LOG.m_strPacketError = GetCxpLinkfPacketError(grabber_id_use_grab, camera_id_use_grab);
    m_siGrab3D_Error_LOG.m_strCxpLinkCableError = GetCurrentEnable_Channel_Connection_State(grabber_id_use_grab);
    m_siGrab3D_Error_LOG.m_nExternalTrigger = GetReciveExternalTrigger(grabber_id_use_grab, camera_id_use_grab);
    m_siGrab3D_Error_LOG.m_strPageTrigger = GetPageTrigger_State(grabber_id_use_grab, camera_id_use_grab);
    m_siGrab3D_Error_LOG.m_nReciveFrametoCamera = GetReciveFrameCount2Camera(grabber_id_use_grab, camera_id_use_grab);
    m_siGrab3D_Error_LOG.m_nCustomGrabEventCount = custom_grab_event_counts[grabber_id_use_grab][camera_id_use_grab];
}

bool FrameGrabber3D_iGrabXE::Check3DNoise(const unsigned int& grabber_id, const unsigned int& camera_id, BYTE** ppbyImg,
    Ipvm::Image8u& o_image, bool bFullSizeInsp)
{
    const int& id_icf = get_camera_icf_id(grabber_id, camera_id);
    //  SDY 카메라 상태 테스트를 위해 3D 카메라의 2D Grab을 진행하여 노이즈 발생을 확인
    // Threshold low보다 평균 밝기가 높은 라인이 Threshold spec low의 개수보다 많을 경우 ERROR 발생 High의 경우에도 동일

    /*********************************************************/
    // 검사에 사용될 Threshold 값 설정
    const long nThresholdLow = 20;
    const long nThresholdHigh = 100;

    //const long nThresholdSpecLow = 10;
    //const long nThresholdSpecHigh = 2;
    /*********************************************************/

    /*********************************************************/
    // 검사 시간 테스트 진행
    /*unsigned __int64 t0, t1, diff;

	t0 = GetTickCount64();*/
    /*********************************************************/

    //long nImageSizeX = DEFAULT_IMAGE_SIZE_X;
    long nImageSizeY{};

    // 해당 Grab hieight가 크기 변경하면 바로 초기화된다. 원복을 위해 하나 추가
    long nOriginalGrabHeight = get_grab_image_height(camera_id);

    if (bFullSizeInsp)
    {
        // Vertical size를 모든 Frame을 찍도록 수정한다.
        nImageSizeY = DEFAULT_IMAGE_FULL_SIZE_Y;
        ChangeVerticalSize(grabber_id, camera_id, id_icf, false, true, 0);
    }
    else
    {
        // Full size 검사가 아닐 경우에는 기존 사이즈를 그대로 사용한다.
        // Lot을 진행할 경우에는 해당 영역에 대한 문제만을 확인하여 테스트를 진행 -> 실제 데이터에 영향이 없으므로 문제가 되지 않는다.
        nImageSizeY = get_grab_image_height(camera_id);
    }

    // 카메라 Param 수정
    ::Ix_SetCustomGrabEvent(grabber_id, camera_id, 1);
    float exposureHigh_ms = 0.f;
    float exposureLow_ms = 0.f;
    ::Ix_GetXCFExposureTime(grabber_id, camera_id, id_icf, exposureHigh_ms, exposureLow_ms);
    ::Ix_SetXCFExposureTime(grabber_id, camera_id, id_icf, 0.016f, 0.016f);
    ::Ix_SetXCFParam(grabber_id, camera_id, id_icf, XCF_CTL_TRIGGER_MODE, _enumTriggermode::UplinkTrigger);
    ::Ix_SetXCFParam(grabber_id, camera_id, id_icf, XCF_CTL_TRIGGER_TYPE, _enumTriggerType::FG_Trigger);

    // Image grab 진행
    Ipvm::Image8u grabImage(DEFAULT_IMAGE_SIZE_X, nImageSizeY);

    bool success = true;

    *ppbyImg = grabImage.GetMem();
    o_image = grabImage;

    if (STATUS_GRAB_SUCCESS
        != ::Ix_MultiGrabProcess(grabber_id, camera_id, ppbyImg, 1, 1, MtgWaitOp::MTG_END_WAIT, 1000))
    {
        ::AfxMessageBox(_T("Cannot grab image"), MB_ICONERROR | MB_OK);
        success = false;

        return false;
    }

    // 카메라 Param 수정
    int nCustEvtNum = 0;
    ::Ix_GetCustomGrabEvent(grabber_id, camera_id, &nCustEvtNum, &custom_grab_event_counts[grabber_id][camera_id]);
    ::Ix_SetCustomGrabEvent(grabber_id, camera_id, custom_grab_event_counts[grabber_id][camera_id]);
    ::Ix_SetXCFParam(grabber_id, camera_id, id_icf, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);
    ::Ix_SetXCFParam(grabber_id, camera_id, id_icf, XCF_CTL_TRIGGER_TYPE, _enumTriggerType::HardwareTrigger);
    ::Ix_SetXCFExposureTime(grabber_id, camera_id, id_icf, exposureHigh_ms, exposureLow_ms);

    if (bFullSizeInsp)
    {
        // Vertical size 원복
        set_grab_image_height(camera_id, nOriginalGrabHeight);
        ChangeVerticalSize(grabber_id, camera_id, id_icf, false, false, get_grab_image_height(camera_id));
    }
    else
    {
        // 아닐 경우에는 해당 데이터를 변환하지 않아 원복이 필요가 없다
    }

    // Grab fail의 경우 False를 return하여 검사에 실패했음을 통지
    if (success == false)
    {
        return false;
    }

    // 이미지 저장

    /*CString pathName;
	pathName.Format(_T("%sScan2DImage_%s.bmp"),
		LPCTSTR(DynamicSystemPath::get(DefineFolder::Temp)),
		_T("Grab2DImage.bmp"));

	grabImage.SaveBmp(pathName);*/

    // 벡터 형식으로 모든 데이터를 넣는다.
    std::vector<std::vector<long>> vec2nXaxisGV;

    for (long y = 0; y < nImageSizeY; y++)
    {
        std::vector<long> vecnYaxisGV;
        const auto* grabMem = grabImage.GetMem(0, y);

        for (long x = 0; x < DEFAULT_IMAGE_SIZE_X; x++)
        {
            vecnYaxisGV.push_back(grabMem[x]);
        }
        vec2nXaxisGV.push_back(vecnYaxisGV);
    }

    // 원본 데이터 수집용 csv 제작

    /*CString strFileName;
	strFileName.Format(DynamicSystemPath::get(DefineFolder::Temp) + _T("3DVisionXAxisGV.csv"));
	FILE* fp = NULL;
	_tfopen_s(&fp, strFileName, _T("a"));

	if (fp != NULL)
	{
		CString strTemp;

		for (long idxY = 0; idxY < vec2nXaxisGV.size(); idxY++)
		{
			for (long idxX = 0; idxX < vec2nXaxisGV[idxY].size(); idxX++)
			{
				strTemp.Format(_T("%d"), vec2nXaxisGV[idxY][idxX]);
				fprintf(fp, "%s,", (LPCSTR)CStringA(strTemp));
			}

			fprintf(fp, "\n");
		}

		fclose(fp);
	}*/

    // 검사 알고리즘

    long nThresholdLowCount = 0;
    long nThresholdHighCount = 0;

    // Y 축으로 평균내서 데이터 연산
    for (long idxY = 0; idxY < vec2nXaxisGV.size(); idxY++)
    {
        long nYaxisSum = 0;

        for (long idxX = 0; idxX < vec2nXaxisGV[idxY].size(); idxX++)
        {
            nYaxisSum += vec2nXaxisGV[idxY][idxX];
        }

        long nYaxisAVG = CAST_LONG(nYaxisSum / vec2nXaxisGV[idxY].size());
        if (nYaxisAVG > nThresholdLow)
        {
            nThresholdLowCount++;

            if (nYaxisAVG > nThresholdHigh)
            {
                nThresholdHighCount++;
            }
        }
    }

    // X 축으로 평균내서 데이터 연산
    for (long idxX = 0; idxX < vec2nXaxisGV[0].size(); idxX++)
    {
        long nXaxisSum = 0;

        for (long idxY = 0; idxY < vec2nXaxisGV.size(); idxY++)
        {
            nXaxisSum += vec2nXaxisGV[idxY][idxX];
        }

        long nXaxisAVG = CAST_LONG(nXaxisSum / vec2nXaxisGV.size());
        if (nXaxisAVG > nThresholdLow)
        {
            nThresholdLowCount++;

            if (nXaxisAVG > nThresholdHigh)
            {
                nThresholdHighCount++;
            }
        }
    }

    /*********************************************************/
    // 검사 시간 테스트 진행
    /*t1 = GetTickCount64();

	diff = t1 - t0;
	CString strTimeResult;
	strTimeResult.Format(_T("검사 소요시간 : %d"), diff);
	AfxMessageBox(strTimeResult);*/
    /*********************************************************/

    // 검사 결과 판정
    if (nThresholdLowCount > 10 || nThresholdHighCount > 2)
    {
        return false;
    }

    return true;
}

CString FrameGrabber3D_iGrabXE::GetCxpLinkfPacketError(const unsigned int& grabber_id, const unsigned int& camera_id)
{
    CString strPacketState("");
    BYTE byEnableChannel_Count(0x00), byEnableChannel_PacketState(0x00);
    if (::Ix_GetCxpLinkState(grabber_id, camera_id, byEnableChannel_Count, byEnableChannel_PacketState)
        == FALSE) //Cxp Link Packet Error
        return _T("Empty");

    //mc_16진수를 2진수 표기법으로 생각하면 된다
    //EX)
    //Value 1: Error, Value 0: NonError
    //1번 Packet Error 0000 0001
    //2번 Packet Error 0000 0010
    //3번 Packet Error 0000 0100
    //4번 Packet Error 0000 1000
    //5번 Packet Error 0001 0000
    //6번 Packet Error 0010 0000
    //7번 Packet Error 0100 0000
    //8번 Packet Error 1000 0000

    const long nByteBitLength(8);

    CString strCurPacketState("");
    for (long nBitidx = nByteBitLength - 1; nBitidx >= 0; nBitidx--)
    {
        CString strBitValue("");
        strBitValue.Format(_T("%d"), (byEnableChannel_PacketState >> nBitidx) & TRUE);
        strCurPacketState.Append(strBitValue);
    }

    strPacketState.AppendFormat(_T("0b%s"), (LPCTSTR)strCurPacketState);

    return strPacketState;
}

CString FrameGrabber3D_iGrabXE::GetCurrentEnable_Channel_Connection_State(const unsigned int& grabber_id)
{
    //mc_16진수를 2진수 표기법으로 생각하면 된다
    //EX)
    //Value 1: Error, Value 0: NonError
    //1번 Packet Error 0000 0001
    //2번 Packet Error 0000 0010
    //3번 Packet Error 0000 0100
    //4번 Packet Error 0000 1000
    //5번 Packet Error 0001 0000
    //6번 Packet Error 0010 0000
    //7번 Packet Error 0100 0000
    //8번 Packet Error 1000 0000

    CString str_EnableChannelConnectionState("");
    BYTE byCableNum(0x00), byChannelState(0x00);

    if (::Ix_CableMonState(grabber_id, byCableNum, byChannelState) == FALSE)
        return _T("Empty");

    m_siGrab3D_Error_LOG.m_byCxpLinkCableState = byChannelState; //2023.04.26_Cable State return val.

    const long nByteBitLength(8);

    CString strCurPacketState("");
    for (long nBitidx = nByteBitLength - 1; nBitidx >= 0; nBitidx--)
    {
        CString strBitValue("");
        strBitValue.Format(_T("%d"), (byChannelState >> nBitidx) & TRUE);
        strCurPacketState.Append(strBitValue);
    }

    str_EnableChannelConnectionState.AppendFormat(_T("0b%s"), (LPCTSTR)strCurPacketState);

    return str_EnableChannelConnectionState;
}

long FrameGrabber3D_iGrabXE::GetReciveExternalTrigger(const unsigned int& grabber_id, const unsigned int& camera_id)
{
    unsigned int nReciveExternalTrigger(0);
    if (::Ix_GetNumExternalTrg(grabber_id, camera_id, nReciveExternalTrigger) == FALSE) //External Trigger
        return -99999;

    return nReciveExternalTrigger;
}

CString FrameGrabber3D_iGrabXE::GetPageTrigger_State(const unsigned int& grabber_id, const unsigned int& camera_id)
{
    CString strGetPageTriggerState("");

    BOOL bGetPageTrigger(FALSE);
    ::Ix_GetCheckPageTrg(grabber_id, camera_id, bGetPageTrigger); //Page Trigger

    strGetPageTriggerState.Format(_T(",Get PageTrigger: %s"), bGetPageTrigger == TRUE ? _T("Success") : _T("Fail"));

    return strGetPageTriggerState;
}

long FrameGrabber3D_iGrabXE::GetReciveFrameCount2Camera(const unsigned int& grabber_id, const unsigned int& camera_id)
{
    unsigned int nReciveFrameCount2Camera(0);
    if (::Ix_GetNumTransportFrm(grabber_id, camera_id, nReciveFrameCount2Camera)
        == FALSE) //Cam -> iGrabXE[B] Frame Count
        return -99999;

    return nReciveFrameCount2Camera;
}

CString FrameGrabber3D_iGrabXE::GetExceptionErrorMSG(const BYTE i_byMSGValue)
{
    CString strExceptionErrorMSG("");

    strExceptionErrorMSG.Format(_T("%x"), i_byMSGValue);

    return strExceptionErrorMSG;
}

iGrab3D_Error_LOG FrameGrabber3D_iGrabXE::GetiGrab3D_Error_LOG()
{
    iGrab3D_Error_LOG iGrab3D_Error_LOG_Copy = m_siGrab3D_Error_LOG;

    //ResetiGrab3D_Error_LOG();

    return iGrab3D_Error_LOG_Copy;
}

void FrameGrabber3D_iGrabXE::ResetiGrab3D_Error_LOG()
{
    m_siGrab3D_Error_LOG.Init();
}

bool FrameGrabber3D_iGrabXE::SetTriggerMode(int nMode)
{
    UNREFERENCED_PARAMETER(nMode);

    return true;
}

void FrameGrabber3D_iGrabXE::set_grabber_temperature(const int& grabber_id)
{
    float val(-1.f);

    if (::Ix_GetBoardTemp(grabber_id, val) == TRUE)
        apply_grabber_temperature(grabber_id, val);
}

void FrameGrabber3D_iGrabXE::get_firmware_version(
    const int& grabber_id, unsigned int& cur_version, unsigned int& need_version)
{
    cur_version = get_use_firmware_version(grabber_id);
    need_version = IF_FW_VERSION_MIN;
}

void FrameGrabber3D_iGrabXE::get_library_version(unsigned int& cur_version, unsigned int& need_version)
{
    cur_version = get_use_library_version();
    need_version = IF_LIB_VERSION;
}