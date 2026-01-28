//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SyncController_SIDE.h"

//CPP_2_________________________________ This project's headers
#include "Firmware/ipis300_pci_avr_v330_1/CommDef.h"
#include "Firmware/ipis300_pci_avr_v330_1/DioDef.h"
#include "SyncCalculator.h"
#include "SyncPci.h"
#include "SyncViewerThread.h"
#include "VirtualVisionIO.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
SyncController_SIDE::SyncController_SIDE(void)
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

    CSyncCalculator::GetInstance().CalibrateIlluminationSide();

    Download(LED_2D_ON_DURATION, 4, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    Download(LED_2D_ON_DURATION, 4, enSideVisionModule::SIDE_VISIONMODULE_REAR);

    m_maskFovIndex = 0;
    m_maskFovIndex |= INPUT_SCAN_AREA_ID_0;
    m_maskFovIndex |= INPUT_SCAN_AREA_ID_1;
    m_maskFovIndex |= INPUT_SCAN_AREA_ID_2;
    m_maskFovIndex |= INPUT_SCAN_AREA_ID_3;
    m_maskFovIndex |= INPUT_SCAN_AREA_ID_4;
    m_maskFovIndex |= INPUT_SCAN_AREA_ID_5;
    m_maskFovIndex |= INPUT_SCAN_AREA_ID_6;
    m_maskFovIndex |= (INPUT_SCAN_AREA_ID_7 << 8);
    m_maskFovIndex |= (INPUT_SIDE_STITCH_ID_0 << 8);
    m_maskFovIndex |= (INPUT_SIDE_STITCH_ID_1 << 8);

    m_maskStitchIndexForSide = 0;
    m_maskStitchIndexForSide |= (INPUT_SIDE_STITCH_ID_0 << 8);
    m_maskStitchIndexForSide |= (INPUT_SIDE_STITCH_ID_1 << 8);

    SetReadySignal(FALSE);
    SetGrabAcquisition(FALSE);
    SetGrabExposure(FALSE);

    CSyncCalculator::GetInstance();
}

SyncController_SIDE::~SyncController_SIDE(void)
{
    if (m_hDevice == INVALID_HANDLE_VALUE)
    {
        return;
    }

    SyncPci_UnloadDevice();
    SyncPci_UnloadDll();

    m_hDevice = INVALID_HANDLE_VALUE;
}

BOOL SyncController_SIDE::IsAvailable()
{
    if (m_hDevice == INVALID_HANDLE_VALUE)
        return FALSE;

    return TRUE;
}

long SyncController_SIDE::GetVisionType()
{
    return VISIONTYPE_SIDE_INSP;
}

void SyncController_SIDE::ReadCPLDVersion(BYTE& byHighValue, BYTE& byLowValue)
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

void SyncController_SIDE::ReadAVRVersion(BYTE& byHighValue, BYTE& byLowValue)
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

void SyncController_SIDE::PopupDioCheckDialog()
{
    if (IsAvailable() == FALSE)
        return;

    ::AfxBeginThread(RUNTIME_CLASS(SyncViewerThread));
}

void SyncController_SIDE::TurnOnLight(long frameIndex, const enSideVisionModule& i_eVisionModuleidx)
{
    if (IsAvailable() == FALSE)
        return;

    Set2DCurrentFrame(frameIndex, i_eVisionModuleidx);
}

void SyncController_SIDE::TurnOffLight(const enSideVisionModule& i_eVisionModuleidx)
{
    if (IsAvailable() == FALSE)
        return;

    Download(ILLUM_COMMAND, COMMAND_CLEAR, i_eVisionModuleidx);
}

bool SyncController_SIDE::GetisVirtualMode() const
{
    return m_hDevice == INVALID_HANDLE_VALUE && VirtualVisionIO::GetInstance().IsEnabled() == true ? true : false;
}

void SyncController_SIDE::SetReadySignal(const BOOL& bActive)
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

BOOL SyncController_SIDE::GetReadySignal()
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BOOL bActive(FALSE);

    GetOutputBit(0, OUTPUT_VISION_READY, bActive);

    return bActive;
}

void SyncController_SIDE::SetGrabAcquisition(const BOOL& bActive)
{
    if (IsAvailable() == FALSE)
        return;

    SetOutputBit(0, OUTPUT_VISION_ACQUISITION, bActive);
}

BOOL SyncController_SIDE::GetGrabAcquisition()
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BOOL bActive(FALSE);

    GetOutputBit(0, OUTPUT_VISION_ACQUISITION, bActive);

    return bActive;
}

void SyncController_SIDE::SetGrabExposure(const BOOL& bActive)
{
    if (IsAvailable() == FALSE)
        return;

    SetOutputBit(0, OUTPUT_VISION_EXPOSURE, bActive);
}

BOOL SyncController_SIDE::GetGrabExposure()
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BOOL bActive(FALSE);

    GetOutputBit(0, OUTPUT_VISION_EXPOSURE, bActive);

    return bActive;
}

BOOL SyncController_SIDE::GetStartSignal()
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BOOL bActive(FALSE);

    GetInputBit(0, INPUT_VISION_START, bActive);

    return bActive;
}

UINT SyncController_SIDE::GetFovIndex()
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

UINT SyncController_SIDE::GetScanIDforSide()
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

    UINT scanID = (((uiInputUpper << 8) | uiInputLower) & m_maskFovIndex) >> uiBitShift;
    scanID &= 0xff;

    return scanID;
}

UINT SyncController_SIDE::GetStitchingIndexForSide()
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

    const UINT uiBitShift(9);

    UINT stitchIndex = (((uiInputUpper << 8) | uiInputLower) & m_maskStitchIndexForSide) >> uiBitShift;

    return stitchIndex;
}

BOOL SyncController_SIDE::GetGrabHighPosGrabBit()
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BOOL bActive(FALSE);

    GetInputBit(1, INPUT_GRAB_HIGH_POS_GRAB_BIT, bActive);

    return bActive;
}

void SyncController_SIDE::SetTriggerEdgeMode(
    BOOL bMainTriggerMode, BYTE GlobalShutterMode, BYTE RisingEdgeMode, const enSideVisionModule& i_eVisionModuleidx)
{
    if (IsAvailable() == FALSE)
        return;

    BYTE bGlobal(0);
    BYTE bRising(0);
    BYTE bTriigerMode(0);

    // Camera Shutter Mode
    if (GlobalShutterMode == 0)
    {
        bGlobal = 0x02;
    }
    else
    {
        bGlobal = 0x00;
    }

    // Trigger Edge Mode
    if (RisingEdgeMode == 0)
    {
        bRising = 0x01;
    }
    else
    {
        bRising = 0x00;
    }

    // Trigger Mode
    if (bMainTriggerMode)
    {
        bTriigerMode = 0x80;
    }
    else
    {
        bTriigerMode = 0x00;
    }

    Download(MAIN_CAM_TRIGGER_EDGEMODE, BYTE(bGlobal | bRising | bTriigerMode), i_eVisionModuleidx);
}

// Bit Write/Read
BOOL SyncController_SIDE::SetOutputBit(const BYTE& byOrder, const BOOL& bOn)
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

BOOL SyncController_SIDE::GetOutputBit(const BYTE& byOrder, BOOL& bOn)
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BYTE byData(0);

    SyncPci_Read(m_hDevice, 1, HANDLER0_W, &byData);

    //내부적으로 반전을 시키는거 같다. !추가
    bOn = !(byData & byOrder) ? TRUE : FALSE;
    return TRUE;
}

BOOL SyncController_SIDE::GetInputBit(const BYTE& byOrder, BOOL& bOn)
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BYTE byData(0);
    SyncPci_Read(m_hDevice, 1, HANDLER0_R, &byData);

    //내부적으로 반전을 시키는거 같다. !추가
    bOn = !(byData & byOrder) ? TRUE : FALSE;
    return TRUE;
}

BOOL SyncController_SIDE::GetInputBit(const BYTE& byByteIndex, const BYTE& byBitMask, BOOL& bValue)
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BYTE byInput(0);

    SyncPci_ReadHandler(m_hDevice, byByteIndex, &byInput);

    bValue = (byInput & byBitMask) ? FALSE : TRUE;

    return TRUE;
}

BOOL SyncController_SIDE::GetOutputBit(const BYTE& byByteIndex, const BYTE& byBitMask, BOOL& bValue)
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BYTE byOutput(0);

    SyncPci_ReadHandlerOut(m_hDevice, byByteIndex, &byOutput);

    bValue = (byOutput & byBitMask) ? FALSE : TRUE;

    return TRUE;
}

BOOL SyncController_SIDE::SetOutputBit(const BYTE& byByteIndex, const BYTE& byBitMask, const BOOL& bValue)
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

BOOL SyncController_SIDE::Download(
    const BYTE& byAddress, const BYTE& byData, const enSideVisionModule& i_eVisionModuleidx)
{
    if (IsAvailable() == FALSE)
        return FALSE;

    Sync_WriteCommand(m_hDevice, _CS0, byAddress, byData, byData, i_eVisionModuleidx);

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

void SyncController_SIDE::StartSyncBoard(BOOL bRetry, const enSideVisionModule& i_eVisionModuleidx)
{
    if (bRetry)
    {
        Download(ILLUM_COMMAND, MEASURE_START_WITHOUT_OUTPUT_ACQUISITION_DONE, i_eVisionModuleidx);
    }
    else
    {
        Download(ILLUM_COMMAND, MEASURE_START, i_eVisionModuleidx);
    }
}

void SyncController_SIDE::StartSyncBoardHighPosGrab(BOOL bRetry, const enSideVisionModule& i_eVisionModuleidx)
{
    UNREFERENCED_PARAMETER(bRetry);
    UNREFERENCED_PARAMETER(i_eVisionModuleidx);

    if (IsAvailable() == FALSE)
        return;

    //if (bRetry == TRUE)
    //{
    //	Download(ILLUM_COMMAND, MEASURE_START_HIGH_POS_GRAB_WITHOUT_OUTPUT_ACQUISITION_DONE);
    //}
    //else
    //{
    //	Download(ILLUM_COMMAND, MEASURE_START_HIGH_POS_GRAB);
    //}
}
void SyncController_SIDE::SetIllumiParameter(const long& nFrame, const std::array<float, 16>& illums_ms,
    const bool& adjustIntensity, const bool& isVerification, const BYTE& verificationType,
    const enSideVisionModule& i_eVisionModuleidx)
{
    for (long nChannel = 0; nChannel < SystemConfig::GetInstance().GetMaxIllumChannelCount(); nChannel++)
    {
        SetIllumiParameter(nFrame, nChannel, illums_ms[nChannel], adjustIntensity, isVerification, verificationType,
            i_eVisionModuleidx);
    }

    return;
}

void SyncController_SIDE::SetIllumiParameter(const long& nFrame, const long& nChannel, const float& illum_ms,
    const bool& adjustIntensity, const bool& isVerification, const BYTE& verificationType,
    const enSideVisionModule& i_eVisionModuleidx)
{
    auto& systemConfig = SystemConfig::
        GetInstance(); // SDY UseChannelCount에서 매번 새로 systemconfig을 새로 만들지 않고 가져다 쓰도록 수정

    //long UseChannelCount = SystemConfig::GetInstance().GetUseIllumChannelCount();
    long nChannelMaxCount = SystemConfig::GetInstance().GetMaxIllumChannelCount();

    if (nChannel >= nChannelMaxCount)
        return;

    if (nFrame < 10 && nFrame >= 0 && nChannel < 16 && nChannel >= 0) //kircheis_IllumHybrid
        m_arfInputExpTime[nFrame][nChannel] = illum_ms;

    //{{ //kircheis_WB
    float duration_ms = illum_ms;
    auto& vecIllumLineEqRef = systemConfig.m_vecIllumFittingRef;
    auto& vecIllumLineEqCur = systemConfig.m_vecIllumFittingCur;
    auto& vecIllumCurveEqRef = systemConfig.m_vecIllumCurveFittingRef; //kircheis_IllumCal
    auto& vecIllumCurveEqCur = systemConfig.m_vecIllumCurveFittingCur; //kircheis_IllumCal
    //auto& vecIllumCurveEqCurReverse = systemConfig.m_vecIllumCurveFittingCurReverse; //kircheis_IllumCal
    //long nCurveEqRefNum = (long)vecIllumCurveEqRef.size(); //kircheis_IllumCal
    //long nCurveEqCurNum = (long)vecIllumCurveEqCurReverse.size(); //kircheis_IllumCal
    //BOOL bUseCurveEq = ((nCurveEqRefNum > 0) && (nCurveEqRefNum == nCurveEqCurNum)); //kircheis_IllumCal
    static BOOL bUseIllumCalType2 = systemConfig.m_bUseIllumCalType2;
    //}}

    //static const long nUsedRingIllumNum = SystemConfig::GetInstance().GetExistRingillumination() == true ? LED_ILLUM_CHANNEL_RING_MAX : 0;
    static const long nTotalillumChannelCount = LED_ILLUM_CHANNEL_SIDE_DEFAULT;
    if (nChannel >= nTotalillumChannelCount)
        duration_ms = 0.f;

    const long nIllumCalType = systemConfig.m_nIlluminationCalType;

    if (adjustIntensity && duration_ms > 0)
    {
        long nMinLineEqNum = (long)min(vecIllumLineEqRef.size(), vecIllumLineEqCur.size());
        long nMinCurveEqNum = (long)min(vecIllumCurveEqRef.size(), vecIllumCurveEqCur.size());

        if (nMinLineEqNum <= nChannel) //현 채널의 보정 Data가 없는 경우
        {
            duration_ms = illum_ms;
        }

        if (nIllumCalType == IllumCalType_Linear_Plus) //먼저 Linear+ 여부 확인 //kircheis_IllumCalType
        {
            duration_ms = CSyncCalculator::GetInstance().CalcExposureTimeByLinearPlus(
                nFrame, nChannel, illum_ms, adjustIntensity, isVerification, verificationType, i_eVisionModuleidx);
        }
        else if (nIllumCalType
            == IllumCalType_Gain) //그다음 Gain 방식인가? //kircheis_IllumCalType//kircheis_IllumExpTime142
        {
            duration_ms = CSyncCalculator::GetInstance().CalcExposureTimeByGain(
                nFrame, nChannel, illum_ms, adjustIntensity, isVerification, verificationType, i_eVisionModuleidx);
        }

        else if (nIllumCalType == IllumCalType_OnlyLine
            || nMinCurveEqNum <= nChannel) // || m_bIsCalibrated2DIllumExisting == true)
        {
            duration_ms = CSyncCalculator::GetInstance().CalcExposureTimeByOnlyLine(
                nFrame, nChannel, illum_ms, adjustIntensity, isVerification, verificationType, i_eVisionModuleidx);
        }

        //{{  //kircheis_WB
        else if ((bUseIllumCalType2 || nIllumCalType == IllumCalType_Curve_Line)
            && vecIllumLineEqRef.size() == nChannelMaxCount && vecIllumLineEqCur.size() == nChannelMaxCount)
        {
            duration_ms = CSyncCalculator::GetInstance().CalcExposureTimeByCurve_Line(
                nFrame, nChannel, illum_ms, adjustIntensity, isVerification, verificationType, i_eVisionModuleidx);
        }

        else
        {
            duration_ms = (float)(illum_ms * CSyncCalculator::GetInstance().m_illuminationGains[nChannel]);
        }
        //}}
    }

    duration_ms = (float)max(0.f, duration_ms);
    if (nFrame < 10 && nFrame >= 0 && nChannel < 16 && nChannel >= 0)
        m_arfOutputExpTime[nFrame][nChannel] = duration_ms; //kircheis_IllumCalType

    if (IsAvailable() == TRUE)
        Set2DLedIntensity(nFrame, nChannel, duration_ms, false, i_eVisionModuleidx);
}

void SyncController_SIDE::Set2DCurrentFrame(long nFrame, const enSideVisionModule& i_eVisionModuleidx)
{
    Download(ILLUM_COMMAND, (BYTE)(nFrame + COMMAND_2D_0), i_eVisionModuleidx);
}

void SyncController_SIDE::SetFrameCount(long n2D_Normal, long n2D_HighPos, const enSideVisionModule& i_eVisionModuleidx)
{
    if (IsAvailable() == FALSE)
        return;

    Download(ILL_2D_COUNT, BYTE(n2D_Normal), i_eVisionModuleidx);
    Download(ILL_2D_HIGH_POS_GRAB_COUNT, BYTE(n2D_HighPos), i_eVisionModuleidx);
}

void SyncController_SIDE::SetFramePeriod(float fTime, const enSideVisionModule& i_eVisionModuleidx)
{
    if (IsAvailable() == FALSE)
        return;

    int nTime = (int)fTime;
    Download(MAIN_CAM_FRAMEPERIOD, (BYTE)nTime, i_eVisionModuleidx);
}

void SyncController_SIDE::SetCameraTransferTime(
    float fTime, const enSideVisionModule& i_eVisionModuleidx) //kircheis_CamTrans
{
    if (IsAvailable() == FALSE)
        return;

    int nTime = (int)fTime;
    Download(FRAME_PERIOD_OFFSET, (BYTE)nTime, i_eVisionModuleidx);
}

void SyncController_SIDE::Set2DLedIntensity(const long& nFrame, const long& nChannel, const float& duration_ms,
    const bool& bIsLongExposure, const enSideVisionModule& i_eVisionModuleidx) //kircheis_LongExp
{
    //{{//kircheis_LongExp
    float fduration_ms(0.f);
    if (bIsLongExposure == false)
        fduration_ms = (float)min(duration_ms, 65.f);
    short shGain = bIsLongExposure ? 50 : 500;
    unsigned short usDuration_us = (unsigned short)(shGain * fduration_ms + 0.5f);
    //}}
    //unsigned short usDuration_us = (unsigned short)(500 * duration_ms + 0.5f);

    unsigned char byHigh = usDuration_us >> 8;
    unsigned char byLow = usDuration_us & 0xFF;
    unsigned char byAddr = (BYTE)((nFrame << 4) + nChannel);

    // 내장 싱크 용량 제한으로 10장 사용 가능
    if (byAddr >= 0xa0)
    {
        return;
    }

    Sync_WriteCommand(m_hDevice, _CS0, byAddr, byLow, byHigh, i_eVisionModuleidx);

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

    BYTE byHighRead = 0;
    BYTE byLowRead = 0;
    BYTE byAddrRead = 0;

    SyncPci_Read(m_hDevice, _CS0, 2, &byHighRead);
    SyncPci_Read(m_hDevice, _CS0, 1, &byLowRead);
    SyncPci_Read(m_hDevice, _CS0, 0, &byAddrRead);

    ASSERT(byHighRead == 0); // 제대로 동작하는 0 이어야 되도록 싱크에서 구현되어 있음.
    ASSERT(byLowRead == byLow);
    ASSERT(byAddrRead == byAddr);
}

BOOL SyncController_SIDE::Sync_WriteCommand(HANDLE hDevice, ULONG cs, BYTE by_Address, BYTE by_Data_Low,
    BYTE by_Data_High, const enSideVisionModule i_eVisionModuleidx)
{
    BYTE byVisionModule_info(0x00), byVisionModuleDataType_info(0x00);
    //Define 된 Side Vision idx를 따라가면 된다
    switch (i_eVisionModuleidx)
    {
        case enSideVisionModule::SIDE_VISIONMODULE_FRONT:
            byVisionModule_info = SERIAL_ADD_FRONT;
            byVisionModuleDataType_info = SERIAL_DATA_FRONT;
            break;
        case enSideVisionModule::SIDE_VISIONMODULE_REAR:
            byVisionModule_info = SERIAL_ADD_REAR;
            byVisionModuleDataType_info = SERIAL_DATA_REAR;
            break;
        default:
            break;
    }

    if (byVisionModule_info == 0x00 || byVisionModuleDataType_info == 0x00)
        return FALSE;

    BOOL bResult(TRUE);

    //{{Data 하나를 쓸때 이 루틴으로 //무조건 Side로 용으로만..[ 계속 16비트로] Command Data 값은 상위나 하위나 똑같이..
    //변경되는 SyncPci_Write에서 Offset값의 주소는 0은 Addr Command 1은 Data Command임
    bResult = SyncPci_Write(hDevice, cs, 1, by_Address); //Addr를 먼저 주소. 선택
    bResult = SyncPci_Write(hDevice, cs, 0, byVisionModule_info);

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

    bResult = SyncPci_Write(hDevice, cs, 2, by_Data_High); //Data 선택 상위 [High]
    bResult = SyncPci_Write(hDevice, cs, 1, by_Data_Low); //Data 선택 하위 [Low]
    bResult = SyncPci_Write(hDevice, cs, 0, byVisionModuleDataType_info); //Data 줄꺼야 선택

    byRecValue = 1;
    for (int i = 0; i < 10; i++)
    {
        SyncPci_ReadAvrStat(m_hDevice, &byRecValue);

        if (!byRecValue)
        {
            break;
        }

        Sleep(1);
    }

    return TRUE;
}