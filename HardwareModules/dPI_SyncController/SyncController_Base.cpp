//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SyncController_Base.h"

//CPP_2_________________________________ This project's headers
#include "SyncCalculator.h"

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
SyncController_Base::SyncController_Base(void)
{
    static const int nVisionType = SystemConfig::GetInstance().GetVisionType();
    if (nVisionType == VISIONTYPE_2D_INSP || nVisionType == VISIONTYPE_TR || nVisionType == VISIONTYPE_SIDE_INSP
        || nVisionType == VISIONTYPE_SWIR_INSP) //kircheis_SWIR
        CSyncCalculator::GetInstance();
}

SyncController_Base::~SyncController_Base(void)
{
}

BOOL SyncController_Base::IsAvailable()
{
    return TRUE;
}

void SyncController_Base::PopupDioCheckDialog()
{
    return;
}

long SyncController_Base::GetVisionType()
{
    return 0;
}

void SyncController_Base::TurnOnLight(long frameIndex, const enSideVisionModule& i_eVisionModuleidx)
{
    UNREFERENCED_PARAMETER(frameIndex);
    UNREFERENCED_PARAMETER(i_eVisionModuleidx);

    return;
}

void SyncController_Base::TurnOnLight(long frameIndex, const enTRVisionmodule& TR_visionmodule)
{
    UNREFERENCED_PARAMETER(frameIndex);
    UNREFERENCED_PARAMETER(TR_visionmodule);

    return;
}

void SyncController_Base::TurnOffLight(const enSideVisionModule& i_eVisionModuleidx)
{
    UNREFERENCED_PARAMETER(i_eVisionModuleidx);

    return;
}

void SyncController_Base::TurnOffLight(const enTRVisionmodule& TR_visionmodule)
{
    UNREFERENCED_PARAMETER(TR_visionmodule);

    return;
}

void SyncController_Base::StartSyncBoard(BOOL bRetry, const enSideVisionModule& i_eVisionModuleidx)
{
    UNREFERENCED_PARAMETER(bRetry);
    UNREFERENCED_PARAMETER(i_eVisionModuleidx);

    return;
}

void SyncController_Base::StartSyncBoard(BOOL bRetry, const bool& single_mode, const enTRVisionmodule& TR_visionmodule)
{
    UNREFERENCED_PARAMETER(bRetry);
    UNREFERENCED_PARAMETER(single_mode);
    UNREFERENCED_PARAMETER(TR_visionmodule);

    return;
}

void SyncController_Base::SetReadySignal(const BOOL& bActive)
{
    UNREFERENCED_PARAMETER(bActive);

    return;
}

BOOL SyncController_Base::GetReadySignal()
{
    return TRUE;
}

void SyncController_Base::SetGrabAcquisition(const BOOL& bActive)
{
    UNREFERENCED_PARAMETER(bActive);

    return;
}

BOOL SyncController_Base::GetGrabAcquisition()
{
    return TRUE;
}

void SyncController_Base::SetGrabExposure(const BOOL& bActive)
{
    UNREFERENCED_PARAMETER(bActive);

    return;
}

BOOL SyncController_Base::GetGrabExposure()
{
    return TRUE;
}

BOOL SyncController_Base::GetStartSignal()
{
    return TRUE;
}

UINT SyncController_Base::GetFovIndex()
{
    return 0;
}

UINT SyncController_Base::GetScanIDforSide()
{
    return 0;
}

UINT SyncController_Base::GetStitchingIndexForSide()
{
    return 0;
}

BOOL SyncController_Base::GetGrabHighPosGrabBit()
{
    return TRUE;
}

void SyncController_Base::SetTriggerEdgeMode(
    BOOL bMainTriggerMode, BYTE GlobalShutterMode, BYTE RisingEdgeMode, const enSideVisionModule& i_eVisionModuleidx)
{
    UNREFERENCED_PARAMETER(bMainTriggerMode);
    UNREFERENCED_PARAMETER(GlobalShutterMode);
    UNREFERENCED_PARAMETER(RisingEdgeMode);
    UNREFERENCED_PARAMETER(i_eVisionModuleidx);

    return;
}

void SyncController_Base::SetTriggerEdgeMode(
    BOOL bMainTriggerMode, BYTE GlobalShutterMode, BYTE RisingEdgeMode, const enTRVisionmodule& TR_visionmodule)
{
    UNREFERENCED_PARAMETER(bMainTriggerMode);
    UNREFERENCED_PARAMETER(GlobalShutterMode);
    UNREFERENCED_PARAMETER(RisingEdgeMode);
    UNREFERENCED_PARAMETER(TR_visionmodule);
}

BOOL SyncController_Base::SetOutputBit(const BYTE& byOrder, const BOOL& bOn)
{
    UNREFERENCED_PARAMETER(byOrder);
    UNREFERENCED_PARAMETER(bOn);

    return TRUE;
}

BOOL SyncController_Base::GetOutputBit(const BYTE& byOrder, BOOL& bOn)
{
    UNREFERENCED_PARAMETER(byOrder);
    UNREFERENCED_PARAMETER(bOn);

    return TRUE;
}

BOOL SyncController_Base::GetInputBit(const BYTE& byOrder, BOOL& bOn)
{
    UNREFERENCED_PARAMETER(byOrder);
    UNREFERENCED_PARAMETER(bOn);

    return TRUE;
}

BOOL SyncController_Base::GetInputBit(const BYTE& byByteIndex, const BYTE& byBitMask, BOOL& bValue)
{
    UNREFERENCED_PARAMETER(byByteIndex);
    UNREFERENCED_PARAMETER(byBitMask);
    UNREFERENCED_PARAMETER(bValue);

    return TRUE;
}

BOOL SyncController_Base::GetOutputBit(const BYTE& byByteIndex, const BYTE& byBitMask, BOOL& bValue)
{
    UNREFERENCED_PARAMETER(byByteIndex);
    UNREFERENCED_PARAMETER(byBitMask);
    UNREFERENCED_PARAMETER(bValue);

    return TRUE;
}

BOOL SyncController_Base::SetOutputBit(const BYTE& byByteIndex, const BYTE& byBitMask, const BOOL& bValue)
{
    UNREFERENCED_PARAMETER(byByteIndex);
    UNREFERENCED_PARAMETER(byBitMask);
    UNREFERENCED_PARAMETER(bValue);

    return TRUE;
}

BOOL SyncController_Base::Download(const BYTE& byAddress, const BYTE& byData)
{
    UNREFERENCED_PARAMETER(byAddress);
    UNREFERENCED_PARAMETER(byData);

    return TRUE;
}

BOOL SyncController_Base::Download(
    const BYTE& byAddress, const BYTE& byData, const enSideVisionModule& i_eVisionModuleidx)
{
    UNREFERENCED_PARAMETER(byAddress);
    UNREFERENCED_PARAMETER(byData);
    UNREFERENCED_PARAMETER(i_eVisionModuleidx);

    return TRUE;
}

BOOL SyncController_Base::Download(const BYTE& byAddress, const BYTE& byData, const enTRVisionmodule& TR_visionmodule)
{
    UNREFERENCED_PARAMETER(byAddress);
    UNREFERENCED_PARAMETER(byData);
    UNREFERENCED_PARAMETER(TR_visionmodule);

    return TRUE;
}

void SyncController_Base::ReadCPLDVersion(BYTE& byHighValue, BYTE& byLowValue)
{
    UNREFERENCED_PARAMETER(byHighValue);
    UNREFERENCED_PARAMETER(byLowValue);

    return;
}

void SyncController_Base::ReadAVRVersion(BYTE& byHighValue, BYTE& byLowValue)
{
    UNREFERENCED_PARAMETER(byHighValue);
    UNREFERENCED_PARAMETER(byLowValue);

    return;
}

void SyncController_Base::SetIllumiParameter(const long& nFrame, const std::array<float, 16>& illums_ms,
    const bool& adjustIntensity, const bool& isVerification, const BYTE& verificationType,
    const enSideVisionModule& i_eVisionModuleidx)
{
    UNREFERENCED_PARAMETER(nFrame);
    UNREFERENCED_PARAMETER(illums_ms);
    UNREFERENCED_PARAMETER(adjustIntensity);
    UNREFERENCED_PARAMETER(isVerification);
    UNREFERENCED_PARAMETER(verificationType);
    UNREFERENCED_PARAMETER(i_eVisionModuleidx);

    return;
}

void SyncController_Base::SetIllumiParameter(const long& nFrame, const std::array<float, 16>& illums_ms,
    const bool& adjustIntensity, const bool& isVerification, const BYTE& verificationType,
    const enTRVisionmodule& TR_visionmodule)
{
    UNREFERENCED_PARAMETER(nFrame);
    UNREFERENCED_PARAMETER(illums_ms);
    UNREFERENCED_PARAMETER(adjustIntensity);
    UNREFERENCED_PARAMETER(isVerification);
    UNREFERENCED_PARAMETER(verificationType);
    UNREFERENCED_PARAMETER(TR_visionmodule);

    return;
}

void SyncController_Base::SetIllumiParameter(const long& nFrame, const long& nChannel, const float& illum_ms,
    const bool& adjustIntensity, const bool& isVerification, const BYTE& verificationType,
    const enSideVisionModule& i_eVisionModuleidx)
{
    UNREFERENCED_PARAMETER(nFrame);
    UNREFERENCED_PARAMETER(nChannel);
    UNREFERENCED_PARAMETER(illum_ms);
    UNREFERENCED_PARAMETER(adjustIntensity);
    UNREFERENCED_PARAMETER(isVerification);
    UNREFERENCED_PARAMETER(verificationType);
    UNREFERENCED_PARAMETER(i_eVisionModuleidx);

    return;
}

void SyncController_Base::SetIllumiParameter(const long& nFrame, const long& nChannel, const float& illum_ms,
    const bool& adjustIntensity, const bool& isVerification, const BYTE& verificationType,
    const enTRVisionmodule& TR_visionmodule)
{
    UNREFERENCED_PARAMETER(nFrame);
    UNREFERENCED_PARAMETER(nChannel);
    UNREFERENCED_PARAMETER(illum_ms);
    UNREFERENCED_PARAMETER(adjustIntensity);
    UNREFERENCED_PARAMETER(isVerification);
    UNREFERENCED_PARAMETER(verificationType);
    UNREFERENCED_PARAMETER(TR_visionmodule);

    return;
}


void SyncController_Base::StartSyncBoardHighPosGrab(BOOL bRetry, const enSideVisionModule& i_eVisionModuleidx)
{
    UNREFERENCED_PARAMETER(bRetry);
    UNREFERENCED_PARAMETER(i_eVisionModuleidx);

    return;
}

void SyncController_Base::StartSyncBoardHighPosGrab(BOOL bRetry, const enTRVisionmodule& TR_visionmodule)
{
    UNREFERENCED_PARAMETER(bRetry);
    UNREFERENCED_PARAMETER(TR_visionmodule);

    return;
}

void SyncController_Base::Set2DCurrentFrame(long nFrame, const enSideVisionModule& i_eVisionModuleidx)
{
    UNREFERENCED_PARAMETER(nFrame);
    UNREFERENCED_PARAMETER(i_eVisionModuleidx);

    return;
}

void SyncController_Base::Set2DCurrentFrame(long nFrame, const enTRVisionmodule& TR_visionmodule)
{
    UNREFERENCED_PARAMETER(nFrame);
    UNREFERENCED_PARAMETER(TR_visionmodule);

    return;
}

void SyncController_Base::SetFrameCount(long n2D_Normal, long n2D_HighPos, const enSideVisionModule& i_eVisionModuleidx)
{
    UNREFERENCED_PARAMETER(n2D_Normal);
    UNREFERENCED_PARAMETER(n2D_HighPos);
    UNREFERENCED_PARAMETER(i_eVisionModuleidx);

    return;
}

void SyncController_Base::SetFrameCount(long n2D_Normal, long n2D_HighPos, const enTRVisionmodule& TR_visionmodule)
{
    UNREFERENCED_PARAMETER(n2D_Normal);
    UNREFERENCED_PARAMETER(n2D_HighPos);
    UNREFERENCED_PARAMETER(TR_visionmodule);

    return;
}

void SyncController_Base::SetFramePeriod(float fTime, const enSideVisionModule& i_eVisionModuleidx)
{
    UNREFERENCED_PARAMETER(fTime);
    UNREFERENCED_PARAMETER(i_eVisionModuleidx);

    return;
}

void SyncController_Base::SetFramePeriod(float fTime, const const enTRVisionmodule& TR_visionmodule)
{
    UNREFERENCED_PARAMETER(fTime);
    UNREFERENCED_PARAMETER(TR_visionmodule);

    return;
}

void SyncController_Base::SetCameraTransferTime(float fTime, const enSideVisionModule& i_eVisionModuleidx)
{
    UNREFERENCED_PARAMETER(fTime);
    UNREFERENCED_PARAMETER(i_eVisionModuleidx);

    return;
}

void SyncController_Base::SetCameraTransferTime(float fTime, const enTRVisionmodule& TR_visionmodule)
{
    UNREFERENCED_PARAMETER(fTime);
    UNREFERENCED_PARAMETER(TR_visionmodule);

    return;
}

void SyncController_Base::SetEncoderScaling(BYTE scaling)
{
    UNREFERENCED_PARAMETER(scaling);

    return;
}

void SyncController_Base::TurnOnLight(long frameIndex, const int& i_nDualMode)
{
    UNREFERENCED_PARAMETER(frameIndex);
    UNREFERENCED_PARAMETER(i_nDualMode);

    return;
}

void SyncController_Base::TurnOffLight(const int& i_nDualMode)
{
    UNREFERENCED_PARAMETER(i_nDualMode);

    return;
}

void SyncController_Base::SetInspectionMode(const int grabMode3D)
{
    UNREFERENCED_PARAMETER(grabMode3D);

    return;
}

void SyncController_Base::SetilluminationValue(const int illumValue)
{
    UNREFERENCED_PARAMETER(illumValue);
    return;
}

void SyncController_Base::SetilluminationMode(const int illumMode)
{
    UNREFERENCED_PARAMETER(illumMode);
    return;
}

void SyncController_Base::SetilluminationTime(const double illumSpecular, const double illumDefused)
{
    UNREFERENCED_PARAMETER(illumSpecular);
    UNREFERENCED_PARAMETER(illumDefused);

    return;
}

BOOL SyncController_Base::Download_DVH(const BYTE& byAddress, const BYTE& byData)
{
    UNREFERENCED_PARAMETER(byAddress);
    UNREFERENCED_PARAMETER(byData);

    return TRUE;
}

void SyncController_Base::StartSyncBoard(BOOL bRetry, const int i_index)
{
    UNREFERENCED_PARAMETER(bRetry);
    UNREFERENCED_PARAMETER(i_index);

    return;
}


void SyncController_Base::SetEncoderMultiply(BOOL useEncoder4Multiply)
{
    UNREFERENCED_PARAMETER(useEncoder4Multiply);

    return;
}

void SyncController_Base::SetGrabPosition(unsigned short position)
{
    UNREFERENCED_PARAMETER(position);

    return;
}

void SyncController_Base::SetGrabDirection(BYTE bGrabDir, BOOL bisDVH)
{
    UNREFERENCED_PARAMETER(bGrabDir);

    return;
}

long SyncController_Base::ReadCount(int& nNewCount)
{
    UNREFERENCED_PARAMETER(nNewCount);

    return 0;
}

void SyncController_Base::SetSignalNGRV_VisionRecvPkgID(const BOOL& bActive)
{
    UNREFERENCED_PARAMETER(bActive);

    return;
}

BOOL SyncController_Base::GetSignalNGRV_VisionRecvPkgID()
{
    return TRUE;
}

void SyncController_Base::SetSignalNGRV_VisionRecvGrabID(const BOOL& bActive)
{
    UNREFERENCED_PARAMETER(bActive);

    return;
}

BOOL SyncController_Base::GetSignalNGRV_VisionRecvGrabID()
{
    return TRUE;
}

void SyncController_Base::SetSignalNGRV_VisionMoveIRPos(const BOOL& bActive)
{
    UNREFERENCED_PARAMETER(bActive);

    return;
}

BOOL SyncController_Base::GetSignalNGRV_VisionMoveIRPos()
{
    return TRUE;
}

void SyncController_Base::SetSignalNGRV_VisionSaveImageReady(const BOOL& bActive)
{
    UNREFERENCED_PARAMETER(bActive);

    return;
}

BOOL SyncController_Base::GetSignalNGRV_VisionSaveImageReady()
{
    return TRUE;
}

void SyncController_Base::SetLongExposureCh(long nIRChID, long nUVChID)
{
    UNREFERENCED_PARAMETER(nIRChID);
    UNREFERENCED_PARAMETER(nUVChID);

    return;
}

UINT SyncController_Base::GetIndexStitchIDNGRV()
{
    return 0;
}

BOOL SyncController_Base::GetSignalNGRV_StartIR()
{
    return TRUE;
}

BOOL SyncController_Base::GetSignalNGRV_GrabID()
{
    return TRUE;
}
