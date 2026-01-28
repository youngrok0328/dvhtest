#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "SyncController_Base.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class SyncController_NGRV : public SyncController_Base
{
public:
    SyncController_NGRV(void);
    virtual ~SyncController_NGRV(void);

public:
    virtual BOOL IsAvailable() override;
    virtual void PopupDioCheckDialog() override;
    virtual void ReadCPLDVersion(BYTE& byHighValue, BYTE& byLowValue) override;
    virtual void ReadAVRVersion(BYTE& byHighValue, BYTE& byLowValue) override;
    virtual long GetVisionType() override;

    virtual void TurnOnLight(long frameIndex,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT) override;
    virtual void TurnOffLight(
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT) override;

    virtual void SetReadySignal(const BOOL& bActive) override;
    virtual BOOL GetReadySignal(); //kircheis_150417 override;
    virtual void SetGrabAcquisition(const BOOL& bActive) override;
    virtual BOOL GetGrabAcquisition() override;
    virtual void SetGrabExposure(const BOOL& bActive) override;
    virtual BOOL GetGrabExposure() override;

    virtual BOOL GetStartSignal() override;
    virtual UINT GetFovIndex() override;
    virtual UINT GetScanIDforSide() override;
    virtual UINT GetStitchingIndexForSide() override;
    virtual void SetTriggerEdgeMode(BOOL bMainTriggerMode, BYTE GlobalShutterMode, BYTE RisingEdgeMode,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT) override;

    // Bit Write/Read
    virtual BOOL SetOutputBit(const BYTE& byOrder, const BOOL& bOn) override;
    virtual BOOL GetOutputBit(const BYTE& byOrder, BOOL& bOn) override;
    virtual BOOL GetInputBit(const BYTE& byOrder, BOOL& bOn) override;

    virtual BOOL GetInputBit(const BYTE& byByteIndex, const BYTE& byBitMask, BOOL& bValue) override;
    virtual BOOL GetOutputBit(const BYTE& byByteIndex, const BYTE& byBitMask, BOOL& bValue) override;
    virtual BOOL SetOutputBit(const BYTE& byByteIndex, const BYTE& byBitMask, const BOOL& bValue) override;

    virtual BOOL Download(const BYTE& byAddress, const BYTE& byData) override;

    virtual void StartSyncBoard(BOOL bRetry = FALSE,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT) override;
    virtual void StartSyncBoardHighPosGrab(BOOL bRetry = FALSE,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT) override;

public:
    virtual void SetIllumiParameter(const long& nFrame, const std::array<float, 16>& illums_ms,
        const bool& adjustIntensity = true, const bool& isVerification = false, const BYTE& verificationType = 0,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT) override;
    virtual void SetIllumiParameter(const long& nFrame, const long& nChannel, const float& illum_ms,
        const bool& adjustIntensity = true, const bool& isVerification = false, const BYTE& verificationType = 0,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT) override;
    virtual void Set2DCurrentFrame(long nFrame,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT) override;
    virtual void SetFrameCount(long n2D_Normal, long n2D_HighPos,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    virtual void SetFramePeriod(
        float fTime, const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    virtual void SetCameraTransferTime(float fTime,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT); //kircheis_CamTrans

    virtual void SetSignalNGRV_VisionRecvPkgID(const BOOL& bActive) override; //kircheis_NGRV_Sync
    virtual BOOL GetSignalNGRV_VisionRecvPkgID() override; //kircheis_NGRV_Sync
    virtual void SetSignalNGRV_VisionRecvGrabID(const BOOL& bActive) override; //kircheis_NGRV_Sync
    virtual BOOL GetSignalNGRV_VisionRecvGrabID() override; //kircheis_NGRV_Sync
    virtual void SetSignalNGRV_VisionMoveIRPos(const BOOL& bActive) override; //kircheis_NGRV_Sync
    virtual BOOL GetSignalNGRV_VisionMoveIRPos() override; //kircheis_NGRV_Sync
    virtual void SetSignalNGRV_VisionSaveImageReady(const BOOL& bActive) override; //kircheis_NGRV_Save
    virtual BOOL GetSignalNGRV_VisionSaveImageReady() override; //kircheis_NGRV_Save
    virtual void SetLongExposureCh(long nIRChID, long nUVChID) override; //kircheis_LongExp

    virtual UINT GetIndexStitchIDNGRV() override; //kircheis_NGRV_Sync
    virtual BOOL GetSignalNGRV_StartIR() override; //kircheis_NGRV_Sync
    virtual BOOL GetSignalNGRV_GrabID() override; //kircheis_NGRV_Sync

private:
    void Set2DLedIntensity(const long& nFrame, const long& nChannel, const float& duration_ms,
        const bool& bIsLongExposure = false); //kircheis_LongExp
    bool GetisVirtualMode() const;

private:
    HANDLE m_hDevice;

    UINT m_maskFovIndex;

    bool m_bisVirtualMode;

    float m_arfInputExpTime[10][16]; //kircheis_IllumHybrid
    float m_arfOutputExpTime[10][16]; //kircheis_IllumHybrid

private:
    double m_illuminationGains[LED_ILLUM_CHANNEL_MAX];
    double m_illuminationGainsMirror[LED_ILLUM_CHANNEL_SIDE_DEFAULT]; //SDY_DualCal Mirror Gain
};