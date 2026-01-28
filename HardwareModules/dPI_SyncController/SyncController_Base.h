#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <array>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class SyncController_Base
{
public:
    SyncController_Base(void);
    virtual ~SyncController_Base(void);

public:
    virtual BOOL IsAvailable();
    virtual void PopupDioCheckDialog();
    virtual long GetVisionType();

public: //공용
    virtual void TurnOnLight(long frameIndex,
        const enSideVisionModule& i_eVisionModuleidx
        = enSideVisionModule::SIDE_VISIONMODULE_FRONT); // For Slitbeam illumination
    virtual void TurnOnLight(long frameIndex,
        const enTRVisionmodule& TR_visionmodule
        = enTRVisionmodule::TR_VISIONMODULE_INPOCKET); // For Slitbeam illumination
    virtual void TurnOffLight(
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    virtual void TurnOffLight(const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET);

    virtual void StartSyncBoard(BOOL bRetry = FALSE,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    virtual void StartSyncBoard(BOOL bRetry = FALSE, const bool& single_mode = false,
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET);
    virtual void SetReadySignal(const BOOL& bActive);
    virtual BOOL GetReadySignal(); //kircheis_150417
    virtual void SetGrabAcquisition(const BOOL& bActive);
    virtual BOOL GetGrabAcquisition();
    virtual void SetGrabExposure(const BOOL& bActive);
    virtual BOOL GetGrabExposure();

    virtual BOOL GetStartSignal();
    virtual UINT GetFovIndex();
    virtual UINT GetScanIDforSide();
    virtual UINT GetStitchingIndexForSide();
    virtual BOOL GetGrabHighPosGrabBit();
    virtual void SetTriggerEdgeMode(BOOL bMainTriggerMode, BYTE GlobalShutterMode, BYTE RisingEdgeMode,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    virtual void SetTriggerEdgeMode(BOOL bMainTriggerMode, BYTE GlobalShutterMode, BYTE RisingEdgeMode,
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET);

    // Bit Write/Read
    virtual BOOL SetOutputBit(const BYTE& byOrder, const BOOL& bOn);
    virtual BOOL GetOutputBit(const BYTE& byOrder, BOOL& bOn);
    virtual BOOL GetInputBit(const BYTE& byOrder, BOOL& bOn);

    virtual BOOL GetInputBit(const BYTE& byByteIndex, const BYTE& byBitMask, BOOL& bValue);
    virtual BOOL GetOutputBit(const BYTE& byByteIndex, const BYTE& byBitMask, BOOL& bValue);
    virtual BOOL SetOutputBit(const BYTE& byByteIndex, const BYTE& byBitMask, const BOOL& bValue);

    virtual BOOL Download(const BYTE& byAddress, const BYTE& byData);
    virtual BOOL Download(const BYTE& byAddress, const BYTE& byData, const enSideVisionModule& i_eVisionModuleidx);
    virtual BOOL Download(const BYTE& byAddress, const BYTE& byData, const enTRVisionmodule& TR_visionmodule);

    virtual void ReadCPLDVersion(BYTE& byHighValue, BYTE& byLowValue);
    virtual void ReadAVRVersion(BYTE& byHighValue, BYTE& byLowValue);

public: //2D 관련
    virtual void SetIllumiParameter(const long& nFrame, const std::array<float, 16>& illums_ms,
        const bool& adjustIntensity = true, const bool& isVerification = false, const BYTE& verificationType = 0,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    virtual void SetIllumiParameter(const long& nFrame, const std::array<float, 16>& illums_ms,
        const bool& adjustIntensity = true, const bool& isVerification = false, const BYTE& verificationType = 0,
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET);
    virtual void SetIllumiParameter(const long& nFrame, const long& nChannel, const float& illum_ms,
        const bool& adjustIntensity = true, const bool& isVerification = false, const BYTE& verificationType = 0,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    virtual void SetIllumiParameter(const long& nFrame, const long& nChannel, const float& illum_ms,
        const bool& adjustIntensity = true, const bool& isVerification = false, const BYTE& verificationType = 0,
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET);
    virtual void StartSyncBoardHighPosGrab(BOOL bRetry = FALSE,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    virtual void StartSyncBoardHighPosGrab(
        BOOL bRetry = FALSE, const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET);
    virtual void Set2DCurrentFrame(
        long nFrame, const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    virtual void Set2DCurrentFrame(
        long nFrame, const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET);
    virtual void SetFrameCount(long n2D_Normal, long n2D_HighPos,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    virtual void SetFrameCount(long n2D_Normal, long n2D_HighPos,
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET);
    virtual void SetFramePeriod(
        float fTime, const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    virtual void SetFramePeriod(
        float fTime, const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET);
    virtual void SetCameraTransferTime(float fTime,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT); //kircheis_CamTrans
    virtual void SetCameraTransferTime(float fTime,
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET); //kircheis_CamTrans

public: //3D 관련
    virtual void SetEncoderScaling(BYTE scaling);
    virtual void SetEncoderMultiply(BOOL useEncoder4Multiply);
    virtual void SetGrabPosition(unsigned short position);
    virtual void SetGrabDirection(BYTE bGrabDir, BOOL bisDVH = false); //kircheis_GrabDir //순뱡향(0-Default), 역방향(1)
    virtual long ReadCount(int& nNewCount); //kircheis_GrabDir

public://3D DVH 관련
    virtual void TurnOnLight(long frameIndex, const int& i_nDualMode = 0); // For Slitbeam illumination
    virtual void TurnOffLight(const int& i_nDualMode = 0); // For Slitbeam illumination
    virtual void SetInspectionMode(const int grabMode3D);
    virtual void SetilluminationTime(const double illumSpecular, const double illumDefused);
    virtual void SetilluminationValue(const int illumValue);
    virtual void SetilluminationMode(const int illumMode);
    virtual BOOL Download_DVH(const BYTE& byAddress, const BYTE& byData);
    virtual void StartSyncBoard(
        BOOL bRetry = FALSE, const int i_index = (int)enDualGrabImageType::DUAL_IMAGE_TOTAL_REFLECTION);

public: //NGRV 관련
    virtual void SetSignalNGRV_VisionRecvPkgID(const BOOL& bActive); //kircheis_NGRV_Sync
    virtual BOOL GetSignalNGRV_VisionRecvPkgID(); //kircheis_NGRV_Sync
    virtual void SetSignalNGRV_VisionRecvGrabID(const BOOL& bActive); //kircheis_NGRV_Sync
    virtual BOOL GetSignalNGRV_VisionRecvGrabID(); //kircheis_NGRV_Sync
    virtual void SetSignalNGRV_VisionMoveIRPos(const BOOL& bActive); //kircheis_NGRV_Sync
    virtual BOOL GetSignalNGRV_VisionMoveIRPos(); //kircheis_NGRV_Sync
    virtual void SetSignalNGRV_VisionSaveImageReady(const BOOL& bActive); //kircheis_NGRV_Save
    virtual BOOL GetSignalNGRV_VisionSaveImageReady(); //kircheis_NGRV_Save
    virtual void SetLongExposureCh(long nIRChID, long nUVChID); //kircheis_LongExp

    virtual UINT GetIndexStitchIDNGRV(); //kircheis_NGRV_Sync
    virtual BOOL GetSignalNGRV_StartIR(); //kircheis_NGRV_Sync
    virtual BOOL GetSignalNGRV_GrabID(); //kircheis_NGRV_Sync

    //bool CalibrateIllumination2D();
};