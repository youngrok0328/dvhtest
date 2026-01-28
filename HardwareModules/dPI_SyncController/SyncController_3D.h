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
class SyncController_3D : public SyncController_Base
{
public:
    SyncController_3D(void);
    virtual ~SyncController_3D(void);

public:
    virtual BOOL IsAvailable() override;
    virtual void PopupDioCheckDialog() override;
    virtual void ReadCPLDVersion(BYTE& byHighValue, BYTE& byLowValue) override;
    virtual void ReadAVRVersion(BYTE& byHighValue, BYTE& byLowValue) override;
    virtual long GetVisionType() override;

    virtual void TurnOnLight(long frameIndex, const int& i_nDualMode = 0) override;
    virtual void TurnOffLight(const int& i_nDualMode = 0) override;

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

    // Bit Write/Read
    virtual BOOL SetOutputBit(const BYTE& byOrder, const BOOL& bOn) override;
    virtual BOOL GetOutputBit(const BYTE& byOrder, BOOL& bOn) override;
    virtual BOOL GetInputBit(const BYTE& byOrder, BOOL& bOn) override;

    virtual BOOL GetInputBit(const BYTE& byByteIndex, const BYTE& byBitMask, BOOL& bValue) override;
    virtual BOOL GetOutputBit(const BYTE& byByteIndex, const BYTE& byBitMask, BOOL& bValue) override;
    virtual BOOL SetOutputBit(const BYTE& byByteIndex, const BYTE& byBitMask, const BOOL& bValue) override;

    virtual BOOL Download(const BYTE& byAddress, const BYTE& byData) override;

    virtual void SetInspectionMode(const int grabMode3D);
    virtual void SetilluminationTime(const double illumSpecular, const double illumDefused);
    virtual void SetilluminationValue(const int illumValue) override;
    virtual void SetilluminationMode(const int illumMode) override;
    virtual BOOL Download_DVH(const BYTE& byAddress, const BYTE& byData) override;
    virtual void StartSyncBoard(
        BOOL bRetry = FALSE, const int i_index = (int)enDualGrabImageType::DUAL_IMAGE_TOTAL_REFLECTION) override;

public:
    void SetEncoderScaling(BYTE scaling) override;
    void SetEncoderMultiply(BOOL useEncoder4Multiply) override;
    void SetGrabPosition(unsigned short position) override;
    void SetGrabDirection(BYTE bGrabDir, BOOL bisDVH = false) override; //kircheis_GrabDir //순뱡향(0-Default), 역방향(1)
    long ReadCount(int& nNewCount) override;
    ; //kircheis_GrabDir

private:
    bool GetisVirtualMode() const;

private:
    HANDLE m_hDevice;

    UINT m_maskFovIndex;

    bool m_bisVirtualMode;
};