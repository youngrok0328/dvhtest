//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SyncController_3D.h"

//CPP_2_________________________________ This project's headers
#include "Firmware/IPIS300_PCI_V51/CommDef.h"
#include "Firmware/IPIS300_PCI_V51/DioDef.h"
#include "SyncPci.h"
#include "SyncViewerThread.h"
#include "VirtualVisionIO.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
SyncController_3D::SyncController_3D(void)
    : m_hDevice(INVALID_HANDLE_VALUE)
{
    if (SyncPci_LoadDll(0) == FALSE)
    {
        ::AfxMessageBox(_T("SyncPci.dll file Not Found."), MB_ICONERROR | MB_OK);
        ::exit(0);
    }

    m_hDevice = SyncPci_LoadDevice(0);

    if (m_hDevice == INVALID_HANDLE_VALUE)
    {
        ::AfxMessageBox(_T("Failed to load syncboard device."), MB_ICONERROR | MB_OK);
        ::exit(0);
    }

    Download(LED_2D_ON_DURATION, 4);

    m_maskFovIndex = 0;
    m_maskFovIndex |= INPUT_SCAN_AREA_ID_0;
    m_maskFovIndex |= INPUT_SCAN_AREA_ID_1;
    m_maskFovIndex |= INPUT_SCAN_AREA_ID_2;
    m_maskFovIndex |= INPUT_SCAN_AREA_ID_3;
    m_maskFovIndex |= INPUT_SCAN_AREA_ID_4;
    m_maskFovIndex |= INPUT_SCAN_AREA_ID_5;
    m_maskFovIndex |= INPUT_SCAN_AREA_ID_6;
    m_maskFovIndex |= (INPUT_SCAN_AREA_ID_7 << 8);
    m_maskFovIndex |= (INPUT_SCAN_AREA_ID_8 << 8);

    SetReadySignal(FALSE);
    SetGrabAcquisition(FALSE);
    SetGrabExposure(FALSE);

    SetEncoderScaling(1);
    SetEncoderMultiply(FALSE);
}

SyncController_3D::~SyncController_3D(void)
{
    if (m_hDevice == INVALID_HANDLE_VALUE)
    {
        return;
    }

    SyncPci_UnloadDevice();
    SyncPci_UnloadDll();

    m_hDevice = INVALID_HANDLE_VALUE;
}

BOOL SyncController_3D::IsAvailable()
{
    if (m_hDevice == INVALID_HANDLE_VALUE)
        return FALSE;

    return TRUE;
}

long SyncController_3D::GetVisionType()
{
    return VISIONTYPE_3D_INSP;
}

void SyncController_3D::PopupDioCheckDialog()
{
    if (IsAvailable() == FALSE)
        return;

    ::AfxBeginThread(RUNTIME_CLASS(SyncViewerThread));
}

void SyncController_3D::ReadCPLDVersion(BYTE& byHighValue, BYTE& byLowValue)
{
    if (IsAvailable() == FALSE)
        return;

    BYTE byReadLowVal(0);
    BYTE byReadHighVal(0);

    SyncPci_Read(m_hDevice, 0, CPLD_L, &byReadLowVal);
    SyncPci_Read(m_hDevice, 0, CPLD_H, &byReadHighVal);

    byHighValue = byReadHighVal;
    byLowValue = byReadLowVal;
}

void SyncController_3D::ReadAVRVersion(BYTE& byHighValue, BYTE& byLowValue)
{
    if (IsAvailable() == FALSE)
        return;

    int ReadVal(0);
    BYTE byReadLowVal(0);
    BYTE byReadHighVal(0);

    SyncPci_Read(m_hDevice, 0, AVR_L, &byReadLowVal);
    SyncPci_Read(m_hDevice, 0, AVR_H, &byReadHighVal);

    byHighValue = byReadHighVal;
    byLowValue = byReadLowVal;

    ReadVal = (int)byReadHighVal;
    ReadVal = (ReadVal << 8) | byReadLowVal;
}

void SyncController_3D::TurnOnLight(long frameIndex, const int& i_nDualMode)
{
    if (i_nDualMode == 0)
    {
        UNREFERENCED_PARAMETER(frameIndex);

        if (IsAvailable() == FALSE)
            return;

        //BYTE byByteIndex = BYTE(OUTPUT_SLITBEAM_ILLUM_CH1 / 8);
        //BYTE byBitMask = 0x01 << BYTE(OUTPUT_SLITBEAM_ILLUM_CH1 % 8);

        SetOutputBit(BYTE(OUTPUT_SLITBEAM_ILLUM_CH1), TRUE);

        //SetOutputBit(byByteIndex, byBitMask, TRUE);
    }
    else if (i_nDualMode == 1)
    {
        if (IsAvailable() == FALSE)
            return;

        SyncPci_WriteAvrParam(m_hDevice, DVH_ILLUM_COMMAND, DVH_COMMAND_LED_ON);
    }
    
}

void SyncController_3D::TurnOffLight(const int& i_nDualMode)
{
    if (i_nDualMode == 0)
    {
        if (IsAvailable() == FALSE)
            return;

        //BYTE byByteIndex = BYTE(OUTPUT_SLITBEAM_ILLUM_CH1 / 8);
        //BYTE byBitMask = 0x01 << BYTE(OUTPUT_SLITBEAM_ILLUM_CH1 % 8);

        SetOutputBit(BYTE(OUTPUT_SLITBEAM_ILLUM_CH1), FALSE);

        //SetOutputBit(byByteIndex, byBitMask, FALSE);
    }
    else
    {
        if (IsAvailable() == FALSE)
            return;

        SyncPci_WriteAvrParam(m_hDevice, DVH_ILLUM_COMMAND, DVH_COMMAND_LED_OFF);

    }
}

bool SyncController_3D::GetisVirtualMode() const
{
    return m_hDevice == INVALID_HANDLE_VALUE && VirtualVisionIO::GetInstance().IsEnabled() == true ? true : false;
}

void SyncController_3D::SetReadySignal(const BOOL& bActive)
{
    //if (GetisVirtualMode() == true)
    //{
    //	VirtualVisionIO::GetInstance().SetVisionReady(bActive ? true : false);
    //	return;
    //}

    if (IsAvailable() == FALSE)
        return;

    SetOutputBit(0, OUTPUT_VISION_READY, bActive);
}

BOOL SyncController_3D::GetReadySignal()
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BOOL bActive(FALSE);

    GetOutputBit(0, OUTPUT_VISION_READY, bActive);

    return bActive;
}

void SyncController_3D::SetGrabAcquisition(const BOOL& bActive)
{
    if (IsAvailable() == FALSE)
        return;

    SetOutputBit(0, OUTPUT_VISION_ACQUISITION, bActive);
}

BOOL SyncController_3D::GetGrabAcquisition()
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BOOL bActive(FALSE);

    GetOutputBit(0, OUTPUT_VISION_ACQUISITION, bActive);

    return bActive;
}

void SyncController_3D::SetGrabExposure(const BOOL& bActive)
{
    if (IsAvailable() == FALSE)
        return;

    SetOutputBit(0, OUTPUT_VISION_EXPOSURE, bActive);
}

BOOL SyncController_3D::GetGrabExposure()
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BOOL bActive(FALSE);

    GetOutputBit(0, OUTPUT_VISION_EXPOSURE, bActive);

    return bActive;
}

BOOL SyncController_3D::GetStartSignal()
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BOOL bActive(FALSE);

    GetInputBit(0, INPUT_VISION_START, bActive);

    return bActive;
}

UINT SyncController_3D::GetFovIndex()
{
    if (IsAvailable() == FALSE)
        return 0;

    BYTE byInputLower(0);

    SyncPci_ReadHandler(m_hDevice, 0, &byInputLower);

    byInputLower = ~byInputLower;

    BYTE byInputUpper = 0;

    SyncPci_ReadHandler(m_hDevice, 1, &byInputUpper);

    byInputUpper = ~byInputUpper;

    UINT uiInputLower = byInputLower;
    UINT uiInputUpper = byInputUpper;

    const UINT uiBitShift(1);

    UINT fovIndex = (((uiInputUpper << 8) | uiInputLower) & m_maskFovIndex) >> uiBitShift;

    return fovIndex;
}

UINT SyncController_3D::GetScanIDforSide()
{
    return GetFovIndex();
}

UINT SyncController_3D::GetStitchingIndexForSide()
{
    return 0;
}

// Bit Write/Read
BOOL SyncController_3D::SetOutputBit(const BYTE& byOrder, const BOOL& bOn)
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BYTE byData(0);
    SyncPci_Read(m_hDevice, 1, HANDLER0_W, &byData);

    //내부적으로 반전을 시키는거 같다. !추가
    if (!bOn)
        byData |= byOrder;
    else
        byData &= ~byOrder;

    return SyncPci_Write(m_hDevice, 1, HANDLER0_W, byData);
}

BOOL SyncController_3D::GetOutputBit(const BYTE& byOrder, BOOL& bOn)
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BYTE byData(0);

    SyncPci_Read(m_hDevice, 1, HANDLER0_W, &byData);

    //내부적으로 반전을 시키는거 같다. !추가
    bOn = !(byData & byOrder) ? TRUE : FALSE;
    return TRUE;
}

BOOL SyncController_3D::GetInputBit(const BYTE& byOrder, BOOL& bOn)
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BYTE byData(0);
    SyncPci_Read(m_hDevice, 1, HANDLER0_R, &byData);

    //내부적으로 반전을 시키는거 같다. !추가
    bOn = !(byData & byOrder) ? TRUE : FALSE;
    return TRUE;
}

BOOL SyncController_3D::GetInputBit(const BYTE& byByteIndex, const BYTE& byBitMask, BOOL& bValue)
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BYTE byInput(0);

    SyncPci_ReadHandler(m_hDevice, byByteIndex, &byInput);

    bValue = (byInput & byBitMask) ? FALSE : TRUE;

    return TRUE;
}

BOOL SyncController_3D::GetOutputBit(const BYTE& byByteIndex, const BYTE& byBitMask, BOOL& bValue)
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BYTE byOutput(0);

    SyncPci_ReadHandlerOut(m_hDevice, byByteIndex, &byOutput);

    bValue = (byOutput & byBitMask) ? FALSE : TRUE;

    return TRUE;
}

BOOL SyncController_3D::SetOutputBit(const BYTE& byByteIndex, const BYTE& byBitMask, const BOOL& bValue)
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BYTE byOutput(0);

    SyncPci_ReadHandlerOut(m_hDevice, byByteIndex, &byOutput);

    if (bValue)
    {
        byOutput = byOutput & ~byBitMask;

        SyncPci_WriteHandler(m_hDevice, byByteIndex, byOutput, 0);
    }
    else
    {
        byOutput = byOutput | byBitMask;

        SyncPci_WriteHandler(m_hDevice, byByteIndex, byOutput, 0);
    }

    return TRUE;
}

BOOL SyncController_3D::Download(const BYTE& byAddress, const BYTE& byData)
{
    if (IsAvailable() == FALSE)
        return FALSE;

    SyncPci_Write(m_hDevice, _CS0, 1, byData);
    SyncPci_Write(m_hDevice, _CS0, 0, byAddress);

    BYTE byRecValue(1);

    for (int i = 0; i < 10; i++)
    {
        SyncPci_ReadAvrStat(m_hDevice, &byRecValue);

        if (!byRecValue)
        {
            break;
        }

        Sleep(1);
    }

    BYTE byLowRead(0);
    BYTE byAddrRead(0);

    SyncPci_Read(m_hDevice, _CS0, 1, &byLowRead);
    ASSERT(byLowRead == byData);

    SyncPci_Read(m_hDevice, _CS0, 0, &byAddrRead);
    ASSERT(byAddrRead == byAddress);

    return BOOL(byRecValue);
}

BOOL SyncController_3D::Download_DVH(const BYTE& byAddress, const BYTE& byData)
{
    if (IsAvailable() == FALSE)
        return FALSE;

    SyncPci_Write(m_hDevice, _CS0, 2, byData);
    SyncPci_Write(m_hDevice, _CS0, 1, byData);
    SyncPci_Write(m_hDevice, _CS0, 0, byAddress);

    BYTE byRecValue = 1;

    for (int i = 0; i < 10; i++)
    {
        SyncPci_ReadAvrStat(m_hDevice, &byRecValue);

        if (!byRecValue)
        {
            break;
        }

        Sleep(1);
    }

    BYTE byHighRead(0);
    BYTE byLowRead(0);
    BYTE byAddrRead(0);

    SyncPci_Read(m_hDevice, _CS0, 2, &byHighRead);
    ASSERT(byHighRead == 0); // 제대로 동작하는 0 이어야 되도록 싱크에서 구현되어 있음.

    SyncPci_Read(m_hDevice, _CS0, 1, &byLowRead);
    ASSERT(byLowRead == byData);

    SyncPci_Read(m_hDevice, _CS0, 0, &byAddrRead);
    ASSERT(byAddrRead == byAddress);

    return BOOL(byRecValue);
}

void SyncController_3D::SetEncoderScaling(BYTE scaling)
{
    if (IsAvailable() == FALSE)
        return;

    //0x01 : Encoder signal 1분주
    //   ......
    //0xff : Encoder signal
    SyncPci_Write(m_hDevice, 1, SCALE, scaling);
}

void SyncController_3D::SetEncoderMultiply(BOOL useEncoder4Multiply)
{
    if (IsAvailable() == FALSE)
        return;

    BYTE DATA = _PAGE_TRIGGER;

    if (0 == FORWARD)
        DATA |= _SCANDIR_FORWARD;
    else if (0 == BACKWARD)
        DATA |= _SCANDIR_BACKWARD;

    if (useEncoder4Multiply)
    {
        DATA |= _ENCODER_4_MULTIPLY;
    }

    SyncPci_Write(m_hDevice, _CS1, ENABLE, DATA);
}

void SyncController_3D::SetGrabPosition(unsigned short position)
{
    if (IsAvailable() == FALSE)
        return;

    BYTE highWrite = (BYTE)(0xff & (position >> 8));
    BYTE lowWrite = (BYTE)(0xff & (position));

    SyncPci_Write(m_hDevice, _CS1, GRABPOS_H, highWrite); // high MainCam
    SyncPci_Write(m_hDevice, _CS1, GRABPOS_L, lowWrite); // low MainCam
}

void SyncController_3D::SetGrabDirection(BYTE bGrabDir, BOOL bisDVH)
{
    if (IsAvailable() == FALSE)
        return;

    BYTE bCurDATA = 0x00;
    BYTE bSetDATA = 0xff;

    SyncPci_Read(m_hDevice, _CS1, ENABLE, &bCurDATA);

    if (bisDVH)
    {
        bCurDATA = bGrabDir;
        SyncPci_Write(m_hDevice, _CS1, ENABLE, bCurDATA);
    }
    else
    {
        if (bGrabDir == FORWARD)
        {
            bSetDATA -= _GRAB_DIRECTION;
            bCurDATA &= bSetDATA;
        }
        else
            bCurDATA |= _GRAB_DIRECTION;

        SyncPci_Write(m_hDevice, _CS1, ENABLE, bCurDATA);
    }
    Sleep(200);
}

void SyncController_3D::SetInspectionMode(const int grabMode3D)
{
    // 3D Vision Inspection Mode를 설정한다.
    if (IsAvailable() == FALSE)
        return;

    // 난반사(0,1), 전반사(2), 복합모드(3)
    // 이외 다른값은 이상한 값이다,, return반환
    if (grabMode3D < 0 || grabMode3D > 3)
        return;

    // grabMode3D가 0이면 byData는 2, 1이면 1, 2이면 3이 되어야 한다.
    BYTE byData = 0x00;
    switch (grabMode3D)
    {
        case 0:
            byData = DVH_DUAL_SPECULAR;
            break;
        case 1:
            byData = DVH_DUAL_DEFUSED;
            break;
        case 2:
            byData = DVH_DUAL_INTEGRATED;
            break;
        default:
            return;
    }

    SyncPci_WriteAvrParam(m_hDevice, DVH_CAMERA_TYPE, byData);
    Sleep(200);
}

void SyncController_3D::SetilluminationValue(const int illumValue)
{
    // 3D 조명 세기를 설정한다.
    if (!IsAvailable())
        return;

    // 조명 값이 0보다 작거나 255보다 크면 이상한 값이다,, return반환
    if (illumValue < 0 || illumValue > 255)
        return;

    BYTE byData = 0x00;
    byData = static_cast<BYTE>(illumValue);

    for (int i = 0; i < 4; ++i)
        SyncPci_WriteAvrParam(m_hDevice, DVH_ILLUM_VALUE_CH01 + i, byData);

    Sleep(200);
}

void SyncController_3D::SetilluminationMode(const int illumMode)
{
    // 3D Grab Mode에 따라 어떤 조명이 켜지고 꺼지는지를 설정한다.
    if (IsAvailable() == FALSE)
        return;

    BYTE byData_L1 = 0xFF;
    BYTE byData_L2 = 0xFF;
    // 3D 조명은 CH9~16번채널이 없기 때문에 무조건 꺼준다.. 255가 조명을 꺼주는 값이다.
    SyncPci_WriteAvrParam(m_hDevice, CH_ON_OFF_H1, static_cast<BYTE>(byData_L1));
    SyncPci_WriteAvrParam(m_hDevice, CH_ON_OFF_H2, static_cast<BYTE>(byData_L1));

    switch (illumMode)
    {
        case 0: // 전반사
        {
            byData_L1 = static_cast<BYTE>(0xFA);
            break;
        }
        case 1: // 난반사
        {
            byData_L1 = static_cast<BYTE>(0xF5);
            break;
        }
        case 2: // 복합모드
        {
            byData_L1 = static_cast<BYTE>(0xFA);
            byData_L2 = static_cast<BYTE>(0xF5);
            break;
        }
        default:
            break;
    }

    SyncPci_WriteAvrParam(m_hDevice, CH_ON_OFF_L1, byData_L1);
    Sleep(200);
    SyncPci_WriteAvrParam(m_hDevice, CH_ON_OFF_L2, byData_L2);
    Sleep(200);
}

void SyncController_3D::SetilluminationTime(const double illumSpecular, const double illumDefused)
{
    // 3D Vision 노출시간을 설정한다.
    if (IsAvailable() == FALSE)
        return;

    uint16_t illumSpecularValue = static_cast<uint16_t>(illumSpecular + 0.5);
    // illumSpecular의 상위 8비트를 byHighWrite에, 하위 8비트를 byLowWrite에 저장한다.
    BYTE highWrite_Specular = static_cast<BYTE>((illumSpecularValue >> 8) & 0xFF);
    BYTE lowWrite_Specular = static_cast<BYTE>(illumSpecularValue) & 0xFF;

    SyncPci_WriteAvrParam(m_hDevice, DVH_STEP2_EXPOSE_H, highWrite_Specular);
    SyncPci_WriteAvrParam(m_hDevice, DVH_STEP2_EXPOSE_L, lowWrite_Specular);

    uint16_t illumDefusedValue = static_cast<uint16_t>(illumDefused + 0.5);
    // illumDefused의 상위 8비트를 byHighWrite에, 하위 8비트를 byLowWrite에 저장한다.
    BYTE highWrite_Defused = static_cast<BYTE>((illumDefusedValue >> 8) & 0xFF);
    BYTE lowWrite_Defused = static_cast<BYTE>(illumDefusedValue) & 0xFF;

    SyncPci_WriteAvrParam(m_hDevice, DVH_STEP1_EXPOSE_H, highWrite_Defused);
    SyncPci_WriteAvrParam(m_hDevice, DVH_STEP1_EXPOSE_L, lowWrite_Defused);
}

void SyncController_3D::StartSyncBoard(BOOL bRetry, const int i_index) // TODO: inline grab 할 때 사용 여부 확인 필요
{
    if (i_index == (int)enDualGrabImageType::DUAL_IMAGE_TOTAL_REFLECTION)
    {
        Download_DVH(ILLUM_COMMAND, DVH_COMMAND_LED_OFF);
    }
    else if (i_index == (int)enDualGrabImageType::DUAL_IMAGE_DIFFUSE_REFLECTION)
    {
        Download_DVH(ILLUM_COMMAND, DVH_COMMAND_LED_OFF);
    }
    else if (i_index == (int)enDualGrabImageType::DUAL_IMAGE_COMPLEX_REFLECTION)
    {
        Download_DVH(ILLUM_COMMAND, DVH_COMMAND_LED_OFF);
    }
}

long SyncController_3D::ReadCount(int& nNewCount)
{
    if (IsAvailable() == FALSE)
        return 0;

    int ReadVal(0);
    BYTE byReadLowVal(0);
    BYTE byReadHighVal(0);

    int ReadVal_New(0);
    BYTE byReadLowVal_New(0);
    BYTE byReadHighVal_New(0);

    SyncPci_Read(m_hDevice, 0, CNTL, &byReadLowVal);
    SyncPci_Read(m_hDevice, 0, CNTH, &byReadHighVal);

    SyncPci_Read(m_hDevice, 1, TRIGGER_COUNT_L, &byReadLowVal_New);
    SyncPci_Read(m_hDevice, 1, TRIGGER_COUNT_H, &byReadHighVal_New);

    ReadVal = (int)byReadHighVal;
    ReadVal = (ReadVal << 8) | byReadLowVal;

    ReadVal_New = (int)byReadHighVal_New;
    nNewCount = ReadVal_New = (ReadVal_New << 8) | byReadLowVal_New;

    return ReadVal;
}