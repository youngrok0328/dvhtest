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

enum TR_MODULE_OFFSET
{
    OFFSET_UNKNOWN_VAL = 99999,

    OFFSET_OTI_ADDR_OFFSET = 0,
    OFFSET_OTI_LOW_DATA_OFFSET,
    OFFSET_OTI_HIGH_DATA_OFFSET,

    OFFSET_INPOCKET_ADDR_OFFSET = 12,
    OFFSET_INPOCKET_LOW_DATA_OFFSET,
    OFFSET_INPOCKET_HIGH_DATA_OFFSET,
};

struct TR_OFFSET_VAL
{
    ULONG offset_addr;
    ULONG offset_low_data;
    ULONG offset_high_data;

    void set_offset_val(const enTRVisionmodule& TR_visionmodule)
    {
        switch (TR_visionmodule)
        {
            case enTRVisionmodule::TR_VISIONMODULE_OTI:
            {
                offset_addr = TR_MODULE_OFFSET::OFFSET_OTI_ADDR_OFFSET;
                offset_low_data = TR_MODULE_OFFSET::OFFSET_OTI_LOW_DATA_OFFSET;
                offset_high_data = TR_MODULE_OFFSET::OFFSET_OTI_HIGH_DATA_OFFSET;
            }
            break;
            case enTRVisionmodule::TR_VISIONMODULE_INPOCKET:
            {
                offset_addr = TR_MODULE_OFFSET::OFFSET_INPOCKET_ADDR_OFFSET;
                offset_low_data = TR_MODULE_OFFSET::OFFSET_INPOCKET_LOW_DATA_OFFSET;
                offset_high_data = TR_MODULE_OFFSET::OFFSET_INPOCKET_HIGH_DATA_OFFSET;
            }
            break;
            default:
            {
                offset_addr = TR_MODULE_OFFSET::OFFSET_UNKNOWN_VAL;
                offset_low_data = TR_MODULE_OFFSET::OFFSET_UNKNOWN_VAL;
                offset_high_data = TR_MODULE_OFFSET::OFFSET_UNKNOWN_VAL;
            }
            break;
        }
    }
};

enum TR_REAL_CHANNEL_ADDR
{
    OTI_CH1 = 0x00,
    OTI_CH2 = 0x01,
    OTI_REVERSE = 0x05,
    INPOCKET_CH1 = 0x0A,
    INPOCKET_CH2 = 0x0B,
    INPOCKET_RING = 0x0C,
};

class SyncController_TR : public SyncController_Base
{
public:
    SyncController_TR(void);
    virtual ~SyncController_TR(void);

public:
    virtual BOOL IsAvailable() override;
    virtual void PopupDioCheckDialog() override;
    virtual void ReadCPLDVersion(BYTE& byHighValue, BYTE& byLowValue) override;
    virtual void ReadAVRVersion(BYTE& byHighValue, BYTE& byLowValue) override;
    virtual long GetVisionType() override;

    virtual void TurnOnLight(
        long frameIndex, const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET) override;
    virtual void TurnOffLight(
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET) override;

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
    virtual BOOL GetGrabHighPosGrabBit() override;
    virtual void SetTriggerEdgeMode(BOOL bMainTriggerMode, BYTE GlobalShutterMode, BYTE RisingEdgeMode,
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET) override;

    // Bit Write/Read
    virtual BOOL SetOutputBit(const BYTE& byOrder, const BOOL& bOn) override;
    virtual BOOL GetOutputBit(const BYTE& byOrder, BOOL& bOn) override;
    virtual BOOL GetInputBit(const BYTE& byOrder, BOOL& bOn) override;

    virtual BOOL GetInputBit(const BYTE& byByteIndex, const BYTE& byBitMask, BOOL& bValue) override;
    virtual BOOL GetOutputBit(const BYTE& byByteIndex, const BYTE& byBitMask, BOOL& bValue) override;
    virtual BOOL SetOutputBit(const BYTE& byByteIndex, const BYTE& byBitMask, const BOOL& bValue) override;

    virtual BOOL Download(const BYTE& byAddress, const BYTE& byData,
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET) override;

    virtual void StartSyncBoard(BOOL bRetry = FALSE, const bool& single_mode = false,
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET) override;
    virtual void StartSyncBoardHighPosGrab(BOOL bRetry = FALSE,
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET) override;

public:
    virtual void SetIllumiParameter(const long& nFrame, const std::array<float, 16>& illums_ms,
        const bool& adjustIntensity = true, const bool& isVerification = false, const BYTE& verificationType = 0,
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET) override;
    virtual void SetIllumiParameter(const long& nFrame, const long& nChannel, const float& illum_ms,
        const bool& adjustIntensity = true, const bool& isVerification = false, const BYTE& verificationType = 0,
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET) override;

    virtual void Set2DCurrentFrame(
        long nFrame, const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET) override;
    virtual void SetFrameCount(long n2D_Normal, long n2D_HighPos,
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET);
    virtual void SetFramePeriod(
        float fTime, const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET);
    virtual void SetCameraTransferTime(float fTime,
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_INPOCKET); //kircheis_CamTrans

private:
    void Set2DLedIntensity(const long& nFrame, const long& nChannel, const float& duration_ms,
        const bool& bIsLongExposure = false,
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_OTI); //kircheis_LongExp
    bool GetisVirtualMode() const;

    void init_use_illum_data(const BYTE& byAddress, const BYTE& data,
        const enTRVisionmodule& TR_visionmodule = enTRVisionmodule::TR_VISIONMODULE_OTI);

private:
    HANDLE m_hDevice;

    UINT m_maskFovIndex;

    bool m_bisVirtualMode;

private:
    float m_arfInputExpTime[10][16]; //kircheis_IllumHybrid
    float m_arfOutputExpTime[10][16]; //kircheis_IllumHybrid

private:
    unsigned char get_TR_real_channel_addr(const long& idx_channel, const enTRVisionmodule& TR_visionmodule) const;
};