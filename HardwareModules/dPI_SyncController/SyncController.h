#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <array>

//HDR_5_________________________________ Forward declarations
class SyncController_Base;

//HDR_6_________________________________ Header body
//
class DPI_SYNCCONTROLLERDEF_API SyncController
{
    SyncController(void);
    ~SyncController(void);

public:
    static SyncController& GetInstance();

public:
    BOOL IsAvailable();
    void PopupDioCheckDialog();
    SyncController_Base* GetSync();

public: //I/O 관련
    void SetReadySignal(BOOL bActive);
    BOOL GetReadySignal(); //kircheis_150417
    void SetGrabAcquisition(BOOL bActive);
    BOOL GetGrabAcquisition();

    BOOL GetStartSignal();
    UINT GetFovIndex();
    UINT GetScanIDforSide();
    UINT GetStitchingIndexForSide();
    BOOL GetGrabHighPosGrabBit();
    void SetTriggerEdgeMode(BOOL bMainTriggerMode, BYTE GlobalShutterMode, BYTE RisingEdgeMode,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void SetTriggerEdgeMode(BOOL bMainTriggerMode, BYTE GlobalShutterMode, BYTE RisingEdgeMode,
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET);

    // From CInterface_DIO
    BOOL GetInputBit(long nBitOrder, BOOL& bValue);
    BOOL GetOutputBit(long nBitOrder, BOOL& bValue);
    BOOL SetOutputBit(long nBitOrder, BOOL bValue);

    // Bit Write/Read
    BOOL SetOutputBit(const BYTE& byOrder, const BOOL& bOn);
    BOOL GetOutputBit(const BYTE& byOrder, BOOL& bOn);
    BOOL GetInputBit(const BYTE& byOrder, BOOL& bOn);

public: //공용
    void SetGrabExposure(BOOL bOn);
    BOOL GetGrabExposure();
    void TurnOnLight(long frameIndex,
        const enSideVisionModule& i_eVisionModuleidx
        = enSideVisionModule::SIDE_VISIONMODULE_FRONT); // For Slitbeam illumination
    void TurnOnLight(long frameIndex,
        const enTRVisionmodule& TR_modules = enTRVisionmodule::TR_VISIONMODULE_INPOCKET); // For Slitbeam illumination
    void TurnOffLight(const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void TurnOffLight(const enTRVisionmodule& TR_modules = enTRVisionmodule::TR_VISIONMODULE_INPOCKET);

    

    BOOL Download(const BYTE& byAddress, const BYTE& byData);
    BOOL Download(const BYTE& byAddress, const BYTE& byData, const enSideVisionModule& i_eVisionModuleidx);
    BOOL Download(const BYTE& byAddress, const BYTE& byData, const enTRVisionmodule& TR_visionmodule);

    void ReadCPLDVersion(BYTE& byHighValue, BYTE& byLowValue);
    void ReadAVRVersion(BYTE& byHighValue, BYTE& byLowValue);

public: //2D 관련
    void StartSyncBoard(BOOL bRetry = FALSE,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void StartSyncBoard(BOOL bRetry = FALSE, const bool& single_mode = false,
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET);
    void SetIllumiParameter(long nFrame, long nChannel, float illum_um, const bool adjustIntensity = true,
        const bool isVerification = false, const BYTE verificationType = 0,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void SetIllumiParameter(long nFrame, long nChannel, float illum_um, const bool adjustIntensity = true,
        const bool isVerification = false, const BYTE verificationType = 0,
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET);
    void SetIllumiParameter(long nFrame, const std::array<float, 16>& illums_ms, const bool adjustIntensity = true,
        const bool isVerification = false, const BYTE verificationType = 0,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void SetIllumiParameter(long nFrame, const std::array<float, 16>& illums_ms, const bool adjustIntensity = true,
        const bool isVerification = false, const BYTE verificationType = 0,
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET);
    void StartSyncBoardHighPosGrab(BOOL bRetry = FALSE,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void StartSyncBoardHighPosGrab(
        BOOL bRetry = FALSE, const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET);
    void SetFrameCount(long n2D_Normal, long n2D_HighPos,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void SetFrameCount(long n2D_Normal, long n2D_HighPos,
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET);
    void SetFramePeriod(
        float fTime, const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void SetFramePeriod(
        float fTime, const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET);
    void SetCameraTransferTime(float fTime,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT); //kircheis_CamTrans
    void SetCameraTransferTime(float fTime,
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET); //kircheis_CamTrans

public: //3D 관련
    void SetEncoderScaling(BYTE scaling);
    void SetEncoderMultiply(BOOL useEncoder4Multiply);
    void SetGrabPosition(unsigned short position);
    void SetGrabDirection(BYTE bGrabDir, BOOL bisDVH = false); //kircheis_GrabDir //순뱡향(0-Default), 역방향(1)
    long ReadCount(int& nNewCount); //kircheis_GrabDir

    // DVH Vision
    void TurnOnLight(long frameIndex, const int& i_nDualMode);
    void TurnOffLight(const int& i_nDualMode);
    
    void SetInspectionMode(const int grabMode3D);
    void SetilluminationTime(const double illumSpecular, const double illumDefused);
    void SetilluminationValue(const int illumValue);
    void SetilluminationMode(const int illumMode);
    BOOL Download_DVH(const BYTE& byAddress, const BYTE& byData);
    void StartSyncBoard(BOOL bRetry = FALSE, const int i_index = (int)enDualGrabImageType::DUAL_IMAGE_TOTAL_REFLECTION);

public: //NGRV 관련
    void SetSignalNGRV_VisionRecvPkgID(const BOOL& bActive); //kircheis_NGRV_Sync
    BOOL GetSignalNGRV_VisionRecvPkgID(); //kircheis_NGRV_Sync
    void SetSignalNGRV_VisionRecvGrabID(const BOOL& bActive); //kircheis_NGRV_Sync
    BOOL GetSignalNGRV_VisionRecvGrabID(); //kircheis_NGRV_Sync
    void SetSignalNGRV_VisionMoveIRPos(const BOOL& bActive); //kircheis_NGRV_Sync
    BOOL GetSignalNGRV_VisionMoveIRPos(); //kircheis_NGRV_Sync
    void SetSignalNGRV_VisionSaveImageReady(const BOOL& bActive); //kircheis_NGRV_Save
    BOOL GetSignalNGRV_VisionSaveImageReady(); //kircheis_NGRV_Save
    void SetLongExposureCh(long nIRChID, long nUVChID); //kircheis_LongExp

    UINT GetIndexStitchIDNGRV(); //kircheis_NGRV_Sync
    BOOL GetSignalNGRV_StartIR(); //kircheis_NGRV_Sync
    BOOL GetSignalNGRV_GrabID(); //kircheis_NGRV_Sync

private:
    SyncController_Base* m_impl;
};
