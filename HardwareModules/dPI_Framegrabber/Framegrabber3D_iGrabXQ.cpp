
//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Framegrabber3D_iGrabXQ.h"

//CPP_2_________________________________ This project's headers
#include "SlitbeamDistortionData.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../InformationModule/dPI_DataBase/SlitBeam3DParameters.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedBuildConfiguration/iGrabXQ/iFsCtrlCmd.h"
#include "../../SharedBuildConfiguration/iGrabXQ/iManXQ.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../dPI_SyncController/SyncController.h"

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
#include <algorithm>
#include <fstream>
#include <thread>
#include <vector>


//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IF_LIB_VERSION 0x00010107
#define IF_FW_VERSION_MIN 0x000f0602

#define DEFAULT_IMAGE_SIZE_X 5120
#define DEFAULT_IMAGE_SIZE_Y 200
#define DEFAULT_IMAGE_FULL_SIZE_Y 4096

#define SWAP(x) ((x & 0xff000000) >> 24) | ((x & 0x000000ff) << 24) | ((x & 0x00ff0000) >> 8) | ((x & 0x0000ff00) << 8)

#define _USE_MATH_DEFINES

#define MAX_PROFILE_NUM 7000

enum CAMERA_ID
{
    CAMERA_0 = 0,
    CAMERA_1,
    CAMERA_2
};

enum GRABBER_ID
{
    iGRAB_BOARD_0 = 0,
    iGRAB_BOARD_1,
    iGRAB_BOARD_2
};

enum ENUM_MIKROTRON_21M
{
    CAM_ADDR_LINKCOFIG = 0x4014, // ConnectionConfigReg
    CAM_VALUE_12x4LINK = 0x40058, // CXP12_X4

    CAM_ADDR_ACQSTART = 0x8204, // AcquisitionStartReg
    CAM_ADDR_ACQSTOP = 0x8208, // AcquisitionStopReg
    CAM_VALUE_ON = 1,
    CAM_VALUE_OFF = 0,
    CAM_VALUE_LOAD = 0x00000002, // ?

    CAM_ADDR_IMAGEHEIGHT = 0x811c, // HeightReg
    CAM_ADDR_IMAGEOFFSETY = 0x8804, // OffsetYReg

    CAM_ADDR_USERSETLOAD = 0x00008824, // UserSetLoadReg
    CAM_ADDR_USERSETSAVE = 0x00008828, // UserSetSaveReg

    CAM_RESPONSE_TIMEOUT = 2000,
    CAM_CAMMAND_SLEEP = 100,
    CAM_COMMUNICATION_REPEATCOUNT = 3,
    CAM_FAN_RPM_PERCENTAGE = 0x00011404
};

//CPP_7_________________________________ Implementation body
//
static const float g_miminumExposureLow_ms = 0.01f;

static CCriticalSection g_cs;

BYTE CmdSetXQ(HANDLE hDev, UINT nAccType, UINT RegAddr, BYTE* pCmdData, INT nLength, BYTE* pReplyData, ULONG Interface,
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
                if (Interface == (ULONG)_IString)
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

bool WritePortXQ(HANDLE hDev, CString strCmd)
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
    BYTE Ret = CmdSetXQ(hDev, 1, COMM_WRITE_DATA, (BYTE*)dataBuff1, static_cast<INT>(numRegisterWritten),
        (BYTE*)&nReadBytes, _BootstrapInterface::_IInteger, 1000);

    return Ret == STATUS_CTRLCMD_SUCCESS;
}

bool ReadPortXQ(HANDLE hDev, CString& strRes)
{
    //int   DbgSize = 0;
    //ULONG DbgBuff[256];

    const unsigned int COMM_READ_LENGTH = 0x80000;
    const unsigned int COMM_READ_DATA = 0x80004;

    int nReadBytes = 0;
    size_t dataLength = 4;

    BOOL Ret = CmdSetXQ(hDev, 0, COMM_READ_LENGTH, NULL, static_cast<INT>(dataLength), (BYTE*)&nReadBytes,
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

    Ret = CmdSetXQ(hDev, 0, COMM_READ_DATA, NULL, static_cast<INT>(numRegistersRead), (BYTE*)Buffer, _IInteger, 1000);

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

void SolveBlackSunProblemXQ(const int& camera_id)
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

    if (WritePortXQ(hDev, _T("aar vieworks")) == FALSE)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPortXQ(hDev, strRes) == FALSE)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (WritePortXQ(hDev, _T("wcis 102 0x256e")) == false)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPortXQ(hDev, strRes) == false)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    //--------------------------------------
    // Read Command "rcis 102"
    //--------------------------------------
    if (WritePortXQ(hDev, _T("rcis 102")) == false)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPortXQ(hDev, strRes) == false)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    ::ctrlClose(hDev);
}

long GetCurBloValueXQ(CString strRes)
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

void CameraAnalogOffsetControlXQ(
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

    if (WritePortXQ(hDev, _T("aar vieworks")) == FALSE)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPortXQ(hDev, strRes) == FALSE)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (WritePortXQ(hDev, _T("gblo")) == false)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPortXQ(hDev, strRes) == false)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    nCurOffset1 = GetCurBloValueXQ(strRes);

    if (WritePortXQ(hDev, _T("gblo2")) == false)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPortXQ(hDev, strRes) == false)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    nCurOffset2 = GetCurBloValueXQ(strRes);

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
    if (WritePortXQ(hDev, strCommand) == false)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPortXQ(hDev, strRes) == false)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    strCommand.Format(_T("sblo2 %d"), nTargetOffset2);
    if (WritePortXQ(hDev, strCommand) == false)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPortXQ(hDev, strRes) == false)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    ::ctrlClose(hDev);
}

void TurnOnOffPrnuCorrectionXQ(const int& camera_id, bool on)
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

    if (WritePortXQ(hDev, _T("aar vieworks")) == FALSE)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPortXQ(hDev, strRes) == FALSE)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (on)
    {
        if (WritePortXQ(hDev, _T("sprnu 1")) == false)
        {
            ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
            ::exit(0);
        }
    }
    else
    {
        if (WritePortXQ(hDev, _T("sprnu 0")) == false)
        {
            ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
            ::exit(0);
        }
    }

    if (ReadPortXQ(hDev, strRes) == false)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    //--------------------------------------
    // Read Command "rprnu"
    //--------------------------------------
    if (WritePortXQ(hDev, _T("rprnu")) == false)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPortXQ(hDev, strRes) == false)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    ::ctrlClose(hDev);
}

void TurnOnOffDsnuCorrectionXQ(const int& camera_id, bool on)
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

    if (WritePortXQ(hDev, _T("aar vieworks")) == FALSE)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPortXQ(hDev, strRes) == FALSE)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (on)
    {
        if (WritePortXQ(hDev, _T("sdsnu 1")) == false)
        {
            ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
            ::exit(0);
        }
    }
    else
    {
        if (WritePortXQ(hDev, _T("sdsnu 0")) == false)
        {
            ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
            ::exit(0);
        }
    }

    if (ReadPortXQ(hDev, strRes) == false)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    //--------------------------------------
    // Read Command "rdsnu"
    //--------------------------------------
    if (WritePortXQ(hDev, _T("rdsnu")) == false)
    {
        ::AfxMessageBox(_T("Cannot send command to camera"), MB_ICONERROR);
        ::exit(0);
    }

    if (ReadPortXQ(hDev, strRes) == false)
    {
        ::AfxMessageBox(_T("Cannot receive response from camera"), MB_ICONERROR);
        ::exit(0);
    }

    ::ctrlClose(hDev);
}

union _tagLinkConfigVal_Mikrotron
{
    struct
    {
        UINT nFisrt : 8;
        UINT nSecond : 8;
        UINT nThird : 8;
        UINT nFourth : 8;
    };

    UINT nValue;
};

typedef _tagLinkConfigVal_Mikrotron LINKCONFVAL_Mikrotron;

BOOL SetData_ToCamera(const int& camera_id, ULONG i_nADDR, ULONG i_nSetData)
{
    HANDLE hDev = ::ctrlOpen(camera_id);

    BYTE pData[4];
    BYTE byAckCode = 0;   

    LINKCONFVAL_Mikrotron nRawData;
    BYTE byResultCMD = 0;
    BYTE byResultACK = 0;
    long nRepeat = -1;

    do
    {
        memset(pData, 0, 4);
        ULONG nSize = sizeof(ULONG);

        nRawData.nValue = i_nSetData;
        pData[0] = nRawData.nFisrt;
        pData[1] = nRawData.nSecond;
        pData[2] = nRawData.nThird;
        pData[3] = nRawData.nFourth;

        byResultCMD = ctrlCmdMsg(hDev, 1, nSize, i_nADDR, (BYTE*)pData, 2000);
        //::Sleep(CAM_CAMMAND_SLEEP);
        if (byResultCMD == STATUS_CTRLCMD_SUCCESS)
        {
            byResultACK = ctrlAckMsg(hDev, byAckCode, nSize, (BYTE*)pData);
            ::Sleep(CAM_CAMMAND_SLEEP);

            //int nRdDataSize = 0;
            //ULONG pDbgRdData[1024];
            //BYTE byDbgResult = ctrlDbgAckMsg(m_hCamHandle, nRdDataSize, pDbgRdData);

            //if (byResultACK != STATUS_CTRLCMD_SUCCESS)
            //{
            //	byDbgResult = byDbgResult;
            //}
            ::Sleep(CAM_CAMMAND_SLEEP);
        }
        else
        {
            // 2023.09.22
            // 실패했을 때에는 시간을 10배 늘려서 조금 더 기다려 주도록 하자
            // 여기의 else 코드가 빠져 있어서 맛탱이 가는 경우 발생되었음
            ::Sleep(CAM_CAMMAND_SLEEP * 10);
        }

        nRepeat++;
    }
    while ((byResultACK != STATUS_CTRLCMD_SUCCESS) && (nRepeat < 3));

    if (byResultACK != STATUS_CTRLCMD_SUCCESS)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL GetData_FromCamera(const int& camera_id, ULONG i_nADDR, ULONG& o_nGetData)
{
    HANDLE hDev = ::ctrlOpen(camera_id);

    BYTE byAckCode = 0;
    BYTE pData[4];

    BYTE byResultCMD = 0;
    BYTE byResultACK = 0;
    long nRepeat = -1;
    do
    {
        memset(pData, 0, 4);
        ULONG nSize = sizeof(ULONG);
        byResultCMD = ctrlCmdMsg(hDev, 0, nSize, i_nADDR, (BYTE*)pData, 2000);

        if (byResultCMD == STATUS_CTRLCMD_SUCCESS)
        {
            byResultACK = ctrlAckMsg(hDev, byAckCode, nSize, (BYTE*)pData);
            ::Sleep(CAM_CAMMAND_SLEEP);

            //int nRdDataSize = 0;
            //ULONG pDbgRdData[1024];
            //BYTE byDbgResult = ctrlDbgAckMsg(m_hCamHandle, nRdDataSize, pDbgRdData);

            //if (byResultACK != STATUS_CTRLCMD_SUCCESS)
            //{
            //	byDbgResult = byDbgResult;
            //}
            //::Sleep(CAM_CAMMAND_SLEEP);
        }

        nRepeat++;
    }
    while ((byResultACK != STATUS_CTRLCMD_SUCCESS) && (nRepeat < 3));

    o_nGetData = *((long*)pData);

    if (byResultACK != STATUS_CTRLCMD_SUCCESS)
    {
        return FALSE;
    }

    return TRUE;
}

Framegrabber3D_iGrabXQ::Framegrabber3D_iGrabXQ()
    : grabber_id_use_grab(0)
    , camera_id_use_grab(0)
    //, m_profileNumber(0)
    , m_zmapImage(new Ipvm::Image32r)
    , m_zmapImage2(new Ipvm::Image32r)
    , m_zmapImage3(new Ipvm::Image32r)
    , m_vmapImage(new Ipvm::Image32r)
    , m_vmapImage2(new Ipvm::Image32r)
    , m_vmapImage3(new Ipvm::Image32r)
    , m_pCore(NULL)
    , m_resultStitchIndex(0)
    , m_scanImageCount(0)
    , m_distortionData(new SlitbeamDistortionData(DEFAULT_IMAGE_SIZE_X))
    , m_nCameraNum(SystemConfig::GetInstance().Get3DVisionCameraNumber())
    , m_nDual3DGrabMode(DUAL_IMAGE_TOTAL_REFLECTION)
    , m_hGrabWaitFirst(NULL)
    , m_hGrabWaitSecond(NULL)
    , m_hCalcWaitFirst(NULL)
    , m_hCalcWaitSecond(NULL)
    , m_n2DRawImageSizeXY(0)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    m_hGrabWaitFirst = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hGrabWaitSecond = CreateEvent(NULL, FALSE, FALSE, NULL);

    m_hCalcWaitFirst = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hCalcWaitSecond = CreateEvent(NULL, FALSE, FALSE, NULL);

    camera_serials.clear();
    custom_grab_event_counts.clear();
    dsnu_images.clear();
    dsnu_binning_images.clear();
    m_use_binning.clear();

    const CString path_xml_file = get_camera_param_save_path();
    const LPCTSTR resource_name = get_camera_param_resourceID(SystemConfig::GetInstance().GetVisionType());

    FrameGrabber_Base::ExtractResourceFile(_T("XCF"), resource_name, path_xml_file);
    FrameGrabber_Base::SavesettingFile(path_xml_file);

    init_grabber_info();

    init_camera_info(path_xml_file);

    init_dsnu_image_buf(DEFAULT_IMAGE_SIZE_X, DEFAULT_IMAGE_FULL_SIZE_Y); //나중에 변수로 받든지 말든지

    init_camera_serial();

    init_image_size_default(DEFAULT_IMAGE_SIZE_X, DEFAULT_IMAGE_SIZE_Y, path_xml_file); //나중에 변수로 받든지 말든지

    init_camera_default_param();

    //apply_camera_additional_param();

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

    // Image init
    //m_zmapImage.resize(m_nCameraNum);
    //m_vmapImage.resize(m_nCameraNum);
    size_t bufferSize = m_nCameraNum + 1;
    m_vmapResizeTempImage.resize(bufferSize);

    m_resultZmapImage.resize(bufferSize);
    m_resultVmapImage.resize(bufferSize);

    m_scanBuffers.resize(bufferSize);
    m_profileNumber.resize(m_nCameraNum);
    m_scanImageCount.resize(m_nCameraNum);

    for (long nCore = 0; nCore < m_nCameraNum; nCore++)
    {
        m_profileNumber[nCore] = 0;
        m_scanImageCount[nCore] = 0;
    }

    for (long nCore = 0; nCore < bufferSize; nCore++)
    {
        /* 
        m_zmapImage[nCore] = new Ipvm::Image32r;
        m_vmapImage[nCore] = new Ipvm::Image32r;*/
        m_vmapResizeTempImage[nCore] = new Ipvm::Image16u;
       
        m_resultZmapImage[nCore] = nullptr;
        m_resultVmapImage[nCore] = nullptr;
    }

    //
    //GPU는 1개로 한정한다
    ////===========================================///////
    ////================== Mem Alloc  ================//////
    m_pCore.resize(m_nCameraNum + 1, nullptr);
    for (long nCore = 0; nCore < m_nCameraNum + 1; nCore++)
    {
        m_pCore[nCore] = new Ipvm::SbtCoreGpu(0);
    }
    //m_pCore = new Ipvm::SbtCoreGpu(0);
    m_pCoreParam = new Ipvm::SbtCorePara;
}

Framegrabber3D_iGrabXQ::~Framegrabber3D_iGrabXQ(void)
{
    if (get_grabber_num() > 0)
    {
        for (long grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
        {
            const int& grabber_id = get_grabber_id(grabber_idx);
            for (long camera_idx = 0; camera_idx < get_camera_num(grabber_idx); camera_idx++)
            {
                const int& camera_id = get_camera_id(grabber_id, camera_idx);
                ::Ixq_CloseChannel(grabber_id, camera_id);
            }

            ::Ixq_CloseSystem(grabber_id);
        }
    }

    for (long nCore = 0; nCore < (long)m_pCore.size(); nCore++)
    {
        delete m_pCore[nCore];
        m_pCore[nCore] = nullptr;
    }
    m_pCore.clear();

    ::CloseHandle(m_hGrabWaitFirst);
    ::CloseHandle(m_hGrabWaitSecond);
    ::CloseHandle(m_hCalcWaitFirst);
    ::CloseHandle(m_hCalcWaitSecond);
}

void Framegrabber3D_iGrabXQ::init_grabber_info()
{
    UINT libraryVersion(0);
    ::Ixq_GetLibraryVersion(libraryVersion);

    if (libraryVersion != IF_LIB_VERSION)
    {
        //mc_다른곳에서 MSG를 띄울것이다
        //SystemConfig::GetInstance().Set_is_iGrabFirmware_and_LibraryVersion_Mismatch(true);
    }

    set_library_version(libraryVersion);

    if (set_grabber_num(::Ixq_DeviceList()) == false)
    {
        ::AfxMessageBox(_T("Failed to open iGrabXE"), MB_ICONERROR | MB_OK);
        ::exit(0);
    }

    for (int grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
    {
        const int& id_grabber = get_grabber_id(grabber_idx);
        if (::Ixq_OpenSystem(grabber_idx, 0) == FALSE)
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
            ::Ixq_GetFirmwareVersion(id_grabber, firmware_version);
            set_firmware_version(id_grabber, firmware_version);

            if (firmware_version < IF_FW_VERSION_MIN)
            {
                //mc_다른곳에서 MSG를 띄울것이다
                //SystemConfig::GetInstance().Set_is_iGrabFirmware_and_LibraryVersion_Mismatch(true);
            }
        }
    }
}

//void Framegrabber2D_iGrabXE::init_camera_info(const CString& camera_param_file_path)
//{
//    if (camera_param_file_path.IsEmpty() == true)
//    {
//        CString str("");
//        str.Format(_T("Cannot open camera set param file."));
//        ::AfxMessageBox(str, MB_ICONERROR | MB_OK);
//        ::exit(0);
//    }
//
//    if (!IsIcfExist(camera_param_file_path))
//    {
//        CString str;
//        str.Format(_T("Cannot find \"%s\" file."), LPCTSTR(camera_param_file_path));
//        ::AfxMessageBox(str, MB_ICONERROR | MB_OK);
//        ::exit(0);
//    }
//
//    //camera & icf set
//    int channel_num(1), id_icf(-1);
//    for (int grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
//    {
//        //Channel ID set [iGrabG2C는 CL type이므로 chaanel이 무조건 1EA다 따라서 0번 chaanel을 사용]
//        const int id_graaber = get_grabber_id(grabber_idx);
//        for (int id_channel = 0; id_channel < channel_num; id_channel++)
//        {
//            if (::Ix_OpenXCF(id_graaber, id_channel, id_icf, CStringA(camera_param_file_path).GetBuffer()) == FALSE)
//            {
//                CString str;
//                str.Format(_T("Cannot open \"%s\" file."), LPCTSTR(camera_param_file_path));
//                ::AfxMessageBox(str, MB_ICONERROR | MB_OK);
//                ::exit(0);
//            }
//            else
//            {
//                set_camera_icf(id_graaber, id_channel, id_icf);
//            }
//        }
//    }
//
//    //size
//    unsigned int width(0), height(0);
//    for (long grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
//    {
//        const int id_grabber = get_grabber_id(grabber_idx);
//        const int connect_camera_num = get_camera_num(id_grabber);
//
//        for (long camera_idx = 0; camera_idx < connect_camera_num; camera_idx++)
//        {
//            const int id_channel = get_camera_id(id_grabber, camera_idx);
//            const int id_icf = get_camera_icf_id(id_grabber, id_channel);
//
//            ::Ix_GetXCFParam(id_grabber, id_channel, id_icf, XCF_CTL_HORIZONTAL_SIZE, &width);
//            ::Ix_GetXCFParam(id_grabber, id_channel, id_icf, XCF_CTL_VERTICAL_SIZE, &height);
//
//            set_grab_image_width(grabber_idx, camera_idx, width);
//            set_grab_image_height(grabber_idx, camera_idx, height);
//        }
//    }
//
//    live_buffer_mono->Create(width, height);
//}

void Framegrabber3D_iGrabXQ::init_camera_info(const CString& camera_param_file_path)
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

    long open_channel_num(0); //Open된 channel이 하나도 없으면 잘못된 상황이다 바로 종료한다
    int id_icf(-1);
    for (int grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
    {
        const int id_graaber = get_grabber_id(grabber_idx);
        for (int camera_idx = 0; camera_idx < MAX_CAMERA_NUMBER; camera_idx++)
        {
            if (check_is_valid_camera_channel(id_graaber, camera_idx) == false)
            {
                //사용하지 않는 channel이니 닫는다
                if (::Ixq_CloseChannel(id_graaber, camera_idx) == FALSE)
                {
                    ::AfxMessageBox(_T("Failed to close camera handle"));
                    exit(0);
                }

                continue;
            }

            
            //// 통신 시도하기 전에 Freerun 모드로 가 있게 한다.
            //::Ixq_NewXCF(id_graaber, camera_idx, id_icf);

            //if (id_icf < 0)
            //{
            //    ::Ixq_CloseChannel(id_graaber, camera_idx);
            //    continue;
            //}

            //::Ixq_SetXCFParam(id_graaber, camera_idx, id_icf, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);

            if (::Ixq_OpenXCF(id_graaber, camera_idx, id_icf, CStringA(camera_param_file_path).GetBuffer()) == FALSE)
            {
                CString str;
                str.Format(_T("Cannot open \"%s\" file."), LPCTSTR(camera_param_file_path));
                ::AfxMessageBox(str, MB_ICONERROR | MB_OK);
                ::exit(0);
            }
            else
            {
                set_camera_icf(id_graaber, camera_idx, id_icf);
            }
            //Camera & icf set
            //set_camera_icf(id_graaber, camera_idx, id_icf);

            ::Ixq_SetXCFParam(id_graaber, camera_idx, id_icf, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);

            UINT nWidth = 0;
            UINT nHeight = 0;

            if (!Ixq_GetXCFParam(id_graaber, camera_idx, id_icf, IGRABXQ::XCF_CTL_HORIZONTAL_SIZE, &nWidth))
            {
                CString str;
                str.Format(_T("Get XCF_CTL_HORIZONTAL_SIZE 실패\n"), LPCTSTR(camera_param_file_path));
                ::AfxMessageBox(str, MB_ICONERROR | MB_OK);
                ::exit(0);
            }

            if (!Ixq_GetXCFParam(id_graaber, camera_idx, id_icf, IGRABXQ::XCF_CTL_VERTICAL_SIZE, &nHeight))
            {
                CString str;
                str.Format(_T("Get XCF_CTL_VERTICAL_SIZE 실패\n"), LPCTSTR(camera_param_file_path));
                ::AfxMessageBox(str, MB_ICONERROR | MB_OK);
                ::exit(0);
            }

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

BOOL Framegrabber3D_iGrabXQ::IsIcfExist(LPCTSTR szPathName)
{
    CFile file;
    if (file.Open(szPathName, CFile::modeRead))
    {
        file.Close();

        return true;
    }

    return false;
}

bool Framegrabber3D_iGrabXQ::check_is_valid_camera_channel(const int& id_graaber, const int& camera_idx)
{
    if (::Ixq_OpenChannel(id_graaber, camera_idx) == FALSE) //이게 fase면 grabber가 문제있는 상황
    {
        return false;
    }
    else
    {
        BYTE plug_channel(0x00);
        if (::Ixq_GetChannelPlugAssign(id_graaber, camera_idx, plug_channel) == TRUE && plug_channel != 0x00)
        {
            return true;
        }
        else //할당이 안된상황g
            return false;
    }

    return true;
}

bool Framegrabber3D_iGrabXQ::init_dsnu_image_buf(const int32_t& width, const int32_t& height)
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

bool Framegrabber3D_iGrabXQ::init_camera_serial()
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
                if (get_device_serial(id_grabber, camera_serial) == true || camera_serial.IsEmpty() == false)
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

bool Framegrabber3D_iGrabXQ::get_device_serial(const int& camera_id, CString& serialNumber)
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

    BYTE ret1 = ::ctrlCmdMsg(handle, 0, 16, ulDeviceSerialNumberAddress, nullptr, 1000);

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

bool Framegrabber3D_iGrabXQ::init_image_size_default(
    const unsigned int& width, const unsigned int& height, const CString camera_param_file_path)
{
    if (width < 0 || height < 0)
    {
        popup_function_error(_T("init_image_size_default"));
        return false;
    }

    UINT nWidth = 0;
    UINT nHeight = 0;
    
    for (long grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
    {
        const int& id_grabber = get_grabber_id(grabber_idx);
        
        for (long camera_idx = 0; camera_idx < get_camera_num(id_grabber); camera_idx++)
        {
            const int& id_icf = get_camera_icf_id(id_grabber, camera_idx);
            
            if (!Ixq_GetXCFParam(grabber_idx, camera_idx, id_icf, IGRABXQ::XCF_CTL_HORIZONTAL_SIZE, &nWidth))
            {
                set_grab_image_width(grabber_idx, camera_idx, width); // 실패하면 default 파라미터 사용
            }
            else
            {
                set_grab_image_width(grabber_idx, camera_idx, nWidth);
            }

            if (!Ixq_GetXCFParam(grabber_idx, camera_idx, id_icf, IGRABXQ::XCF_CTL_VERTICAL_SIZE, &nHeight))
            {
                set_grab_image_height(grabber_idx, camera_idx, height); // 실패하면 default 파라미터 사용
            }
            else
            {
                set_grab_image_height(grabber_idx, camera_idx, nHeight);
            }
            set_grab_image_offset_Y(grabber_idx, camera_idx, 0);
        }
    }

    return true;
}

bool Framegrabber3D_iGrabXQ::init_camera_default_param()
{
    // Register address
    ULONG ulUserBaseAddress = 0x10000000;
    ULONG ulTrigBaseAddress = ulUserBaseAddress + 0x00020000;
    ULONG ulAddressAcqStart = ulTrigBaseAddress + 0x00001084;
    ULONG ulAddressAcqStop = ulTrigBaseAddress + 0x000010C4;

    if (SystemConfig::GetInstance().Get3DVisionCameraType() == CAMERATYPE_MIKROTRON_EOSENS_21CXP2)
    {
        ulAddressAcqStart = 0x8204;
        ulAddressAcqStop = 0x8208;
    }

    // Communication Status Check
    long nTrue = 1;
    long nFalse = 0;
    BYTE ret = 0;
    

    for (int grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
    {
        const int& id_grabber = get_grabber_id(grabber_idx);
        for (int camera_idx = 0; camera_idx < get_camera_num(id_grabber); camera_idx++)
        {
            const int& id_camera = get_camera_id(id_grabber, camera_idx);
            const int& id_icf = get_camera_icf_id(id_grabber, id_camera);

            HANDLE handle = ::ctrlOpen(id_camera);

            // Acq Stop
            ret = ::ctrlCmdMsg(handle, 1, sizeof(long), ulAddressAcqStart, (BYTE*)&nFalse, 1000);
            Sleep(CAM_CAMMAND_SLEEP);
            ret = ::ctrlCmdMsg(handle, 1, sizeof(long), ulAddressAcqStop, (BYTE*)&nTrue, 1000);
            Sleep(CAM_CAMMAND_SLEEP);

            ::Ixq_SetXCFParam(
                id_grabber, id_camera, id_icf, XCF_CTL_HORIZONTAL_SIZE, get_grab_image_width(grabber_idx, camera_idx));
            ::Ixq_SetXCFParam(
                id_grabber, id_camera, id_icf, XCF_CTL_VERTICAL_SIZE, get_grab_image_height(grabber_idx, camera_idx));
            ::Ixq_SetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_TRIGGER_TYPE, _enumTriggerType::HardwareTrigger);
            ::Ixq_SetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_PAGE_TRIGGER, FALSE);
            ::Ixq_SetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_IMAGE_BIT, _enumImagebit::Image8bit);
            ::Ixq_SetXCFParam(id_grabber, id_camera, id_icf, XCF_CTL_CAM_MODE, _enumCameraMode::AreaScan);

            double validFramePeriod_ms(0.);
            set_exposure_time(grabber_idx, camera_idx, 2., validFramePeriod_ms);

            int cur_cust_event_num(0), max_cust_event_num(0);
            ::Ixq_GetCustomGrabEvent(id_grabber, id_camera, &cur_cust_event_num, &max_cust_event_num);
            ::Ixq_SetCustomGrabEvent(id_grabber, id_camera, max_cust_event_num);
            custom_grab_event_counts[id_grabber][id_camera] = max_cust_event_num;

            // Acq Start
            ret = ::ctrlCmdMsg(handle, 1, sizeof(long), ulAddressAcqStop, (BYTE*)&nFalse, 1000);
            Sleep(CAM_CAMMAND_SLEEP);
            ret = ::ctrlCmdMsg(handle, 1, sizeof(long), ulAddressAcqStart, (BYTE*)&nTrue, 1000);
            Sleep(CAM_CAMMAND_SLEEP);

            // Close Device...
            ret = CAST_BYTE(::ctrlClose(handle));

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

bool Framegrabber3D_iGrabXQ::apply_camera_additional_param()
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

            SolveBlackSunProblemXQ(id_camera);
            CameraAnalogOffsetControlXQ(id_camera, n3DCameraAnalogOffset1, n3DCameraAnalogOffset2, camera_serial);
            TurnOnOffPrnuCorrectionXQ(id_camera, true);
            TurnOnOffDsnuCorrectionXQ(id_camera, false);
        }
    }

    return true;
}

bool Framegrabber3D_iGrabXQ::apply_camera_dsnu()
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

bool Framegrabber3D_iGrabXQ::grab(const int grabber_idx, const int camera_idx, const int stitchIndex,
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

    m_resultZmapImage[id_grabber] = &zmapImage;
    m_resultVmapImage[id_grabber] = &vmapImage;
    m_resultStitchIndex = stitchIndex;
    height_range_min = -0.5f * m_pCore[id_grabber]->GetHeightRange_um(*m_pCoreParam);
    height_range_max = 0.5f * m_pCore[id_grabber]->GetHeightRange_um(*m_pCoreParam);

    //m_zmapImage[id_grabber]->FillZero();
    //m_vmapImage[id_grabber]->FillZero();

    m_zmapImage->FillZero();
    m_vmapImage->FillZero();

    auto coreResult = m_pCore[id_grabber]->ProcessAsync(NULL, *m_zmapImage, *m_vmapImage);

    if (coreResult != Ipvm::Status::e_ok)
    {
        m_strErrorMessage = Ipvm::ToString(coreResult);
        SetiGrab3D_Error_LOG();
        return false;
    }

    ::Ixq_SetXCFParam(id_grabber, id_camera, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::UplinkTrigger);

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

    m_scanImageCount[0] = 0;

    const DWORD dwCalcTimeOut = SystemConfig::GetInstance().GetGrabTimeoutTime();
    DWORD dwCalcTimeOut2 = 15000;

    DWORD ret = ::Ixq_MultiGrabProcess(id_grabber, id_camera, NULL, 0, m_profileNumber[id_grabber],
        MtgWaitOp::MTG_GRB_ISSUE, dwCalcTimeOut2, ScanEnd, this);

    //2023.02.23_DEV. Debug Log
    //Ix_DbgHardIntrCount(id_grabber, id_camera, g_hard_intr_count_s); // 항상 0이 나와야됨.
    //Ix_DbgSoftIntrCount(id_grabber, id_camera, g_soft_intr_count_s); // 항상 0이 나와야됨.

    //m_siGrab3D_Error_LOG.m_hard_intr_count_s = g_hard_intr_count_s;
    //m_siGrab3D_Error_LOG.m_soft_intr_count_s = g_soft_intr_count_s;
    //

    //2023.05.30_DEV. Debug Log
    m_siGrab3D_Error_LOG.m_dwDbgMultGrabProcessReturnVal = ret;
    //

    if (ret != STATUS_GRAB_START_SUCCESS)
    {
        ::Ixq_SetXCFParam(id_grabber, id_camera, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);

        m_strErrorMessage = _T("MultiGrabProcess Fail");

        SetiGrab3D_Error_LOG();
    }

    return ret;
}

bool Framegrabber3D_iGrabXQ::multigrab(const int stitchIndex, VisionImageLot& imageLot, float& height_range_min,
    float& height_range_max, Ipvm::AsyncProgress* progress)
{
    if (progress)
    {
        progress->Post(L"Prepairing grab");
    }

    DWORD ret;

    for (int grabber_idx = 0; grabber_idx < get_grabber_num(); grabber_idx++)
    {
        const int& id_grabber = get_grabber_id(grabber_idx);
        const int& id_camera = get_camera_id(id_grabber, 0);

        if (is_valid_index(id_grabber, id_camera) == false)
            return false;

        const int& icf_id = get_camera_icf_id(id_grabber, id_camera);
        grabber_id_use_grab = id_grabber;
        camera_id_use_grab = id_camera;

        ResetiGrab3D_Error_LOG();

        if (id_grabber == 0)
        {
            if (progress)
            {
                CString str;
                str.Format(_T("Prepairing grab(core-%d)"), grabber_idx);
                progress->Post(str);
            }

            m_resultZmapImage[id_grabber] = &imageLot.GetZmapImage(id_grabber);
            m_resultVmapImage[id_grabber] = &imageLot.GetVmapImage(id_grabber);

            m_resultStitchIndex = stitchIndex;
            height_range_min = -0.5f * m_pCore[id_grabber]->GetHeightRange_um(*m_pCoreParam);
            height_range_max = 0.5f * m_pCore[id_grabber]->GetHeightRange_um(*m_pCoreParam);

            m_zmapImage->FillZero();
            m_vmapImage->FillZero();

            auto coreResult = m_pCore[id_grabber]->ProcessAsync(NULL, *m_zmapImage, *m_vmapImage);

            if (coreResult != Ipvm::Status::e_ok)
            {
                m_strErrorMessage = Ipvm::ToString(coreResult);
                SetiGrab3D_Error_LOG();
                return false;
            }

            imageLot.Set3DScanInfo(
                height_range_min, height_range_max, (int)enDualGrabImageType::DUAL_IMAGE_TOTAL_REFLECTION);

            if (m_nDual3DGrabMode == VISION_3D_GRABMODE_INTEGRATED)
            {
                if (progress)
                {
                    CString str;
                    str.Format(_T("Prepairing grab(core-%d)"), grabber_idx);
                    progress->Post(str);
                }

                m_resultZmapImage[id_grabber + 2] = &imageLot.GetZmapImage(id_grabber + 2);
                m_resultVmapImage[id_grabber + 2] = &imageLot.GetVmapImage(id_grabber + 2);

                height_range_min = -0.5f * m_pCore[id_grabber + 2]->GetHeightRange_um(*m_pCoreParam);
                height_range_max = 0.5f * m_pCore[id_grabber + 2]->GetHeightRange_um(*m_pCoreParam);

                m_zmapImage3->FillZero();
                m_vmapImage3->FillZero();

                auto coreResult = m_pCore[id_grabber + 2]->ProcessAsync(NULL, *m_zmapImage3, *m_vmapImage3);

                if (coreResult != Ipvm::Status::e_ok)
                {
                    m_strErrorMessage = Ipvm::ToString(coreResult);
                    SetiGrab3D_Error_LOG();
                    return false;
                }

                imageLot.Set3DScanInfo(
                    height_range_min, height_range_max, (int)enDualGrabImageType::DUAL_IMAGE_COMPLEX_REFLECTION);
            }
        }
        else if (id_grabber == 1)
        {
            if (progress)
            {
                CString str;
                str.Format(_T("Prepairing grab(core-%d)"), grabber_idx);
                progress->Post(str);
            }

            m_resultZmapImage[id_grabber] = &imageLot.GetZmapImage(id_grabber);
            m_resultVmapImage[id_grabber] = &imageLot.GetVmapImage(id_grabber);

            m_resultStitchIndex = stitchIndex;
            height_range_min = -0.5f * m_pCore[id_grabber]->GetHeightRange_um(*m_pCoreParam);
            height_range_max = 0.5f * m_pCore[id_grabber]->GetHeightRange_um(*m_pCoreParam);

            m_zmapImage2->FillZero();
            m_vmapImage2->FillZero();

            auto coreResult = m_pCore[id_grabber]->ProcessAsync(NULL, *m_zmapImage2, *m_vmapImage2);

            if (coreResult != Ipvm::Status::e_ok)
            {
                m_strErrorMessage = Ipvm::ToString(coreResult);
                SetiGrab3D_Error_LOG();
                return false;
            }

            imageLot.Set3DScanInfo(
                height_range_min, height_range_max, (int)enDualGrabImageType::DUAL_IMAGE_DIFFUSE_REFLECTION);
        }

        ::Ixq_SetXCFParam(id_grabber, id_camera, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::UplinkTrigger);
    }

    if (progress)
    {
        progress->Post(L"Grab start");
    }
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

    m_scanImageCount[0] = 0;
    m_scanImageCount[1] = 0;

    const DWORD dwCalcTimeOut = SystemConfig::GetInstance().GetGrabTimeoutTime();
    //const long nGrabTimeOut = 15000;

    ResetEvent(m_hGrabWaitFirst);
    ::AfxBeginThread(GrabWaitFirst, this, THREAD_PRIORITY_HIGHEST);

    ResetEvent(m_hGrabWaitSecond);
    ::AfxBeginThread(GrabWaitSecond, this, THREAD_PRIORITY_HIGHEST);

    //if (id_grabber == 0)
    //{
    //    if (m_nDual3DGrabMode == VISION_3D_GRABMODE_DEFUSED)
    //    {
    //        ret = ::Ixq_MultiGrabProcess(id_grabber, id_camera, NULL, 0, m_profileNumber[id_grabber],
    //            MtgWaitOp::MTG_GRB_ISSUE, nGrabTimeOut, ScanEnd, this);
    //    }
    //    else if (m_nDual3DGrabMode == VISION_3D_GRABMODE_INTEGRATED)
    //    {
    //        ret = ::Ixq_MultiGrabProcess(id_grabber, id_camera, NULL, 0, m_profileNumber[id_grabber],
    //            MtgWaitOp::MTG_GRB_ISSUE, nGrabTimeOut, ScanEnd, this);
    //    }
    //}
    //else if (id_grabber == 1)
    //{
    //    ret = ::Ixq_MultiGrabProcess(id_grabber, id_camera, NULL, 0, m_profileNumber[id_grabber],
    //        MtgWaitOp::MTG_GRB_ISSUE, nGrabTimeOut, ScanEnd2, this);
    //}

    //2023.05.30_DEV. Debug Log
    //m_siGrab3D_Error_LOG.m_dwDbgMultGrabProcessReturnVal = ret;
    //

    // -----------------  WAIT CAM0/CAM1 GRAB END  ----------------- //
    // m_hCalcWaitFirst : CAM 0 의 Grab Signal
    // m_hCalcWaitSecond : CAM 1 의 Grab Signal
    // Signal Handle 은 Auto Reset 이 되도록 생성 되었으므로
    // WaitForMultipleObjects 를 빠져나가면 자동으로 UnSignal 상태로 전환된다.
    ResetEvent(m_hCalcWaitFirst);
    ResetEvent(m_hCalcWaitSecond);
    std::vector<BOOL> vecbGrab(m_nCameraNum, FALSE);
    HANDLE hGrabHandle[] = {m_hGrabWaitFirst, m_hGrabWaitSecond};

    float fFirstGrabTime = 0.f;
    float fSecondGrabTime = 0.f;
    float fGrabTime = 0;
    BOOL bGrabFail = FALSE;

    CString strErrorLog;
    const long nGrabTimeOut = 20000;
    const long nCalcTimeOut = 20000;

    while (1)
    {
        // Grab 이 끝나면 m_hGrabWaitFirst, m_hGrabWaitSecond Signal 이 On 되므로
        // Wait 가 끝나고 아래의 시퀀스(Switch case) 가 진행된다.
        // Signal 이 On 되었다면 그랩된 이미지로 COG Calc Thread 를 실행시킨다.
        DWORD dwResult = ::WaitForMultipleObjects(m_nCameraNum, hGrabHandle, FALSE, nGrabTimeOut);
        //DWORD dwResult = ::Ixq_MultiGrabWait(iGRAB_BOARD_0, CAMERA_0);

        switch (dwResult)
        {
            // [CAM 0] Grab End Signal On
            case WAIT_OBJECT_0:
            {
                //fFirstGrabTime = (float)m_CoreCPU[CAMERA_0]->GetTimeChk();
                // -----------------  CAM 0 COG CALC START  ----------------- //
                ::AfxBeginThread(CalcWaitFirst, this);
            }
                vecbGrab[CAMERA_0] = TRUE;
                break;
            // [CAM 1] Grab End Signal On
            case WAIT_OBJECT_0 + 1:
            {
                //fSecondGrabTime = (float)m_CoreCPU[CAMERA_1]->GetTimeChk();
                // -----------------  CAM 1 COG CALC START  ----------------- //
                ::AfxBeginThread(CalcWaitSecond, this);
            }
                vecbGrab[CAMERA_1] = TRUE;
                break;
            case WAIT_FAILED:
                bGrabFail = TRUE;

                strErrorLog.Format(
                    _T("GRAB FAIL, FILENAME : %s, FUNCNAME : %s, LINENO : %d"), __FILE__, __FUNCTION__, __LINE__);

                break;
            case WAIT_TIMEOUT:
                bGrabFail = TRUE;

                strErrorLog.Format(
                    _T("GRAB TIMEOUT, FILENAME : %s, FUNCNAME : %s, LINENO : %d"), __FILE__, __FUNCTION__, __LINE__);

                break;
        }

        if (vecbGrab[CAMERA_0] & vecbGrab[CAMERA_1])
        {
            SyncController::GetInstance().SetGrabExposure(FALSE);
            SyncController::GetInstance().SetGrabAcquisition(FALSE);

            // CAM 0, CAM 1 의 Grab 이 모두 성공한 경우
            fGrabTime = max(fFirstGrabTime, fSecondGrabTime);
            break;
        }

        if (bGrabFail)
        {
            // CAM 0, CAM 1 중 하나라도 Grab 이 실패한 경우
            //fGrabTime = (float)max(m_CoreCPU[CAMERA_0]->GetTimeChk(), m_CoreCPU[CAMERA_1]->GetTimeChk());
            break;
        }
    }

    // CAM0/1 의 Grab 이 완료되면 조명을 꺼주도록 하자.
    //m_pSyncControllerSlit->Set3DLedIntensity(0xa0, 0x1b);

    // -----------------  WAIT CAM0/CAM1 COG CALC END  ----------------- //
    // m_hCalcWaitFirst : CAM 0 의 COG Calc Signal
    // m_hCalcWaitSecond : CAM 1 의  COG Calc Signal
    // Signal Handle 은 Auto Reset 이 되도록 생성 되었으므로
    // WaitForMultipleObjects 를 빠져나가면 자동으로 UnSignal 상태로 전환된다.
    std::vector<BOOL> vecbCalc(m_nCameraNum, FALSE);
    HANDLE hCalcHandle[] = {m_hCalcWaitFirst, m_hCalcWaitSecond};

    float fFirstCalcTime = 0.f;
    float fSecondCalcTime = 0.f;
    BOOL bCalcFail = FALSE;
    float fCalcTime = 0.f;
    while (1)
    {
        // COG 계산이 끝나면 m_hGrabWaitFirst, m_hGrabWaitSecond Signal 이 On 되며 Grab Sequence 가 종료된다.
        DWORD dwResult = ::WaitForMultipleObjects(m_nCameraNum, hCalcHandle, FALSE, nCalcTimeOut);

        switch (dwResult)
        {
            // [CAM 0] COG Calc End Signal On
            case WAIT_OBJECT_0:
                //fFirstCalcTime = (float)m_CoreCPU[IGRAB::CAMERA_0]->GetTimeChk();
                vecbCalc[CAMERA_0] = TRUE;
                break;
            // [CAM 1] COG Calc Signal On
            case WAIT_OBJECT_0 + 1:
                //fSecondCalcTime = (float)m_CoreCPU[IGRAB::CAMERA_1]->GetTimeChk();
                vecbCalc[CAMERA_1] = TRUE;
                break;
            case WAIT_FAILED:
                bCalcFail = TRUE;

                strErrorLog.Format(
                    _T("GRAB FAIL, FILENAME : %s, FUNCNAME : %s, LINENO : %d"), __FILE__, __FUNCTION__, __LINE__);

                break;
            case WAIT_TIMEOUT:
                bCalcFail = TRUE;

                strErrorLog.Format(
                    _T("GRAB TIMEOUT, FILENAME : %s, FUNCNAME : %s, LINENO : %d"), __FILE__, __FUNCTION__, __LINE__);

                break;
        }

        if (vecbCalc[CAMERA_0] & vecbCalc[CAMERA_1])
        {
            // CAM 0, CAM 1 의 COG Calc 가 모두 성공한 경우
            fCalcTime = max(fFirstCalcTime, fSecondCalcTime);
            break;
        }

        if (bCalcFail)
        {
            // CAM 0, CAM 1 중 하나라도 COG Calc 가 실패한 경우
            //fCalcTime = (float)max(m_CoreCPU[IGRAB::CAMERA_0]->GetTimeChk(), m_CoreCPU[IGRAB::CAMERA_1]->GetTimeChk());

            for (long nCam = 0; nCam < m_nCameraNum; nCam++)
            {
                m_pCore[nCam]->Abort();
            }
            break;
        }
    }

    // Grab 또는 Calc 가 실패한 경우
    if (bGrabFail == TRUE || bCalcFail == TRUE)
        return FALSE;

    return TRUE;
}

UINT Framegrabber3D_iGrabXQ::GrabWaitFirst(void* pData)
{
    return ((Framegrabber3D_iGrabXQ*)pData)->GrabWaitFirst();
}

UINT Framegrabber3D_iGrabXQ::GrabWaitFirst()
{
    Ipvm::AsyncProgress progress(_T("Waiting for Grab(cam-1) done"));

    if (is_valid_index(iGRAB_BOARD_0, CAMERA_0) == false)
        return false;

    const int& id_grabber = get_grabber_id(iGRAB_BOARD_0);
    const int& id_camera = get_camera_id(id_grabber, CAMERA_0);
    const int& icf_id = get_camera_icf_id(id_grabber, id_camera);

    CString strErrorLog;
    const long nGrabTimeOut = 15000;

    DWORD ret = ::Ixq_MultiGrabProcess(id_grabber, id_camera, NULL, 0, m_profileNumber[id_grabber],
        MtgWaitOp::MTG_GRB_ISSUE, nGrabTimeOut, ScanEnd, this);

    if (ret != STATUS_GRAB_START_SUCCESS)
    {
        ::Ixq_SetXCFParam(id_grabber, id_camera, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);

        m_strErrorMessage = _T("MultiGrabProcess Fail");

        SetiGrab3D_Error_LOG();
    }
    else
    {
        SyncController::GetInstance().SetGrabExposure(TRUE);
        SyncController::GetInstance().SetGrabAcquisition(TRUE);

        DWORD dwRet = ::Ixq_MultiGrabWait(id_grabber, id_camera);

        HANDLE hThread = GetCurrentThread();
        SetThreadPriority(hThread, m_dwCurrentThreadPriority);

        ::Ixq_SetXCFParam(id_grabber, id_camera, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);

        if (dwRet == STATUS_GRAB_SUCCESS)
        {
            // 성공 로그 저장
            SetiGrab3D_Error_LOG();
            const CString logPath = DynamicSystemPath::get(DefineFolder::Log) + _T("GrabLog.csv");
            SaveGrabLogToCsv(logPath, m_siGrab3D_Error_LOG, id_grabber, id_camera, dwRet, m_profileNumber[id_grabber],
                m_scanImageCount[0]);

            SetEvent(m_hGrabWaitFirst);
        }
    }
}

UINT Framegrabber3D_iGrabXQ::GrabWaitSecond(void* pData)
{
    return ((Framegrabber3D_iGrabXQ*)pData)->GrabWaitSecond();
}

UINT Framegrabber3D_iGrabXQ::GrabWaitSecond()
{
    Ipvm::AsyncProgress progress(_T("Waiting for Grab(cam-2) done"));

    if (is_valid_index(iGRAB_BOARD_1, CAMERA_0) == false)
        return false;

    const int& id_grabber = get_grabber_id(iGRAB_BOARD_1);
    const int& id_camera = get_camera_id(id_grabber, CAMERA_0);
    const int& icf_id = get_camera_icf_id(id_grabber, id_camera);

    CString strErrorLog;
    const long nGrabTimeOut = 15000;

    DWORD ret = ::Ixq_MultiGrabProcess(id_grabber, id_camera, NULL, 0, m_profileNumber[id_grabber],
        MtgWaitOp::MTG_GRB_ISSUE, nGrabTimeOut, ScanEnd2, this);

    if (ret != STATUS_GRAB_START_SUCCESS)
    {
        ::Ixq_SetXCFParam(id_grabber, id_camera, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);

        m_strErrorMessage = _T("MultiGrabProcess Fail");

        SetiGrab3D_Error_LOG();
    }
    else
    {
        SyncController::GetInstance().SetGrabExposure(TRUE);
        SyncController::GetInstance().SetGrabAcquisition(TRUE);

        DWORD dwRet = ::Ixq_MultiGrabWait(id_grabber, id_camera);

        HANDLE hThread = GetCurrentThread();
        SetThreadPriority(hThread, m_dwCurrentThreadPriority);

        ::Ixq_SetXCFParam(id_grabber, id_camera, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);

        if (dwRet == STATUS_GRAB_SUCCESS)
        {
            // 성공 로그 저장
            //SetiGrab3D_Error_LOG();
            //const CString logPath = DynamicSystemPath::get(DefineFolder::Log) + _T("GrabLog.csv");
            //SaveGrabLogToCsv(logPath, m_siGrab3D_Error_LOG, id_grabber, id_camera, dwRet, m_profileNumber[id_grabber],
                //m_scanImageCount);

            SetEvent(m_hGrabWaitSecond);
        }
    }
}

UINT Framegrabber3D_iGrabXQ::CalcWaitFirst(LPVOID pData)
{
    Framegrabber3D_iGrabXQ* pGrabber = (Framegrabber3D_iGrabXQ*)pData;
    return pGrabber->CalcWaitFirst();
}

UINT Framegrabber3D_iGrabXQ::CalcWaitFirst()
{
    Ipvm::AsyncProgress progress(_T("Waiting for calculation done(core-1)"));
    auto coreResult = m_pCore[iGRAB_BOARD_0]->Wait();

    if (coreResult == Ipvm::Status::e_ok)
    {
        m_strErrorMessage = Ipvm::ToString(coreResult);

        const int physicalScanWidth_px = m_zmapImage->GetSizeX();
        const int physicalScanLength_px = m_zmapImage->GetSizeY();

        const int logicalScanLength_px = m_resultZmapImage[iGRAB_BOARD_0]->GetSizeX();
        const int maxStitchCount = m_resultZmapImage[iGRAB_BOARD_0]->GetSizeY() / physicalScanWidth_px;

        const int stitchIndex = max(0, min(m_resultStitchIndex, maxStitchCount - 1));

        Ipvm::Rect32s imageRect(
            0, physicalScanWidth_px * stitchIndex, logicalScanLength_px, physicalScanWidth_px * (stitchIndex + 1));

        Ipvm::Image32r childResultZmapImage(*m_resultZmapImage[iGRAB_BOARD_0], imageRect);
        Ipvm::Image16u childResultVmapImage(*m_resultVmapImage[iGRAB_BOARD_0], imageRect);

        // ZMAP 회전, REISZE 및 DISTORTION 적용
        m_distortionData->applyAndRotateCw90_zmap(*m_zmapImage, childResultZmapImage);

        // VMAP 회전, RESIZE
        if (1)
        {
            if (physicalScanLength_px != logicalScanLength_px)
            {
                Ipvm::ImageProcessingGpu::RotateCw90(*m_vmapImage, 255.f, *m_vmapResizeTempImage[iGRAB_BOARD_0]);
                Ipvm::ImageProcessing::ResizeNearestInterpolation(
                    *m_vmapResizeTempImage[iGRAB_BOARD_0], childResultVmapImage);
            }
            else
            {
                Ipvm::ImageProcessingGpu::RotateCw90(*m_vmapImage, 255.f, childResultVmapImage);
            }
        }
    }

    if (m_nDual3DGrabMode == VISION_3D_GRABMODE_INTEGRATED)
    {
        Ipvm::AsyncProgress progress(_T("Waiting for calculation done(core-3)"));
        auto coreResult2 = m_pCore[iGRAB_BOARD_2]->Wait();

        if (coreResult2 == Ipvm::Status::e_ok)
        {
            const int physicalScanWidth_px = m_zmapImage3->GetSizeX();
            const int physicalScanLength_px = m_zmapImage3->GetSizeY();

            const int logicalScanLength_px = m_resultZmapImage[iGRAB_BOARD_2]->GetSizeX();
            const int maxStitchCount = m_resultZmapImage[iGRAB_BOARD_2]->GetSizeY() / physicalScanWidth_px;

            const int stitchIndex = max(0, min(m_resultStitchIndex, maxStitchCount - 1));

            Ipvm::Rect32s imageRect(
                0, physicalScanWidth_px * stitchIndex, logicalScanLength_px, physicalScanWidth_px * (stitchIndex + 1));

            Ipvm::Image32r childResultZmapImage(*m_resultZmapImage[iGRAB_BOARD_2], imageRect);
            Ipvm::Image16u childResultVmapImage(*m_resultVmapImage[iGRAB_BOARD_2], imageRect);

            // ZMAP 회전, REISZE 및 DISTORTION 적용
            m_distortionData->applyAndRotateCw90_zmap(*m_zmapImage3, childResultZmapImage);

            // VMAP 회전, RESIZE
            if (1)
            {
                if (physicalScanLength_px != logicalScanLength_px)
                {
                    Ipvm::ImageProcessingGpu::RotateCw90(*m_vmapImage3, 255.f, *m_vmapResizeTempImage[iGRAB_BOARD_2]);
                    Ipvm::ImageProcessing::ResizeNearestInterpolation(
                        *m_vmapResizeTempImage[iGRAB_BOARD_2], childResultVmapImage);
                }
                else
                {
                    Ipvm::ImageProcessingGpu::RotateCw90(*m_vmapImage3, 255.f, childResultVmapImage);
                }
            }
        }
    }

    SetEvent(m_hCalcWaitFirst);

    return 0;
}

UINT Framegrabber3D_iGrabXQ::CalcWaitSecond(LPVOID pData)
{
    Framegrabber3D_iGrabXQ* pGrabber = (Framegrabber3D_iGrabXQ*)pData;
    return pGrabber->CalcWaitSecond();
}

UINT Framegrabber3D_iGrabXQ::CalcWaitSecond()
{
    Ipvm::AsyncProgress progress(_T("Waiting for calculation done(core-2)"));

    auto coreResult = m_pCore[iGRAB_BOARD_1]->Wait();

    if (coreResult == Ipvm::Status::e_ok)
    {
        m_strErrorMessage = Ipvm::ToString(coreResult);

        const int physicalScanWidth_px = m_zmapImage2->GetSizeX();
        const int physicalScanLength_px = m_zmapImage2->GetSizeY();

        const int logicalScanLength_px = m_resultZmapImage[iGRAB_BOARD_1]->GetSizeX();
        const int maxStitchCount = m_resultZmapImage[iGRAB_BOARD_1]->GetSizeY() / physicalScanWidth_px;

        const int stitchIndex = max(0, min(m_resultStitchIndex, maxStitchCount - 1));

        Ipvm::Rect32s imageRect(
            0, physicalScanWidth_px * stitchIndex, logicalScanLength_px, physicalScanWidth_px * (stitchIndex + 1));

        Ipvm::Image32r childResultZmapImage(*m_resultZmapImage[iGRAB_BOARD_1], imageRect);
        Ipvm::Image16u childResultVmapImage(*m_resultVmapImage[iGRAB_BOARD_1], imageRect);

        // ZMAP 회전, REISZE 및 DISTORTION 적용
        m_distortionData->applyAndRotateCw90_zmap(*m_zmapImage2, childResultZmapImage);

        // VMAP 회전, RESIZE
        if (1)
        {
            if (physicalScanLength_px != logicalScanLength_px)
            {
                Ipvm::ImageProcessingGpu::RotateCw90(*m_vmapImage2, 255.f, *m_vmapResizeTempImage[iGRAB_BOARD_1]);
                Ipvm::ImageProcessing::ResizeNearestInterpolation(
                    *m_vmapResizeTempImage[iGRAB_BOARD_1], childResultVmapImage);
            }
            else
            {
                Ipvm::ImageProcessingGpu::RotateCw90(*m_vmapImage2, 255.f, childResultVmapImage);
            }
        }
    }

    SetEvent(m_hCalcWaitSecond);

    return 0;
}

bool Framegrabber3D_iGrabXQ::wait_grab_end(const int& grabber_idx, const int& camera_idx, Ipvm::AsyncProgress* progress)
{
    const int& id_grabber = get_grabber_id(grabber_idx);
    const int& id_camera = get_camera_id(id_grabber, camera_idx);
    const int& icf_id = get_camera_icf_id(id_grabber, id_camera);

    m_strErrorMessage.Empty();
    if (progress)
    {
        CString str;
        str.Format(_T("Wait for grab(cam-%d) end"), grabber_idx);
        progress->Post(str);
    }

    //{{mc_2023.03.14_H/W Info
    //m_siGrab3D_Error_LOG.m_fCPU_Temperature = SystemConfig::GetInstance().Get_CPU_InfoData(eCPUInfoDataType::eCPUInfoDataType_CPU_Temperature);
    m_siGrab3D_Error_LOG.m_fPC_UsagePhysicalMemory = SystemConfig::GetInstance().Get_UsageMemoryData(
        eMemoryType::eMemoryType_PC, eMemoryDataType::eMemoryDataType_PhysicalMemory);
    m_siGrab3D_Error_LOG.m_fiPack_UsagePhysicalMemory = SystemConfig::GetInstance().Get_UsageMemoryData(
        eMemoryType::eMemoryType_iPack, eMemoryDataType::eMemoryDataType_PhysicalMemory);
    //}}

    //여기!! 여기서 풀기!
    DWORD dwRet = ::Ixq_MultiGrabWait(id_grabber, id_camera);

    HANDLE hThread = GetCurrentThread();
    SetThreadPriority(hThread, m_dwCurrentThreadPriority);

    ::Ixq_SetXCFParam(id_grabber, id_camera, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);

    //2023.03.08_DEV. Debug Log Test
    DWORD dwState(CAST_DWORD(-1));
    //::Ix_DbgRunState(id_grabber, id_camera, dwState);
    m_siGrab3D_Error_LOG.m_dwDbgRunState = dwState;
    //

    //2023.03.14_ATM Log Test
    if (dwState != -1)
    {
        //2023.02.23_DEV. Debug Log
        //Ix_DbgHardIntrCount(id_grabber, id_camera, g_hard_intr_count_e);
        //Ix_DbgSoftIntrCount(id_grabber, id_camera, g_soft_intr_count_e);

        m_siGrab3D_Error_LOG.m_hard_intr_count_e = g_hard_intr_count_e;
        m_siGrab3D_Error_LOG.m_soft_intr_count_e = g_soft_intr_count_e;
        //
    }
    //

    if (dwRet == STATUS_GRAB_SUCCESS)
    {
        // 성공 로그 저장
        SetiGrab3D_Error_LOG();
        const CString logPath = DynamicSystemPath::get(DefineFolder::Log) + _T("GrabLog.csv");
        SaveGrabLogToCsv(logPath, m_siGrab3D_Error_LOG, id_grabber, id_camera, dwRet, m_profileNumber[id_grabber],
            m_scanImageCount[0]);
        return true;
    }

    m_pCore[id_grabber]->Abort();

    if (id_grabber == 0 && m_nDual3DGrabMode == VISION_3D_GRABMODE_INTEGRATED)
        m_pCore[id_grabber + 2]->Abort();

    // 실패 케이스 세부 메시지 구성 기존 코드…
    SetiGrab3D_Error_LOG();

    // 실패 로그 저장
    const CString logPath = DynamicSystemPath::get(DefineFolder::Log) + _T("GrabLog.csv");
    SaveGrabLogToCsv(
        logPath, m_siGrab3D_Error_LOG, id_grabber, id_camera, dwRet, m_profileNumber[id_grabber], m_scanImageCount[0]);
    
    //2023.02.23_DEV. Debug Log
    //Ix_DbgHardIntrCount(id_grabber, id_camera, g_hard_intr_count_e);
    //Ix_DbgSoftIntrCount(id_grabber, id_camera, g_soft_intr_count_e);

    m_siGrab3D_Error_LOG.m_hard_intr_count_e = g_hard_intr_count_e;
    m_siGrab3D_Error_LOG.m_soft_intr_count_e = g_soft_intr_count_e;
    //

    //2023.03.08_DEV. Debug Log
    DWORD dwStateFail(CAST_DWORD(-1));
    /*::Ix_DbgRunState(id_grabber, id_camera, dwStateFail);
    m_siGrab3D_Error_LOG.m_dwDbgRunState = dwState;*/
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
    ::AfxMessageBox(strMessage);

    SetiGrab3D_Error_LOG();

    return false;
}

bool Framegrabber3D_iGrabXQ::wait_calc_end(
    const int& grabber_idx, const int& camera_idx, const bool& save_slit_images, Ipvm::AsyncProgress* progress)
{
    if (save_slit_images == true)
    {
        if (progress)
        {
            progress->Post(L"Scanning : Save raw images");
        }

        const unsigned int& width = get_grab_image_width(grabber_idx, camera_idx);
        const unsigned int& height = get_grab_image_height(grabber_idx, camera_idx);
        Save2DImage(grabber_idx, width, height, m_resultStitchIndex);
    }

    if (grabber_idx == 0 && m_nDual3DGrabMode == VISION_3D_GRABMODE_INTEGRATED)
    {
        if (progress)
        {
            CString str;
            str.Format(_T("Wait for calculation(core-%d) end"), grabber_idx + 2);
            progress->Post(str);
        }
        auto coreResult = m_pCore[grabber_idx + 2]->Wait();

        if (progress)
        {
            progress->Post(_T("Wait for post processing done"));
        }

        PostWaitCalcEndWork(grabber_idx + 2); //kircheis_3DCalcRetry

        if (coreResult != Ipvm::Status::e_ok)
        {
            m_strErrorMessage = Ipvm::ToString(coreResult);
            SetiGrab3D_Error_LOG();
            return false;
        }
    }
    else
    {
        if (progress)
        {
            CString str;
            str.Format(_T("Wait for calculation(core-%d) end"), grabber_idx);
            //progress->Post(_T("Wait for calculation end"));
            progress->Post(str);
        }

        auto coreResult = m_pCore[grabber_idx]->Wait();

        if (progress)
        {
            progress->Post(_T("Wait for post processing done"));
        }

        PostWaitCalcEndWork(grabber_idx); //kircheis_3DCalcRetry

        if (coreResult != Ipvm::Status::e_ok)
        {
            m_strErrorMessage = Ipvm::ToString(coreResult);
            SetiGrab3D_Error_LOG();
            return false;
        }
    }

    return true;
}

bool Framegrabber3D_iGrabXQ::re_calc_cuda_3D(const int& grabber_idx, const int& camera_idx) //kircheis_3DCalcRetry
{
    DWORD nCount = 0;
    long nBufCnt = (long)m_scanBuffers[grabber_idx].size();

    Sleep(300);
    //m_zmapImage[grabber_idx]->FillZero();
    //m_vmapImage[grabber_idx]->FillZero();
    //auto coreResult = m_pCore[grabber_idx]->ProcessAsync(NULL, *m_zmapImage[grabber_idx], *m_vmapImage[grabber_idx]); //kircheis_SDK127

    if (grabber_idx == 0)
    {
        m_zmapImage->FillZero();
        m_vmapImage->FillZero();
        auto coreResult = m_pCore[grabber_idx]->ProcessAsync(NULL, *m_zmapImage, *m_vmapImage); //kircheis_SDK127

        if (coreResult != Ipvm::Status::e_ok)
        {
            m_strErrorMessage = Ipvm::ToString(coreResult);
            SetiGrab3D_Error_LOG();
            return false;
        }
    }
    else if (grabber_idx == 1)
    {
        m_zmapImage2->FillZero();
        m_vmapImage2->FillZero();
        auto coreResult = m_pCore[grabber_idx]->ProcessAsync(NULL, *m_zmapImage2, *m_vmapImage2); //kircheis_SDK127

        if (coreResult != Ipvm::Status::e_ok)
        {
            m_strErrorMessage = Ipvm::ToString(coreResult);
            SetiGrab3D_Error_LOG();
            return false;
        }
    }

    for (long nIdx = 0; nIdx < nBufCnt; nIdx++)
    {
        nCount++;
        m_scanImageCount[0] = (long)nCount;
        Ipvm::SbtCore::CallbackAfterFrameUpdated(nCount, m_pCore[grabber_idx]);
    }

    auto coreWaitResult = m_pCore[grabber_idx]->Wait();

    PostWaitCalcEndWork(grabber_idx); //kircheis_3DCalcRetry

    if (coreWaitResult != Ipvm::Status::e_ok)
    {
        m_strErrorMessage = Ipvm::ToString(coreWaitResult);
        SetiGrab3D_Error_LOG();
        return false;
    }

    return true;
}

void Framegrabber3D_iGrabXQ::PostWaitCalcEndWork(const int& grabber_idx) //kircheis_3DCalcRetry
{
    TRACE(_T("PostWaitCalcEndWork : %d"), grabber_idx);

    if (grabber_idx == 0)
    {
        const int physicalScanWidth_px = m_zmapImage->GetSizeX();
        const int physicalScanLength_px = m_zmapImage->GetSizeY();

        const int logicalScanLength_px = m_resultZmapImage[grabber_idx]->GetSizeX();
        const int maxStitchCount = m_resultZmapImage[grabber_idx]->GetSizeY() / physicalScanWidth_px;

        const int stitchIndex = max(0, min(m_resultStitchIndex, maxStitchCount - 1));

        Ipvm::Rect32s imageRect(
            0, physicalScanWidth_px * stitchIndex, logicalScanLength_px, physicalScanWidth_px * (stitchIndex + 1));

        //const float heightRangeMin = -0.5f * m_pCore->GetHeightRange_um(*m_pCoreParam);
        //const float heightRangeMax = 0.5f * m_pCore->GetHeightRange_um(*m_pCoreParam);

        Ipvm::Image32r childResultZmapImage(*m_resultZmapImage[grabber_idx], imageRect);
        Ipvm::Image16u childResultVmapImage(*m_resultVmapImage[grabber_idx], imageRect);

        // ZMAP 회전, REISZE 및 DISTORTION 적용
        m_distortionData->applyAndRotateCw90_zmap(*m_zmapImage, childResultZmapImage);

        // VMAP 회전, RESIZE
        if (1)
        {
            if (physicalScanLength_px != logicalScanLength_px)
            {
                Ipvm::ImageProcessingGpu::RotateCw90(*m_vmapImage, 255.f, *m_vmapResizeTempImage[grabber_idx]);
                Ipvm::ImageProcessing::ResizeNearestInterpolation(
                    *m_vmapResizeTempImage[grabber_idx], childResultVmapImage);
            }
            else
            {
                Ipvm::ImageProcessingGpu::RotateCw90(*m_vmapImage, 255.f, childResultVmapImage);
            }
        }

        if (m_nDual3DGrabMode == VISION_3D_GRABMODE_INTEGRATED)
        {
            const int physicalScanWidth_px = m_zmapImage3->GetSizeX();
            const int physicalScanLength_px = m_zmapImage3->GetSizeY();

            const int logicalScanLength_px = m_resultZmapImage[grabber_idx + 2]->GetSizeX();
            const int maxStitchCount = m_resultZmapImage[grabber_idx + 2]->GetSizeY() / physicalScanWidth_px;

            const int stitchIndex = max(0, min(m_resultStitchIndex, maxStitchCount - 1));

            Ipvm::Rect32s imageRect(
                0, physicalScanWidth_px * stitchIndex, logicalScanLength_px, physicalScanWidth_px * (stitchIndex + 1));

            //const float heightRangeMin = -0.5f * m_pCore->GetHeightRange_um(*m_pCoreParam);
            //const float heightRangeMax = 0.5f * m_pCore->GetHeightRange_um(*m_pCoreParam);

            Ipvm::Image32r childResultZmapImage(*m_resultZmapImage[grabber_idx + 2], imageRect);
            Ipvm::Image16u childResultVmapImage(*m_resultVmapImage[grabber_idx + 2], imageRect);

            // ZMAP 회전, REISZE 및 DISTORTION 적용
            m_distortionData->applyAndRotateCw90_zmap(*m_zmapImage3, childResultZmapImage);

            // VMAP 회전, RESIZE
            if (1)
            {
                if (physicalScanLength_px != logicalScanLength_px)
                {
                    Ipvm::ImageProcessingGpu::RotateCw90(*m_vmapImage3, 255.f, *m_vmapResizeTempImage[grabber_idx + 2]);
                    Ipvm::ImageProcessing::ResizeNearestInterpolation(
                        *m_vmapResizeTempImage[grabber_idx + 2], childResultVmapImage);
                }
                else
                {
                    Ipvm::ImageProcessingGpu::RotateCw90(*m_vmapImage3, 255.f, childResultVmapImage);
                }
            }
        }
    }
    else if (grabber_idx == 1)
    {
        const int physicalScanWidth_px = m_zmapImage2->GetSizeX();
        const int physicalScanLength_px = m_zmapImage2->GetSizeY();

        const int logicalScanLength_px = m_resultZmapImage[grabber_idx]->GetSizeX();
        const int maxStitchCount = m_resultZmapImage[grabber_idx]->GetSizeY() / physicalScanWidth_px;

        const int stitchIndex = max(0, min(m_resultStitchIndex, maxStitchCount - 1));

        Ipvm::Rect32s imageRect(
            0, physicalScanWidth_px * stitchIndex, logicalScanLength_px, physicalScanWidth_px * (stitchIndex + 1));

        //const float heightRangeMin = -0.5f * m_pCore->GetHeightRange_um(*m_pCoreParam);
        //const float heightRangeMax = 0.5f * m_pCore->GetHeightRange_um(*m_pCoreParam);

        Ipvm::Image32r childResultZmapImage(*m_resultZmapImage[grabber_idx], imageRect);
        Ipvm::Image16u childResultVmapImage(*m_resultVmapImage[grabber_idx], imageRect);

        // ZMAP 회전, REISZE 및 DISTORTION 적용
        m_distortionData->applyAndRotateCw90_zmap(*m_zmapImage2, childResultZmapImage);

        // VMAP 회전, RESIZE
        if (1)
        {
            if (physicalScanLength_px != logicalScanLength_px)
            {
                Ipvm::ImageProcessingGpu::RotateCw90(*m_vmapImage2, 255.f, *m_vmapResizeTempImage[grabber_idx]);
                Ipvm::ImageProcessing::ResizeNearestInterpolation(
                    *m_vmapResizeTempImage[grabber_idx], childResultVmapImage);
            }
            else
            {
                Ipvm::ImageProcessingGpu::RotateCw90(*m_vmapImage2, 255.f, childResultVmapImage);
            }
        }
    }
}



bool Framegrabber3D_iGrabXQ::set_exposure_time(
    const int& grabber_idx, const int& camera_idx, const double& exposureHigh_ms, double& validFramePeriod_ms)
{
    // 아래 테이블은 뷰웍스 매뉴얼 참조함.
    //	Line	FPS
    Ipvm::Point32r2 data[] = {
        {500.f, 1000.f / 1800.f}, // ~1800 fps
        {1000.f, 1000.f / 950.f}, // ~950 fps
        {1500.f, 1000.f / 650.f}, // ~650 fps
        {2000.f, 1000.f / 480.f}, // ~480 fps
        {2500.f, 1000.f / 380.f}, // ~380 fps
        {3072.f, 1000.f / 305.f}, // ~305 fps (4096x3072 spec 근사)
        {4096.f, 1000.f / 230.f}, // ~230 fps (Full Height)
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

    BOOL bResult = ::Ixq_SetXCFExposureTime(id_grabber, id_camera, id_icf, float(desirableExposureTime_ms), 0.001f);

    return true;
}

bool Framegrabber3D_iGrabXQ::set_slitbeam_parameters(const int& grabber_idx, const int& camera_idx,
    const SlitBeam3DParameters& slitbeamPara, const float& height_scaling, const bool& use_height_quantization,
    const bool& use_distortion_compensation)
{
    Ipvm::AsyncProgress progress(_T("Setup slitbeam GPU core"));

    m_nDual3DGrabMode = slitbeamPara.m_cameraGrabMode;

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

    /*
    const int& width = get_grab_image_width(grabber_idx, camera_idx);
    const int& height = get_grab_image_height(grabber_idx, camera_idx);
    */
    const int& width = get_grab_image_width(grabber_idx, camera_idx);
    const int& height = get_grab_image_height(grabber_idx, camera_idx);

    live_buffer_mono->Create(width, height);
    live_buffer_mono2->Create(width, height);

    m_n2DRawImageSizeXY = width * height;
    const long oldProfileNumber = m_profileNumber[id_grabber];
    
    if (id_grabber == 0 && slitbeamPara.m_cameraGrabMode == VISION_3D_GRABMODE_INTEGRATED)
    {
        m_scanBuffers[id_grabber].resize(max(physicalScanLength_px / 2, m_scanBuffers[id_grabber].size() / 2));

        for (long idx = 0; idx < m_scanBuffers[id_grabber].size(); idx++)
        {
            auto& image = m_scanBuffers[id_grabber][idx];

            if (image.GetSizeY() < height)
            {
                if (image.Create(width, height, true) != Ipvm::Status::e_ok)
                {
                    ::AfxMessageBox(_T("image Create 실패!!!"));
                    image.FillZero();
                }
            }
        }

        m_scanBuffers[id_grabber + 2].resize(max(physicalScanLength_px / 2, m_scanBuffers[id_grabber + 2].size() / 2));

        for (long idx = 0; idx < m_scanBuffers[id_grabber + 2].size() ; idx++)
        {
            auto& image = m_scanBuffers[id_grabber+2][idx];

            if (image.GetSizeY() < height)
            {
                if (image.Create(width, height, true) != Ipvm::Status::e_ok)
                {
                    ::AfxMessageBox(_T("image Create 실패!!!"));
                    image.FillZero();
                }
            }
        }
    }
    else
    {
        m_scanBuffers[id_grabber].resize(max(physicalScanLength_px, m_scanBuffers[id_grabber].size()));

        for (long idx = 0; idx < physicalScanLength_px; idx++)
        {
            auto& image = m_scanBuffers[id_grabber][idx];

            if (image.GetSizeY() < height)
            {
                if (image.Create(width, height, true) != Ipvm::Status::e_ok)
                {
                    ::AfxMessageBox(_T("image Create 실패!!!"));
                    image.FillZero();
                }
            }
        }
    }

    if (physicalScanLength_px != oldProfileNumber)
    {
        if (id_grabber == 0)
        {
            if (m_zmapImage->Create(width, physicalScanLength_px, true) != Ipvm::Status::e_ok)
            {
                ::AfxMessageBox(_T("m_zmapImage create 실패!!!"));
                m_zmapImage->FillZero();
            }
            if (m_vmapImage->Create(width, physicalScanLength_px, true) != Ipvm::Status::e_ok)
            {
                ::AfxMessageBox(_T("m_vmapImage create 실패!!!"));
                m_vmapImage->FillZero();
            }
        }
        else if (id_grabber == 1)
        {
            if (m_zmapImage2->Create(width, physicalScanLength_px, true) != Ipvm::Status::e_ok)
            {
                ::AfxMessageBox(_T("m_zmapImage create 실패!!!"));
                m_zmapImage2->FillZero();
            }
            if (m_vmapImage2->Create(width, physicalScanLength_px, true) != Ipvm::Status::e_ok)
            {
                ::AfxMessageBox(_T("m_vmapImage create 실패!!!"));
                m_vmapImage2->FillZero();
            }
        }
    }

    if (id_grabber == 0 && slitbeamPara.m_cameraGrabMode == VISION_3D_GRABMODE_INTEGRATED)
    {
        if (m_zmapImage3->Create(width, physicalScanLength_px, true) != Ipvm::Status::e_ok)
        {
            ::AfxMessageBox(_T("m_zmapImage create 실패!!!"));
            m_zmapImage3->FillZero();
        }
        if (m_vmapImage3->Create(width, physicalScanLength_px, true) != Ipvm::Status::e_ok)
        {
            ::AfxMessageBox(_T("m_vmapImage create 실패!!!"));
            m_vmapImage3->FillZero();
        }
    }

    m_vmapResizeTempImage[grabber_idx]->Create(physicalScanLength_px, width);

    m_profileNumber[id_grabber] = physicalScanLength_px;

    /*
    m_distortionData->ready(slitbeamPara.m_scanDepth_um, slitbeamPara.m_cameraVerticalBinning ? true : false,
        m_zmapImage[grabber_idx]->GetSizeY(), m_zmapImage[grabber_idx]->GetSizeX(), use_distortion_compensation);
        */

    if (id_grabber == 0)
    {
        m_distortionData->ready(slitbeamPara.m_scanDepth_um, slitbeamPara.m_cameraVerticalBinning ? true : false,
            m_zmapImage->GetSizeY(), m_zmapImage->GetSizeX(), use_distortion_compensation);
    }
    else if (id_grabber == 1)
    {
        m_distortionData->ready(slitbeamPara.m_scanDepth_um, slitbeamPara.m_cameraVerticalBinning ? true : false,
            m_zmapImage2->GetSizeY(), m_zmapImage2->GetSizeX(), use_distortion_compensation);
    }

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
    m_pCoreParam->m_imageBuffers = m_scanBuffers[id_grabber].data();
    m_pCoreParam->m_imageBufferCount = physicalScanLength_px;
    m_pCoreParam->m_dsnuBuffer = &dsnuImage;
    m_pCoreParam->m_saturationIntensity = 255;
    //m_pCoreParam->m_distortionProfile = m_distortionData->getDistortionProfile();
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
    m_pCoreParam->m_frameUpdateTimeout_msec = 20000;

    if (id_grabber == 0 && slitbeamPara.m_cameraGrabMode == VISION_3D_GRABMODE_INTEGRATED)
    {
        m_pCoreParam->m_scanLength = physicalScanLength_px / 2;
        m_pCoreParam->m_imageBuffers = m_scanBuffers[id_grabber].data();
        m_pCoreParam->m_imageBufferCount = physicalScanLength_px / 2;
        auto coreResult = m_pCore[id_grabber]->Setup(*m_pCoreParam);

        if (coreResult != Ipvm::Status::e_ok)
        {
            ::AfxMessageBox(CString(Ipvm::ToString(coreResult)));
            return true; //kircheis_3DGrabIssue
        }

        m_pCoreParam->m_scanLength = physicalScanLength_px / 2;
        m_pCoreParam->m_imageBuffers = m_scanBuffers[id_grabber + 2].data();
        m_pCoreParam->m_imageBufferCount = physicalScanLength_px / 2;
        coreResult = m_pCore[id_grabber + 2]->Setup(*m_pCoreParam);

        if (coreResult != Ipvm::Status::e_ok)
        {
            ::AfxMessageBox(CString(Ipvm::ToString(coreResult)));
            return true; //kircheis_3DGrabIssue
        }
    }
    else
    {
        m_pCoreParam->m_imageBuffers = m_scanBuffers[id_grabber].data();
        m_pCoreParam->m_imageBufferCount = physicalScanLength_px ;
        auto coreResult = m_pCore[id_grabber]->Setup(*m_pCoreParam);

        if (coreResult != Ipvm::Status::e_ok)
        {
            ::AfxMessageBox(CString(Ipvm::ToString(coreResult)));
            return true; //kircheis_3DGrabIssue
        }
    }

    return bIsSuccessedChangeVerticalSize; //kircheis_3DGrabIssue
}

void Framegrabber3D_iGrabXQ::set_live_image_size(
    const int& grabber_idx, const int& camera_idx, const bool& use_binning, const bool& is_full_size)
{
    const int& id_grabber = get_grabber_id(grabber_idx);
    const int& id_camera = get_camera_id(id_grabber, camera_idx);
    const int& id_icf = get_camera_icf_id(id_grabber, id_camera);

    ChangeVerticalSize(
        id_grabber, id_camera, id_icf, use_binning, is_full_size, get_grab_image_height(grabber_idx, camera_idx));
}

long Framegrabber3D_iGrabXQ::get_grab_image_height_3D(
    const double& scan_depth_um, const bool& use_binning, const double& height_scaling) const
{
    const float heightResolution_px2um = get_height_resolution_px2um(use_binning, CAST_FLOAT(height_scaling), true);

    const long grabImageHeight = CAST_LONG(::ceil(scan_depth_um / heightResolution_px2um));

    const long validGrabImageHeight = Ipvm::Image::CalcWidthBytes(grabImageHeight, 8, 4);

    return validGrabImageHeight;
}

bool Framegrabber3D_iGrabXQ::load_dsnu_image(
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

bool Framegrabber3D_iGrabXQ::make_dsnu_image(const int& grabber_id, const int& camera_id, const int& icf_id,
    int& custom_grab_count, Ipvm::Image8u& dsnu_image, Ipvm::Image8u& dsnu_binning_image, const CString& camera_serial)
{
    Ipvm::AsyncProgress progress(_T("Make DSNU Image"));

    ChangeVerticalSize(grabber_id, camera_id, icf_id, false, true, 0);

    ::Ixq_SetCustomGrabEvent(grabber_id, camera_id, 1);
    float exposureHigh_ms = 0.f;
    float exposureLow_ms = 0.f;
    ::Ixq_GetXCFExposureTime(grabber_id, camera_id, icf_id, exposureHigh_ms, exposureLow_ms);
    ::Ixq_SetXCFExposureTime(grabber_id, camera_id, icf_id, exposureHigh_ms, 90.f);
    ::Ixq_SetXCFParam(grabber_id, camera_id, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::UplinkTrigger);
    ::Ixq_SetXCFParam(grabber_id, camera_id, icf_id, XCF_CTL_TRIGGER_TYPE, _enumTriggerType::FG_Trigger);

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
            != ::Ixq_MultiGrabProcess(grabber_id, camera_id, &address, 1, 1, MtgWaitOp::MTG_END_WAIT, 1000, NULL, NULL))
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
    ::Ixq_GetCustomGrabEvent(grabber_id, camera_id, &nCustEvtNum, &custom_grab_count);
    ::Ixq_SetCustomGrabEvent(grabber_id, camera_id, custom_grab_count);
    ::Ixq_SetXCFParam(grabber_id, camera_id, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);
    ::Ixq_SetXCFParam(grabber_id, camera_id, icf_id, XCF_CTL_TRIGGER_TYPE, _enumTriggerType::HardwareTrigger);
    ::Ixq_SetXCFExposureTime(grabber_id, camera_id, icf_id, exposureHigh_ms, exposureLow_ms);

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

bool Framegrabber3D_iGrabXQ::ChangeVerticalSize(const int& grabber_id, const int& camera_id, const int& icf_id,
    const bool cameraVerticalBinning, const bool fullSizeY, const long scanSizeY)
{
    Ipvm::AsyncProgress progress(_T("Modify camera parameters"));

    int nCustEvtNum(0);
    ::Ixq_SetCustomGrabEvent(grabber_id, camera_id, 1);
    ::Ixq_GetCustomGrabEvent(grabber_id, camera_id, &nCustEvtNum, &custom_grab_event_counts[grabber_id][camera_id]);

    UINT uiImageFullSizeY = DEFAULT_IMAGE_FULL_SIZE_Y / (cameraVerticalBinning ? 2 : 1);
    set_grab_image_height(grabber_id, camera_id, fullSizeY ? uiImageFullSizeY : scanSizeY);
    m_use_binning[grabber_id][camera_id] = cameraVerticalBinning;

    // Camera Open...
    //HANDLE handle = ::ctrlOpen(camera_id);
    HANDLE handle = ::ctrlOpen(grabber_id);

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

    if (SystemConfig::GetInstance().Get3DVisionCameraType() == CAMERATYPE_MIKROTRON_EOSENS_21CXP2)
    {
        ulAddressAcqStart = 0x8204;
        ulAddressAcqStop = 0x8208;
        ulAddressOffsetY = 0x8804;
        ulAddressHeight = 0x811c;
    }

    // Communication Status Check
    long nTrue = 1;
    long nFalse = 0;

    BYTE ret = 0;
    DWORD dwTimeOutMS = cameraVerticalBinning ? 4000 : 1000;
    // Acq Stop
    ret = ::ctrlCmdMsg(handle, 1, sizeof(long), ulAddressAcqStart, (BYTE*)&nFalse, 1000);
    Sleep(CAM_CAMMAND_SLEEP);
    ret = ::ctrlCmdMsg(handle, 1, sizeof(long), ulAddressAcqStop, (BYTE*)&nTrue, 1000);
    Sleep(CAM_CAMMAND_SLEEP);

    // Write binning
    UINT uiBinning = cameraVerticalBinning ? 2 : 1;
    ret = ::ctrlCmdMsg(handle, 1, sizeof(uiBinning), ulAddressBinningY, (BYTE*)&uiBinning, dwTimeOutMS);
    Sleep(CAM_CAMMAND_SLEEP);

    UINT nNewOffset = 0;
    if (fullSizeY == true)
    {
        if (SystemConfig::GetInstance().Get3DVisionCameraType() == CAMERATYPE_MIKROTRON_EOSENS_21CXP2)
        {
            int change_offset_Y = 0;
            set_grab_image_offset_Y(grabber_id, camera_id, change_offset_Y);

            ret = ::ctrlCmdMsg(handle, 1, sizeof(nNewOffset), ulAddressOffsetY, (BYTE*)&nNewOffset, dwTimeOutMS);
            Sleep(CAM_CAMMAND_SLEEP);
            
            ret = ::ctrlCmdMsg(
                handle, 1, sizeof(uiImageFullSizeY), ulAddressHeight, (BYTE*)&uiImageFullSizeY, dwTimeOutMS);
            Sleep(CAM_CAMMAND_SLEEP);
        }
        else
        {
            // Write offset
            int change_offset_Y = 0;
            set_grab_image_offset_Y(grabber_id, camera_id, change_offset_Y);

            ret = ::ctrlCmdMsg(handle, 1, sizeof(get_grab_image_offset_Y(grabber_id, camera_id)), ulAddressOffsetY,
                (BYTE*)&change_offset_Y, dwTimeOutMS);

            set_grab_image_offset_Y(grabber_id, camera_id, change_offset_Y);

            Sleep(10);
            // Write height
            auto height = get_grab_image_height(grabber_id, camera_id);
            ret = ::ctrlCmdMsg(handle, 1, sizeof(get_grab_image_height(grabber_id, camera_id)), ulAddressHeight,
                (BYTE*)&height, dwTimeOutMS);

            set_grab_image_height(grabber_id, camera_id, height);
        }
    }
    else
    {
        // Write height
        //auto height = get_grab_image_height(grabber_id, camera_id);
        auto height = 224; // TODO: 여기서 height 값을 계산할 필요. 연구소에서 236이 안된다고 함.
        //long nCurImageHeight = m_grabImageHeight;
        ret = ::ctrlCmdMsg(handle, 1, sizeof(height), ulAddressHeight, (BYTE*)&height, dwTimeOutMS);
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
        ret = ::ctrlCmdMsg(handle, 1, sizeof(offset_Y), ulAddressOffsetY, (BYTE*)&offset_Y, dwTimeOutMS);
    }

    // Acq Start
    ret = ::ctrlCmdMsg(handle, 1, sizeof(long), ulAddressAcqStop, (BYTE*)&nFalse, 1000);
    Sleep(CAM_CAMMAND_SLEEP);
    ret = ::ctrlCmdMsg(handle, 1, sizeof(long), ulAddressAcqStart, (BYTE*)&nTrue, 1000);
    Sleep(CAM_CAMMAND_SLEEP);
    // Close Device...
    ret = CAST_BYTE(::ctrlClose(handle));

    // 이미지 사이즈 설정
    ::Ixq_SetCustomGrabEvent(grabber_id, camera_id, 1);
    ::Ixq_SetXCFParam(
        grabber_id, camera_id, icf_id, XCF_CTL_VERTICAL_SIZE, get_grab_image_height(grabber_id, camera_id));
    //{{//kircheis_3DGrabIssue
    unsigned int nGrabImageHeight = 0;
    Sleep(CAM_CAMMAND_SLEEP);
    ::Ixq_GetXCFParam(grabber_id, camera_id, icf_id, XCF_CTL_VERTICAL_SIZE, &nGrabImageHeight);
    if (CAST_UINT(get_grab_image_height(grabber_id, camera_id)) != nGrabImageHeight)
        return false;
    //}}

    ::Ixq_GetCustomGrabEvent(grabber_id, camera_id, &nCustEvtNum, &custom_grab_event_counts[grabber_id][camera_id]);
    ::Ixq_SetCustomGrabEvent(grabber_id, camera_id, custom_grab_event_counts[grabber_id][camera_id]);

    live_buffer_mono->Create(get_grab_image_width(grabber_id, camera_id), get_grab_image_height(grabber_id, camera_id));
    live_buffer_mono2->Create(get_grab_image_width(grabber_id, camera_id), get_grab_image_height(grabber_id, camera_id));

    return true;
}

void Framegrabber3D_iGrabXQ::ScanEnd(void* pUserData, DWORD nCount, void* pParam1, void* pParam2)
{
    UNREFERENCED_PARAMETER(pParam2);

    ((Framegrabber3D_iGrabXQ*)pUserData)->ScanEnd(nCount, (BYTE*)pParam1);
}

void Framegrabber3D_iGrabXQ::ScanEnd(DWORD nCount, BYTE* pDmaBuffer)
{

    if (m_nDual3DGrabMode == VISION_3D_GRABMODE_INTEGRATED)
    {
        --nCount;
        if (nCount & 1)
        {
            int scanImageCount = nCount * 0.5;
            m_scanImageCount[0] = scanImageCount;
            memcpy(m_scanBuffers[2][scanImageCount].GetMem(), pDmaBuffer, m_n2DRawImageSizeXY);
            Ipvm::SbtCore::CallbackAfterFrameUpdated(scanImageCount + 1, m_pCore[2]);
        }
        else
        {
            int scanImageCount = nCount * 0.5;
            m_scanImageCount[0] = scanImageCount;
            memcpy(m_scanBuffers[0][scanImageCount].GetMem(), pDmaBuffer, m_n2DRawImageSizeXY);
            Ipvm::SbtCore::CallbackAfterFrameUpdated(scanImageCount + 1, m_pCore[0]);
        }
    }
    else
    {
        memcpy(m_scanBuffers[0][nCount - 1].GetMem(), pDmaBuffer, m_n2DRawImageSizeXY);
        m_scanImageCount[0] = nCount;
        Ipvm::SbtCore::CallbackAfterFrameUpdated(nCount, m_pCore[0]);
    }
}

void Framegrabber3D_iGrabXQ::ScanEnd2(void* pUserData, DWORD nCount, void* pParam1, void* pParam2)
{
    UNREFERENCED_PARAMETER(pParam2);

    ((Framegrabber3D_iGrabXQ*)pUserData)->ScanEnd2(nCount, (BYTE*)pParam1);
}

void Framegrabber3D_iGrabXQ::ScanEnd2(DWORD nCount, BYTE* pDmaBuffer)
{
    //2023.02.23_DEV. Debug Log
    g_cb_call_count++;
    g_cb_sum_count = (int)max(g_cb_sum_count, nCount);

    m_siGrab3D_Error_LOG.m_cb_call_count = g_cb_call_count;
    m_siGrab3D_Error_LOG.m_cb_sum_count = g_cb_sum_count;
    //

    BYTE* pCpyImageBuffer = m_scanBuffers[1][nCount - 1].GetMem();

    const long frameSize = get_grab_image_width(1, 0) * get_grab_image_height(1, 0);

    memcpy(pCpyImageBuffer, pDmaBuffer, frameSize);

    m_scanImageCount[1] = nCount;

    //Ipvm::SbtCore::CallbackAfterFrameUpdated(nCount, m_pCore);
    Ipvm::SbtCore::CallbackAfterFrameUpdated(nCount, m_pCore[1]);
}

void Framegrabber3D_iGrabXQ::LiveEnd(void* pUserData, DWORD nCount, void* pParam1, void* pParam2)
{
    UNREFERENCED_PARAMETER(pParam2);

    ((Framegrabber3D_iGrabXQ*)pUserData)->LiveEnd(nCount, (BYTE*)pParam1);
}

void Framegrabber3D_iGrabXQ::LiveEnd(DWORD nCount, BYTE* pDmaBuffer)
{
    if (nCount > 1)
    {
        return;
    }

    memcpy(live_buffer_mono->GetMem(), pDmaBuffer, live_buffer_mono->GetWidthBytes() * live_buffer_mono->GetSizeY());
}

void Framegrabber3D_iGrabXQ::LiveEnd2(void* pUserData, DWORD nCount, void* pParam1, void* pParam2)
{
    UNREFERENCED_PARAMETER(pParam2);

    ((Framegrabber3D_iGrabXQ*)pUserData)->LiveEnd2(nCount, (BYTE*)pParam1);
}

void Framegrabber3D_iGrabXQ::LiveEnd2(DWORD nCount, BYTE* pDmaBuffer)
{
    if (nCount > 1)
    {
        return;
    }

    memcpy(live_buffer_mono2->GetMem(), pDmaBuffer, live_buffer_mono2->GetWidthBytes() * live_buffer_mono2->GetSizeY());
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

void Framegrabber3D_iGrabXQ::Save2DImage(
    const int& grabber_idx, const unsigned int& image_width, const unsigned int& image_height, const long splitSetIndex)
{
    CString strDirectory;
    
    strDirectory.Format(_T("%sSlitbeamScanImage_%01d_%01d\\"), LPCTSTR(DynamicSystemPath::get(DefineFolder::Image)),
        splitSetIndex, grabber_idx);

    ::CreateDirectory(strDirectory, NULL);

    strDirectory += CTime::GetCurrentTime().Format("%Y%m%d_%H%M%S");

    ::CreateDirectory(strDirectory, NULL);

    static const long threadCount = 6;

    ThreadArgs args = {LPCTSTR(strDirectory), CAST_LONG(image_width), CAST_LONG(image_height),
        m_scanBuffers[grabber_idx].data(), 0, threadCount, m_profileNumber[grabber_idx]};

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

bool Framegrabber3D_iGrabXQ::LiveCallback(const int& grabber_id, const int& camera_id)
{
    CSingleLock lock(&g_cs, TRUE);

    ::Ixq_SetCustomGrabEvent(grabber_id, camera_id, 1);
    float exposureHigh_ms = 0.f;
    float exposureLow_ms = 0.f;
    const int& icf_id = get_camera_icf_id(grabber_id, camera_id);
    ::Ixq_GetXCFExposureTime(grabber_id, camera_id, icf_id, exposureHigh_ms, exposureLow_ms);
    ::Ixq_SetXCFExposureTime(grabber_id, camera_id, icf_id, exposureHigh_ms, 90.f);
    ::Ixq_SetXCFParam(grabber_id, camera_id, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::UplinkTrigger);
    ::Ixq_SetXCFParam(grabber_id, camera_id, icf_id, XCF_CTL_TRIGGER_TYPE, _enumTriggerType::FG_Trigger);

    auto ret1 = -1;
    if (grabber_id == 0)
        ret1 = ::Ixq_MultiGrabProcess(
            grabber_id, camera_id, nullptr, 0, 1, MtgWaitOp::MTG_GRB_ISSUE, 1000, LiveEnd, this);
    else if (grabber_id == 1)
        ret1 = ::Ixq_MultiGrabProcess(
            grabber_id, camera_id, nullptr, 0, 1, MtgWaitOp::MTG_GRB_ISSUE, 1000, LiveEnd2, this);

    if (STATUS_GRAB_START_SUCCESS == ret1)
    {
        auto ret2 = ::Ixq_MultiGrabWait(grabber_id, camera_id);

        int nCustEvtNum = 0;
        ::Ixq_GetCustomGrabEvent(grabber_id, camera_id, &nCustEvtNum, &custom_grab_event_counts[grabber_id][camera_id]);
        ::Ixq_SetCustomGrabEvent(grabber_id, camera_id, custom_grab_event_counts[grabber_id][camera_id]);
        ::Ixq_SetXCFParam(grabber_id, camera_id, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);
        ::Ixq_SetXCFParam(grabber_id, camera_id, icf_id, XCF_CTL_TRIGGER_TYPE, _enumTriggerType::HardwareTrigger);
        ::Ixq_SetXCFExposureTime(grabber_id, camera_id, icf_id, exposureHigh_ms, exposureLow_ms);

        const int offset_Y = get_grab_image_offset_Y(grabber_id, camera_id);
        const int image_height = get_grab_image_height(grabber_id, camera_id);

        if (m_use_binning[grabber_id][camera_id] == true)
        {
            Ipvm::Image8u roiImage(dsnu_binning_images[grabber_id][camera_id],
                Ipvm::Rect32s(0, offset_Y, DEFAULT_IMAGE_SIZE_X, offset_Y + image_height));

            Ipvm::ImageProcessing::Subtract(roiImage, Ipvm::Rect32s(roiImage), 0, *live_buffer_mono);
        }
        else
        {
            Ipvm::Image8u roiImage(dsnu_images[grabber_id][camera_id],
                Ipvm::Rect32s(0, offset_Y, DEFAULT_IMAGE_SIZE_X, offset_Y + image_height));

            Ipvm::ImageProcessing::Subtract(roiImage, Ipvm::Rect32s(roiImage), 0, *live_buffer_mono);
        }

        return STATUS_GRAB_SUCCESS == ret2;
    }
    else
    {
        ::Ixq_GrabStop(grabber_id, camera_id);

        int nCustEvtNum = 0;
        ::Ixq_GetCustomGrabEvent(grabber_id, camera_id, &nCustEvtNum, &custom_grab_event_counts[grabber_id][camera_id]);
        ::Ixq_SetCustomGrabEvent(grabber_id, camera_id, custom_grab_event_counts[grabber_id][camera_id]);
        ::Ixq_SetXCFParam(grabber_id, camera_id, icf_id, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);
        ::Ixq_SetXCFParam(grabber_id, camera_id, icf_id, XCF_CTL_TRIGGER_TYPE, _enumTriggerType::HardwareTrigger);
        ::Ixq_SetXCFExposureTime(grabber_id, camera_id, icf_id, exposureHigh_ms, exposureLow_ms);

        return false;
    }

    return true;
}

float Framegrabber3D_iGrabXQ::get_height_resolution_px2um(
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

long Framegrabber3D_iGrabXQ::get_scan_length_px() const
{
    return m_profileNumber[0];
}

long Framegrabber3D_iGrabXQ::get_scan_count() const
{
    return m_scanImageCount[0];
}

void Framegrabber3D_iGrabXQ::init_slit_beam_distortion()
{
    m_distortionData->reloadProfile();
    m_distortionData->resetSelectDistortionFull();
}

CString Framegrabber3D_iGrabXQ::Get3DGrabErrorMessage()
{
    CString strReturn = m_strErrorMessage;
    m_strErrorMessage.Empty();
    return strReturn;
}

void Framegrabber3D_iGrabXQ::SetiGrab3D_Error_LOG()
{
    m_siGrab3D_Error_LOG.m_strPacketError = GetCxpLinkfPacketError(grabber_id_use_grab, camera_id_use_grab);
    m_siGrab3D_Error_LOG.m_strCxpLinkCableError = GetCurrentEnable_Channel_Connection_State(grabber_id_use_grab);
    m_siGrab3D_Error_LOG.m_nExternalTrigger = GetReciveExternalTrigger(grabber_id_use_grab, camera_id_use_grab);
    m_siGrab3D_Error_LOG.m_strPageTrigger = GetPageTrigger_State(grabber_id_use_grab, camera_id_use_grab);
    m_siGrab3D_Error_LOG.m_nReciveFrametoCamera = GetReciveFrameCount2Camera(grabber_id_use_grab, camera_id_use_grab);
    m_siGrab3D_Error_LOG.m_nCustomGrabEventCount = custom_grab_event_counts[grabber_id_use_grab][camera_id_use_grab];
}

bool Framegrabber3D_iGrabXQ::Check3DNoise(const unsigned int& grabber_id, const unsigned int& camera_id, BYTE** ppbyImg,
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
    ::Ixq_SetCustomGrabEvent(grabber_id, camera_id, 1);
    float exposureHigh_ms = 0.f;
    float exposureLow_ms = 0.f;
    ::Ixq_GetXCFExposureTime(grabber_id, camera_id, id_icf, exposureHigh_ms, exposureLow_ms);
    ::Ixq_SetXCFExposureTime(grabber_id, camera_id, id_icf, 0.016f, 0.016f);
    ::Ixq_SetXCFParam(grabber_id, camera_id, id_icf, XCF_CTL_TRIGGER_MODE, _enumTriggermode::UplinkTrigger);
    ::Ixq_SetXCFParam(grabber_id, camera_id, id_icf, XCF_CTL_TRIGGER_TYPE, _enumTriggerType::FG_Trigger);

    // Image grab 진행
    Ipvm::Image8u grabImage(DEFAULT_IMAGE_SIZE_X, nImageSizeY);

    bool success = true;

    *ppbyImg = grabImage.GetMem();
    o_image = grabImage;

    if (STATUS_GRAB_SUCCESS
        != ::Ixq_MultiGrabProcess(grabber_id, camera_id, ppbyImg, 1, 1, MtgWaitOp::MTG_END_WAIT, 1000, NULL, NULL))
    {
        ::AfxMessageBox(_T("Cannot grab image"), MB_ICONERROR | MB_OK);
        success = false;

        return false;
    }

    // 카메라 Param 수정
    int nCustEvtNum = 0;
    ::Ixq_GetCustomGrabEvent(grabber_id, camera_id, &nCustEvtNum, &custom_grab_event_counts[grabber_id][camera_id]);
    ::Ixq_SetCustomGrabEvent(grabber_id, camera_id, custom_grab_event_counts[grabber_id][camera_id]);
    ::Ixq_SetXCFParam(grabber_id, camera_id, id_icf, XCF_CTL_TRIGGER_MODE, _enumTriggermode::Freerun);
    ::Ixq_SetXCFParam(grabber_id, camera_id, id_icf, XCF_CTL_TRIGGER_TYPE, _enumTriggerType::HardwareTrigger);
    ::Ixq_SetXCFExposureTime(grabber_id, camera_id, id_icf, exposureHigh_ms, exposureLow_ms);

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

CString Framegrabber3D_iGrabXQ::GetCxpLinkfPacketError(const unsigned int& grabber_id, const unsigned int& camera_id)
{
    CString strPacketState("");
    BYTE byEnableChannel_Count(0x00), byEnableChannel_PacketState(0x00);
    if (::Ixq_GetCxpLinkState(grabber_id, camera_id, byEnableChannel_Count, byEnableChannel_PacketState)
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

CString Framegrabber3D_iGrabXQ::GetCurrentEnable_Channel_Connection_State(const unsigned int& grabber_id)
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

    if (::Ixq_CableMonState(grabber_id, byCableNum, byChannelState) == FALSE)
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

long Framegrabber3D_iGrabXQ::GetReciveExternalTrigger(const unsigned int& grabber_id, const unsigned int& camera_id)
{
    unsigned int nReciveExternalTrigger(0);
    if (::Ixq_GetNumExternalTrg(grabber_id, camera_id, nReciveExternalTrigger) == FALSE) //External Trigger
        return -99999;

    return nReciveExternalTrigger;
}

CString Framegrabber3D_iGrabXQ::GetPageTrigger_State(const unsigned int& grabber_id, const unsigned int& camera_id)
{
    CString strGetPageTriggerState("");

    BOOL bGetPageTrigger(FALSE);
    ::Ixq_GetCheckPageTrg(grabber_id, camera_id, bGetPageTrigger); //Page Trigger

    strGetPageTriggerState.Format(_T(",Get PageTrigger: %s"), bGetPageTrigger == TRUE ? _T("Success") : _T("Fail"));

    return strGetPageTriggerState;
}

long Framegrabber3D_iGrabXQ::GetReciveFrameCount2Camera(const unsigned int& grabber_id, const unsigned int& camera_id)
{
    unsigned int nReciveFrameCount2Camera(0);
    if (::Ixq_GetNumTransportFrm(grabber_id, camera_id, nReciveFrameCount2Camera)
        == FALSE) //Cam -> iGrabXE[B] Frame Count
        return -99999;

    return nReciveFrameCount2Camera;
}

CString Framegrabber3D_iGrabXQ::GetExceptionErrorMSG(const BYTE i_byMSGValue)
{
    CString strExceptionErrorMSG("");

    strExceptionErrorMSG.Format(_T("%x"), i_byMSGValue);

    return strExceptionErrorMSG;
}

iGrab3D_Error_LOG Framegrabber3D_iGrabXQ::GetiGrab3D_Error_LOG()
{
    iGrab3D_Error_LOG iGrab3D_Error_LOG_Copy = m_siGrab3D_Error_LOG;

    //ResetiGrab3D_Error_LOG();

    return iGrab3D_Error_LOG_Copy;
}

void Framegrabber3D_iGrabXQ::ResetiGrab3D_Error_LOG()
{
    m_siGrab3D_Error_LOG.Init();
}

bool Framegrabber3D_iGrabXQ::SetTriggerMode(int nMode)
{
    UNREFERENCED_PARAMETER(nMode);

    return true;
}

void Framegrabber3D_iGrabXQ::set_grabber_temperature(const int& grabber_id)
{
    float val(-1.f);

    if (::Ixq_GetBoardTemp(grabber_id, val) == TRUE)
        apply_grabber_temperature(grabber_id, val);
}

void Framegrabber3D_iGrabXQ::get_firmware_version(
    const int& grabber_id, unsigned int& cur_version, unsigned int& need_version)
{
    cur_version = get_use_firmware_version(grabber_id);
    need_version = IF_FW_VERSION_MIN;
}

void Framegrabber3D_iGrabXQ::get_library_version(unsigned int& cur_version, unsigned int& need_version)
{
    cur_version = get_use_library_version();
    need_version = IF_LIB_VERSION;
}

void Framegrabber3D_iGrabXQ::SaveGrabLogToCsv(const CString& filePath, const iGrab3D_Error_LOG& log, int grabber_id,
    int camera_id,
    DWORD waitRetCode, long profileNumber, long scanImageCount)
{
    CStdioFile file;
    CFileException ex;
    const UINT mode = CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite;

    if (file.Open(filePath, mode, &ex))
    {
        file.SeekToEnd();

        if (file.GetLength() == 0)
        {
            file.WriteString(_T("Timestamp,GrabberId,CameraId,WaitRet,ProfileNum,ScanCount,")
                             _T("PacketErr,CableState,ExternalTrig,PageTrig,RecvFrameCount,CustomEventCount,")
                             _T("HardIntrStart,SoftIntrStart,HardIntrEnd,SoftIntrEnd,RunState,MultGrabWaitRet\n"));
        }

        CString line;
        line.Format(_T("%s,%d,%d,%lu,%ld,%ld,%s,%u,%ld,%s,%ld,%ld,%ld,%ld,%ld,%ld,%lu\n"),
            CTime::GetCurrentTime().Format(_T("%Y-%m-%d %H:%M:%S")), grabber_id, camera_id, waitRetCode, profileNumber,
            scanImageCount,
            log.m_strPacketError, // 예: "0b00000000"
            log.m_byCxpLinkCableState, // 케이블 링크 상태 비트필드
            log.m_nExternalTrigger, // 외부 트리거 수
            log.m_strPageTrigger, // 페이지 트리거 상태 문자열
            log.m_nReciveFrametoCamera, // 카메라->Grabber 프레임 수
            log.m_nCustomGrabEventCount, // 커스텀 이벤트 카운트
            log.m_hard_intr_count_s, // 하드 인터럽트 시작 카운트
            log.m_soft_intr_count_s, // 소프트 인터럽트 시작 카운트
            log.m_hard_intr_count_e, // 하드 인터럽트 종료 카운트
            log.m_soft_intr_count_e, // 소프트 인터럽트 종료 카운트
            log.m_dwMultGrabWaitReturnVal // MultGrabWait 반환 값
        );

        file.WriteString(line);
        file.Close();
    }
}