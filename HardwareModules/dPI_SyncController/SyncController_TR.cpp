//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SyncController_TR.h"

//CPP_2_________________________________ This project's headers
#include "Firmware/IPIS300_PCI_V51/DioDef.h"
#include "Firmware/TR/sync_ram.h"
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
SyncController_TR::SyncController_TR(void)
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

    init_use_illum_data(USED_ILLUM_SEL_BIT, 255, enTRVisionmodule::TR_VISIONMODULE_OTI);
    init_use_illum_data(USED_ILLUM_SEL_BIT, 255, enTRVisionmodule::TR_VISIONMODULE_INPOCKET);

    Download(LED_2D_ON_DURATION, 4, enTRVisionmodule::TR_VISIONMODULE_OTI);
    Download(LED_2D_ON_DURATION, 4, enTRVisionmodule::TR_VISIONMODULE_INPOCKET);

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

    CSyncCalculator::GetInstance();
}

SyncController_TR::~SyncController_TR(void)
{
    if (m_hDevice == INVALID_HANDLE_VALUE)
    {
        return;
    }

    SyncPci_UnloadDevice();
    SyncPci_UnloadDll();

    m_hDevice = INVALID_HANDLE_VALUE;
}

void SyncController_TR::init_use_illum_data(
    const BYTE& byAddress, const BYTE& data, const enTRVisionmodule& TR_visionmodule)
{
    if (IsAvailable() == FALSE)
        return;

    TR_OFFSET_VAL tr_offset;
    tr_offset.set_offset_val(TR_visionmodule);

    BYTE byHigh(0);
    BYTE byLow(0);
    if (TR_visionmodule == enTRVisionmodule::TR_VISIONMODULE_OTI)
        byLow = data;
    else
        byHigh = data;

    SyncPci_Write(m_hDevice, _CS0, tr_offset.offset_high_data, byHigh);
    SyncPci_Write(m_hDevice, _CS0, tr_offset.offset_low_data, byLow);
    SyncPci_Write(m_hDevice, _CS0, tr_offset.offset_addr, byAddress);

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

    SyncPci_Read(m_hDevice, _CS0, tr_offset.offset_high_data, &byHighRead);
    ASSERT(byHighRead == 0); // 제대로 동작하는 0 이어야 되도록 싱크에서 구현되어 있음.

    SyncPci_Read(m_hDevice, _CS0, tr_offset.offset_low_data, &byLowRead);
    ASSERT(byLowRead == data);

    SyncPci_Read(m_hDevice, _CS0, tr_offset.offset_addr, &byAddrRead);
    ASSERT(byAddrRead == byAddress);
}

BOOL SyncController_TR::IsAvailable()
{
    if (m_hDevice == INVALID_HANDLE_VALUE)
        return FALSE;

    return TRUE;
}

long SyncController_TR::GetVisionType()
{
    return VISIONTYPE_TR;
}

void SyncController_TR::PopupDioCheckDialog()
{
    if (IsAvailable() == FALSE)
        return;

    ::AfxBeginThread(RUNTIME_CLASS(SyncViewerThread));
}

void SyncController_TR::ReadCPLDVersion(BYTE& byHighValue, BYTE& byLowValue)
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

void SyncController_TR::ReadAVRVersion(BYTE& byHighValue, BYTE& byLowValue)
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

void SyncController_TR::TurnOnLight(long frameIndex, const enTRVisionmodule& TR_visionmodule)
{
    if (IsAvailable() == FALSE)
        return;

    Set2DCurrentFrame(frameIndex, TR_visionmodule);
}

void SyncController_TR::TurnOffLight(const enTRVisionmodule& TR_visionmodule)
{
    if (IsAvailable() == FALSE)
        return;

    Download(ILLUM_COMMAND, COMMAND_CLEAR, TR_visionmodule);
}

bool SyncController_TR::GetisVirtualMode() const
{
    return m_hDevice == INVALID_HANDLE_VALUE && VirtualVisionIO::GetInstance().IsEnabled() == true ? true : false;
}

void SyncController_TR::SetReadySignal(const BOOL& bActive)
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

BOOL SyncController_TR::GetReadySignal()
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BOOL bActive(FALSE);

    GetOutputBit(0, OUTPUT_VISION_READY, bActive);

    return bActive;
}

void SyncController_TR::SetGrabAcquisition(const BOOL& bActive)
{
    if (IsAvailable() == FALSE)
        return;

    SetOutputBit(0, OUTPUT_VISION_ACQUISITION, bActive);
}

BOOL SyncController_TR::GetGrabAcquisition()
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BOOL bActive(FALSE);

    GetOutputBit(0, OUTPUT_VISION_ACQUISITION, bActive);

    return bActive;
}

void SyncController_TR::SetGrabExposure(const BOOL& bActive)
{
    if (IsAvailable() == FALSE)
        return;

    SetOutputBit(0, OUTPUT_VISION_EXPOSURE, bActive);
}

BOOL SyncController_TR::GetGrabExposure()
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BOOL bActive(FALSE);

    GetOutputBit(0, OUTPUT_VISION_EXPOSURE, bActive);

    return bActive;
}

BOOL SyncController_TR::GetStartSignal()
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BOOL bActive(FALSE);

    GetInputBit(0, INPUT_VISION_START, bActive);

    return bActive;
}

UINT SyncController_TR::GetFovIndex()
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

UINT SyncController_TR::GetScanIDforSide()
{
    return GetFovIndex();
}

UINT SyncController_TR::GetStitchingIndexForSide()
{
    return 0;
}

BOOL SyncController_TR::GetGrabHighPosGrabBit()
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BOOL bActive(FALSE);

    GetInputBit(1, INPUT_GRAB_HIGH_POS_GRAB_BIT, bActive);

    return bActive;
}

void SyncController_TR::SetTriggerEdgeMode(
    BOOL bMainTriggerMode, BYTE GlobalShutterMode, BYTE RisingEdgeMode, const enTRVisionmodule& TR_visionmodule)
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

    Download(SYNC_OP_OPTION, BYTE(bGlobal | bRising | bTriigerMode), TR_visionmodule);
}

// Bit Write/Read
BOOL SyncController_TR::SetOutputBit(const BYTE& byOrder, const BOOL& bOn)
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

BOOL SyncController_TR::GetOutputBit(const BYTE& byOrder, BOOL& bOn)
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BYTE byData(0);

    SyncPci_Read(m_hDevice, 1, HANDLER0_W, &byData);

    //내부적으로 반전을 시키는거 같다. !추가
    bOn = !(byData & byOrder) ? TRUE : FALSE;
    return TRUE;
}

BOOL SyncController_TR::GetInputBit(const BYTE& byOrder, BOOL& bOn)
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BYTE byData(0);
    SyncPci_Read(m_hDevice, 1, HANDLER0_R, &byData);

    //내부적으로 반전을 시키는거 같다. !추가
    bOn = !(byData & byOrder) ? TRUE : FALSE;
    return TRUE;
}

BOOL SyncController_TR::GetInputBit(const BYTE& byByteIndex, const BYTE& byBitMask, BOOL& bValue)
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BYTE byInput(0);

    SyncPci_ReadHandler(m_hDevice, byByteIndex, &byInput);

    bValue = (byInput & byBitMask) ? FALSE : TRUE;

    return TRUE;
}

BOOL SyncController_TR::GetOutputBit(const BYTE& byByteIndex, const BYTE& byBitMask, BOOL& bValue)
{
    if (IsAvailable() == FALSE)
        return FALSE;

    BYTE byOutput(0);

    SyncPci_ReadHandlerOut(m_hDevice, byByteIndex, &byOutput);

    bValue = (byOutput & byBitMask) ? FALSE : TRUE;

    return TRUE;
}

BOOL SyncController_TR::SetOutputBit(const BYTE& byByteIndex, const BYTE& byBitMask, const BOOL& bValue)
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

BOOL SyncController_TR::Download(const BYTE& byAddress, const BYTE& byData, const enTRVisionmodule& TR_visionmodule)
{
    if (IsAvailable() == FALSE)
        return FALSE;

    TR_OFFSET_VAL tr_offset;
    tr_offset.set_offset_val(TR_visionmodule);

    SyncPci_Write(m_hDevice, _CS0, tr_offset.offset_high_data, byData);
    SyncPci_Write(m_hDevice, _CS0, tr_offset.offset_low_data, byData);
    SyncPci_Write(m_hDevice, _CS0, tr_offset.offset_addr, byAddress);

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

    SyncPci_Read(m_hDevice, _CS0, tr_offset.offset_high_data, &byHighRead);
    ASSERT(byHighRead == 0); // 제대로 동작하는 0 이어야 되도록 싱크에서 구현되어 있음.

    SyncPci_Read(m_hDevice, _CS0, tr_offset.offset_low_data, &byLowRead);
    ASSERT(byLowRead == byData);

    SyncPci_Read(m_hDevice, _CS0, tr_offset.offset_addr, &byAddrRead);
    ASSERT(byAddrRead == byAddress);

    return BOOL(byRecValue);
}

void SyncController_TR::StartSyncBoard(BOOL bRetry, const bool& single_mode, const enTRVisionmodule& TR_visionmodule)
{
    UNREFERENCED_PARAMETER(bRetry);

    const BYTE& command = single_mode == true ? MEASURE_INS : COINCIDENCE_MEASURE;

    Download(ILLUM_COMMAND, command, TR_visionmodule);
}

void SyncController_TR::StartSyncBoardHighPosGrab(BOOL bRetry, const enTRVisionmodule& TR_visionmodule)
{
    if (IsAvailable() == FALSE)
        return;

    UNREFERENCED_PARAMETER(bRetry);
    UNREFERENCED_PARAMETER(TR_visionmodule);

    //if (bRetry == TRUE)
    //{
    //    Download(ILLUM_COMMAND, MEASURE_START_HIGH_POS_GRAB_WITHOUT_OUTPUT_ACQUISITION_DONE, TR_visionmodule);
    //}
    //else
    //{
    //    Download(ILLUM_COMMAND, MEASURE_START_HIGH_POS_GRAB, TR_visionmodule);
    //}
}

void SyncController_TR::SetIllumiParameter(const long& nFrame, const std::array<float, 16>& illums_ms,
    const bool& adjustIntensity, const bool& isVerification, const BYTE& verificationType,
    const enTRVisionmodule& TR_visionmodule)
{
    for (long nChannel = 0; nChannel < SystemConfig::GetInstance().GetMaxIllumChannelCount(TR_visionmodule); nChannel++)
    {
        SetIllumiParameter(nFrame, nChannel, illums_ms[nChannel], adjustIntensity, isVerification, verificationType);
    }

    return;
}

void SyncController_TR::SetIllumiParameter(const long& nFrame, const long& nChannel, const float& illum_ms,
    const bool& adjustIntensity, const bool& isVerification, const BYTE& verificationType,
    const enTRVisionmodule& TR_visionmodule)
{
    auto& systemConfig = SystemConfig::
        GetInstance(); // SDY UseChannelCount에서 매번 새로 systemconfig을 새로 만들지 않고 가져다 쓰도록 수정

    //long UseChannelCount = SystemConfig::GetInstance().GetUseIllumChannelCount();
    long nChannelMaxCount = SystemConfig::GetInstance().GetMaxIllumChannelCount(TR_visionmodule);

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

    static const long nTotalillumChannelCount = nChannelMaxCount;
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
                nFrame, nChannel, illum_ms, adjustIntensity, isVerification, verificationType);
        }

        else if (nIllumCalType
            == IllumCalType_Gain) //그다음 Gain 방식인가? //kircheis_IllumCalType//kircheis_IllumExpTime142
        {
            duration_ms = CSyncCalculator::GetInstance().CalcExposureTimeByGain(
                nFrame, nChannel, illum_ms, adjustIntensity, isVerification, verificationType);
        }

        else if (nIllumCalType == IllumCalType_OnlyLine || nMinCurveEqNum <= nChannel
            || CSyncCalculator::GetInstance().IsCalibrated2DExisting() == true)
        {
            duration_ms = CSyncCalculator::GetInstance().CalcExposureTimeByOnlyLine(
                nFrame, nChannel, illum_ms, adjustIntensity, isVerification, verificationType);
        }

        //{{  //kircheis_WB
        else if ((bUseIllumCalType2 || nIllumCalType == IllumCalType_Curve_Line)
            && vecIllumLineEqRef.size() == nChannelMaxCount && vecIllumLineEqCur.size() == nChannelMaxCount)
        {
            duration_ms = CSyncCalculator::GetInstance().CalcExposureTimeByCurve_Line(
                nFrame, nChannel, illum_ms, adjustIntensity, isVerification, verificationType);
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
        Set2DLedIntensity(nFrame, nChannel, duration_ms);
}

void SyncController_TR::Set2DCurrentFrame(long nFrame, const enTRVisionmodule& TR_visionmodule)
{
    Download(ILLUM_COMMAND, (BYTE)(nFrame + LIVE_COMMAND_2D_0), TR_visionmodule);
}

void SyncController_TR::SetFrameCount(long n2D_Normal, long n2D_HighPos, const enTRVisionmodule& TR_visionmodule)
{
    if (IsAvailable() == FALSE)
        return;

    Download(ILL_2D_COUNT, BYTE(n2D_Normal), TR_visionmodule);
    UNREFERENCED_PARAMETER(n2D_HighPos);
    //Download(ILL_2D_HIGH_POS_GRAB_COUNT, BYTE(n2D_HighPos), TR_visionmodule);
}

void SyncController_TR::SetFramePeriod(float fTime, const enTRVisionmodule& TR_visionmodule)
{
    if (IsAvailable() == FALSE)
        return;

    int nTime = (int)fTime;
    Download(CAMERA_FRAMEPERIOD, (BYTE)nTime, TR_visionmodule);
}

void SyncController_TR::SetCameraTransferTime(float fTime, const enTRVisionmodule& TR_visionmodule) //kircheis_CamTrans
{
    if (IsAvailable() == FALSE)
        return;

    int nTime = (int)fTime;

    Download(GRAB_SELECT_MODE, (BYTE)nTime, TR_visionmodule);
}

void SyncController_TR::Set2DLedIntensity(const long& nFrame, const long& nChannel, const float& duration_ms,
    const bool& bIsLongExposure, const enTRVisionmodule& TR_visionmodule) //kircheis_LongExp
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
    //unsigned char byAddr = (BYTE)((nFrame << 4) + nChannel);
    unsigned char byAddr = (BYTE)((nFrame << 4) + get_TR_real_channel_addr(nChannel, TR_visionmodule));

    TR_OFFSET_VAL tr_offset;
    tr_offset.set_offset_val(TR_visionmodule);

    // 내장 싱크 용량 제한으로 10장 사용 가능
    if (byAddr >= 0xa0)
    {
        return;
    }

    SyncPci_Write(m_hDevice, _CS0, tr_offset.offset_high_data, byHigh);
    SyncPci_Write(m_hDevice, _CS0, tr_offset.offset_low_data, byLow);
    SyncPci_Write(m_hDevice, _CS0, tr_offset.offset_addr, byAddr);

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

    SyncPci_Read(m_hDevice, _CS0, tr_offset.offset_high_data, &byHighRead);
    SyncPci_Read(m_hDevice, _CS0, tr_offset.offset_low_data, &byLowRead);
    SyncPci_Read(m_hDevice, _CS0, tr_offset.offset_addr, &byAddrRead);

    ASSERT(byHighRead == 0); // 제대로 동작하는 0 이어야 되도록 싱크에서 구현되어 있음.
    ASSERT(byLowRead == byLow);
    ASSERT(byAddrRead == byAddr);
}

unsigned char SyncController_TR::get_TR_real_channel_addr(
    const long& idx_channel, const enTRVisionmodule& TR_visionmodule) const
{
    if (TR_visionmodule == enTRVisionmodule::TR_VISIONMODULE_OTI)
    {
        switch (idx_channel)
        {
            case 0:
                return TR_REAL_CHANNEL_ADDR::OTI_CH1;
            case 1:
                return TR_REAL_CHANNEL_ADDR::OTI_CH2;
            case 2:
                return TR_REAL_CHANNEL_ADDR::OTI_REVERSE;
        }
    }
    else
    {
        switch (idx_channel)
        {
            case 0:
                return TR_REAL_CHANNEL_ADDR::INPOCKET_CH1;
            case 1:
                return TR_REAL_CHANNEL_ADDR::INPOCKET_CH2;
            case 2:
                return TR_REAL_CHANNEL_ADDR::INPOCKET_RING;
        }
    }
}