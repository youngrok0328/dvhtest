//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SyncController.h"

//CPP_2_________________________________ This project's headers
#include "SyncController_2D.h"
#include "SyncController_3D.h"
#include "SyncController_Base.h"
#include "SyncController_NGRV.h"
#include "SyncController_SIDE.h"
#include "SyncController_TR.h"

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
SyncController::SyncController()
    : m_impl(NULL)
{
    if (SystemConfig::GetInstance().m_bHardwareExist)
    {
        switch (SystemConfig::GetInstance().GetVisionType())
        {
            case VISIONTYPE_2D_INSP:
            case VISIONTYPE_SWIR_INSP: //kircheis_SWIR// NeedRefac우선은 2D와 동일하게 처리
                m_impl = new SyncController_2D();
                break;
            case VISIONTYPE_3D_INSP:
                m_impl = new SyncController_3D();
                break;
            case VISIONTYPE_NGRV_INSP:
                m_impl = new SyncController_NGRV();
                break;
            case VISIONTYPE_SIDE_INSP:
                m_impl = new SyncController_SIDE();
                break;
            case VISIONTYPE_TR:
                m_impl = new SyncController_TR();
                break;
            default:
                ::AfxMessageBox(_T("Vision type is invalid"), MB_ICONERROR | MB_OK);
                ::exit(0);
        }
    }
    else //Virtual 할때 사용할 자리
    {
        switch (SystemConfig::GetInstance().GetVisionType())
        {
            case VISIONTYPE_2D_INSP:
            case VISIONTYPE_SWIR_INSP: //kircheis_SWIR// NeedRefac우선은 2D와 동일하게 처리
                m_impl = new SyncController_Base();
                break;
            case VISIONTYPE_3D_INSP:
                m_impl = new SyncController_Base();
                break;
            case VISIONTYPE_NGRV_INSP:
                m_impl = new SyncController_Base();
                break;
            case VISIONTYPE_SIDE_INSP:
                m_impl = new SyncController_Base();
                break;
            case VISIONTYPE_TR:
                m_impl = new SyncController_Base();
                break;
            default:
                ::AfxMessageBox(_T("Vision type is invalid"), MB_ICONERROR | MB_OK);
                ::exit(0);
        }
    }
}

SyncController::~SyncController(void)
{
    delete m_impl;
}

SyncController& SyncController::GetInstance()
{
    static SyncController singleton;

    return singleton;
}

BOOL SyncController::IsAvailable()
{
    return m_impl->IsAvailable();
}

void SyncController::SetIllumiParameter(long nFrame, long nChannel, float illum_um, const bool adjustIntensity,
    const bool isVerification, const BYTE verificationType, const enSideVisionModule& i_eVisionModuleidx)
{
    m_impl->SetIllumiParameter(
        nFrame, nChannel, illum_um, adjustIntensity, isVerification, verificationType, i_eVisionModuleidx);
}

void SyncController::SetIllumiParameter(long nFrame, long nChannel, float illum_um, const bool adjustIntensity,
    const bool isVerification, const BYTE verificationType, const enTRVisionmodule& TR_visionmodule)
{
    m_impl->SetIllumiParameter(
        nFrame, nChannel, illum_um, adjustIntensity, isVerification, verificationType, TR_visionmodule);
}

void SyncController::SetIllumiParameter(long nFrame, const std::array<float, 16>& illums_ms, const bool adjustIntensity,
    const bool isVerification, const BYTE verificationType, const enSideVisionModule& i_eVisionModuleidx)
{
    m_impl->SetIllumiParameter(
        nFrame, illums_ms, adjustIntensity, isVerification, verificationType, i_eVisionModuleidx);
}

void SyncController::SetIllumiParameter(long nFrame, const std::array<float, 16>& illums_ms, const bool adjustIntensity,
    const bool isVerification, const BYTE verificationType, const enTRVisionmodule& TR_visionmodule)
{
    m_impl->SetIllumiParameter(nFrame, illums_ms, adjustIntensity, isVerification, verificationType, TR_visionmodule);
}

void SyncController::StartSyncBoard(BOOL bRetry, const enSideVisionModule& i_eVisionModuleidx)
{
    m_impl->StartSyncBoard(bRetry, i_eVisionModuleidx);
}

void SyncController::StartSyncBoard(BOOL bRetry, const bool& single_mode, const enTRVisionmodule& TR_visionmodule)
{
    m_impl->StartSyncBoard(bRetry, single_mode, TR_visionmodule);
}

void SyncController::StartSyncBoardHighPosGrab(BOOL bRetry, const enSideVisionModule& i_eVisionModuleidx)
{
    m_impl->StartSyncBoardHighPosGrab(bRetry, i_eVisionModuleidx);
}

void SyncController::StartSyncBoardHighPosGrab(BOOL bRetry, const enTRVisionmodule& TR_visionmodule)
{
    m_impl->StartSyncBoardHighPosGrab(bRetry, TR_visionmodule);
}

void SyncController::SetGrabExposure(BOOL bOn)
{
    m_impl->SetGrabExposure(bOn);
}

BOOL SyncController::GetGrabExposure()
{
    return m_impl->GetGrabExposure();
}

void SyncController::TurnOnLight(long frameIndex, const enSideVisionModule& i_eVisionModuleidx)
{
    m_impl->TurnOnLight(frameIndex, i_eVisionModuleidx);
}

void SyncController::TurnOnLight(long frameIndex, const enTRVisionmodule& TR_modules)
{
    m_impl->TurnOnLight(frameIndex, TR_modules);
}

void SyncController::TurnOffLight(const enSideVisionModule& i_eVisionModuleidx)
{
    m_impl->TurnOffLight(i_eVisionModuleidx);
}

void SyncController::TurnOffLight(const enTRVisionmodule& TR_modules)
{
    m_impl->TurnOffLight(TR_modules);
}

void SyncController::TurnOnLight(long frameIndex, const int& i_nDualMode)
{
    m_impl->TurnOnLight(frameIndex, i_nDualMode);
}

void SyncController::TurnOffLight(const int& i_nDualMode)
{
    m_impl->TurnOffLight(i_nDualMode);
}

BOOL SyncController::Download(const BYTE& byAddress, const BYTE& byData)
{
    return m_impl->Download(byAddress, byData);
}

BOOL SyncController::Download(const BYTE& byAddress, const BYTE& byData, const enSideVisionModule& i_eVisionModuleidx)
{
    return m_impl->Download(byAddress, byData, i_eVisionModuleidx);
}

BOOL SyncController::Download(const BYTE& byAddress, const BYTE& byData, const enTRVisionmodule& TR_visionmodule)
{
    return m_impl->Download(byAddress, byData, TR_visionmodule);
}

void SyncController::ReadCPLDVersion(BYTE& byHighValue, BYTE& byLowValue)
{
    m_impl->ReadCPLDVersion(byHighValue, byLowValue);
}

void SyncController::ReadAVRVersion(BYTE& byHighValue, BYTE& byLowValue)
{
    m_impl->ReadAVRVersion(byHighValue, byLowValue);
}

void SyncController::PopupDioCheckDialog()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    m_impl->PopupDioCheckDialog();
}

SyncController_Base* SyncController::GetSync()
{
    return m_impl;
}

void SyncController::SetFrameCount(long n2D_Normal, long n2D_HighPos, const enSideVisionModule& i_eVisionModuleidx)
{
    m_impl->SetFrameCount(n2D_Normal, n2D_HighPos, i_eVisionModuleidx);
}

void SyncController::SetFrameCount(long n2D_Normal, long n2D_HighPos, const enTRVisionmodule& TR_visionmodule)
{
    m_impl->SetFrameCount(n2D_Normal, n2D_HighPos, TR_visionmodule);
}

void SyncController::SetFramePeriod(float fTime, const enSideVisionModule& i_eVisionModuleidx)
{
    m_impl->SetFramePeriod(fTime, i_eVisionModuleidx);
}

void SyncController::SetFramePeriod(float fTime, const const enTRVisionmodule& TR_visionmodule)
{
    m_impl->SetFramePeriod(fTime, TR_visionmodule);
}

void SyncController::SetCameraTransferTime(
    float fTime, const enSideVisionModule& i_eVisionModuleidx) //kircheis_CamTrans
{
    m_impl->SetCameraTransferTime(fTime, i_eVisionModuleidx);
}

void SyncController::SetCameraTransferTime(
    float fTime, const const enTRVisionmodule& TR_visionmodule) //kircheis_CamTrans
{
    m_impl->SetCameraTransferTime(fTime, TR_visionmodule);
}

void SyncController::SetTriggerEdgeMode(
    BOOL bMainTriggerMode, BYTE GlobalShutterMode, BYTE RisingEdgeMode, const enSideVisionModule& i_eVisionModuleidx)
{
    m_impl->SetTriggerEdgeMode(bMainTriggerMode, GlobalShutterMode, RisingEdgeMode, i_eVisionModuleidx);
}

void SyncController::SetTriggerEdgeMode(
    BOOL bMainTriggerMode, BYTE GlobalShutterMode, BYTE RisingEdgeMode, const enTRVisionmodule& TR_visionmodule)
{
    m_impl->SetTriggerEdgeMode(bMainTriggerMode, GlobalShutterMode, RisingEdgeMode, TR_visionmodule);
}

void SyncController::SetReadySignal(BOOL bActive)
{
    m_impl->SetReadySignal(bActive);
}

BOOL SyncController::GetReadySignal() //kircheis_150417
{
    return m_impl->GetReadySignal();
}

void SyncController::SetGrabAcquisition(BOOL bActive)
{
    m_impl->SetGrabAcquisition(bActive);
}

BOOL SyncController::GetGrabAcquisition()
{
    return m_impl->GetGrabAcquisition();
}

void SyncController::SetSignalNGRV_VisionRecvPkgID(const BOOL& bActive) //kircheis_NGRV_Sync
{
    m_impl->SetSignalNGRV_VisionRecvPkgID(bActive);
}

BOOL SyncController::GetSignalNGRV_VisionRecvPkgID() //kircheis_NGRV_Sync
{
    return m_impl->GetSignalNGRV_VisionRecvPkgID();
}

void SyncController::SetSignalNGRV_VisionRecvGrabID(const BOOL& bActive) //kircheis_NGRV_Sync
{
    m_impl->SetSignalNGRV_VisionRecvGrabID(bActive);
}

BOOL SyncController::GetSignalNGRV_VisionRecvGrabID() //kircheis_NGRV_Sync
{
    return m_impl->GetSignalNGRV_VisionRecvGrabID();
}

void SyncController::SetSignalNGRV_VisionMoveIRPos(const BOOL& bActive) //kircheis_NGRV_Sync
{
    m_impl->SetSignalNGRV_VisionMoveIRPos(bActive);
}

BOOL SyncController::GetSignalNGRV_VisionMoveIRPos() //kircheis_NGRV_Sync
{
    return m_impl->GetSignalNGRV_VisionMoveIRPos();
}

void SyncController::SetSignalNGRV_VisionSaveImageReady(const BOOL& bActive) //kircheis_NGRV_Sync
{
    m_impl->SetSignalNGRV_VisionSaveImageReady(bActive);
}

BOOL SyncController::GetSignalNGRV_VisionSaveImageReady() //kircheis_NGRV_Sync
{
    return m_impl->GetSignalNGRV_VisionSaveImageReady();
}

BOOL SyncController::GetStartSignal()
{
    return m_impl->GetStartSignal();
}

UINT SyncController::GetFovIndex()
{
    return m_impl->GetFovIndex();
}

UINT SyncController::GetScanIDforSide()
{
    return m_impl->GetScanIDforSide();
}
UINT SyncController::GetStitchingIndexForSide()
{
    return m_impl->GetStitchingIndexForSide();
}

BOOL SyncController::GetGrabHighPosGrabBit()
{
    return m_impl->GetGrabHighPosGrabBit();
}

void SyncController::SetEncoderScaling(BYTE scaling)
{
    m_impl->SetEncoderScaling(scaling);
}

void SyncController::SetInspectionMode(const int grabMode3D)
{
    m_impl->SetInspectionMode(grabMode3D);
}

void SyncController::SetilluminationValue(const int illumValue)
{
    m_impl->SetilluminationValue(illumValue);
}

void SyncController::SetilluminationMode(const int illumMode)
{
    m_impl->SetilluminationMode(illumMode);
}

void SyncController::SetilluminationTime(const double illumSpecular, const double illumDefused)
{
    m_impl->SetilluminationTime(illumSpecular, illumDefused);
}

BOOL SyncController::Download_DVH(const BYTE& byAddress, const BYTE& byData)
{
    return m_impl->Download_DVH(byAddress, byData);
}

void SyncController::StartSyncBoard(BOOL bRetry, const int i_index)
{
    m_impl->StartSyncBoard(bRetry, i_index);
}

void SyncController::SetEncoderMultiply(BOOL useEncoder4Multiply)
{
    m_impl->SetEncoderMultiply(useEncoder4Multiply);
}

void SyncController::SetGrabPosition(unsigned short position)
{
    m_impl->SetGrabPosition(position);
}

void SyncController::SetGrabDirection(BYTE bGrabDir, BOOL bisDVH) //kircheis_GrabDir
{
    m_impl->SetGrabDirection(bGrabDir, bisDVH);
}

long SyncController::ReadCount(int& nNewCount) //kircheis_GrabDir
{
    return m_impl->ReadCount(nNewCount);
}

//bool SyncController::CalibrateIllumination2D()
//{
//	return m_pImpl->CalibrateIllumination2D();
//}

void SyncController::SetLongExposureCh(long nIRChID, long nUVChID) //kircheis_LongExp
{
    if (!IsAvailable())
        return;

    m_impl->SetLongExposureCh(nIRChID, nUVChID);
}

UINT SyncController::GetIndexStitchIDNGRV()
{
    return m_impl->GetIndexStitchIDNGRV();
}

BOOL SyncController::GetSignalNGRV_StartIR()
{
    return m_impl->GetSignalNGRV_StartIR();
}

BOOL SyncController::GetSignalNGRV_GrabID()
{
    return m_impl->GetSignalNGRV_GrabID();
}

// From CInterface_DIO
BOOL SyncController::GetInputBit(long nBitOrder, BOOL& bValue)
{
    BYTE byByteIndex = BYTE(nBitOrder / 8);
    BYTE byBitMask = 0x01 << BYTE(nBitOrder % 8);

    return m_impl->GetInputBit(byByteIndex, byBitMask, bValue);
}

BOOL SyncController::GetOutputBit(long nBitOrder, BOOL& bValue)
{
    BYTE byByteIndex = BYTE(nBitOrder / 8);
    BYTE byBitMask = 0x01 << BYTE(nBitOrder % 8);

    return m_impl->GetOutputBit(byByteIndex, byBitMask, bValue);
}

BOOL SyncController::SetOutputBit(long nBitOrder, BOOL bValue)
{
    BYTE byByteIndex = BYTE(nBitOrder / 8);
    BYTE byBitMask = 0x01 << BYTE(nBitOrder % 8);

    return m_impl->SetOutputBit(byByteIndex, byBitMask, bValue);
}

// Bit Write/Read
BOOL SyncController::SetOutputBit(const BYTE& byOrder, const BOOL& bOn)
{
    return m_impl->SetOutputBit(byOrder, bOn);
}

BOOL SyncController::GetOutputBit(const BYTE& byOrder, BOOL& bOn)
{
    return m_impl->GetOutputBit(byOrder, bOn);
}

BOOL SyncController::GetInputBit(const BYTE& byOrder, BOOL& bOn)
{
    return m_impl->GetInputBit(byOrder, bOn);
}
