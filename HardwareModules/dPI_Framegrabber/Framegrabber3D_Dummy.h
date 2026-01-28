#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "FrameGrabber_Base.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
namespace IntekPlus
{
namespace VisionMadang
{
class SbtCore;
}
} // namespace IntekPlus

//HDR_6_________________________________ Header body
//
class FrameGrabber3D_Dummy : public FrameGrabber_Base
{
public:
    FrameGrabber3D_Dummy(void);
    ~FrameGrabber3D_Dummy(void) override;

public: // 그랩 함수
    virtual bool grab(const int grabber_idx, const int camera_idx, const int stitchIndex, Ipvm::Image32r& zmapImage,
        Ipvm::Image16u& vmapImage, float& height_range_min, float& height_range_max,
        Ipvm::AsyncProgress* progress) override;

public: // 3D 전용 함수
    virtual void set_live_image_size(
        const int& grabber_idx, const int& camera_idx, const bool& use_binning, const bool& is_full_size) override;

public: // Camera Control
    virtual bool OpenCameraComm(const int& camera_id) override;
    virtual bool CloseCameraComm() override;
    virtual bool SetDigitalGainValue(float i_fInputGain, BOOL bInline) override;
    virtual float GetCurrentCameraGain() override;

protected:
    virtual bool LiveCallback(const int& graaber_id = 0, const int& camera_id = 0) override;
    virtual bool SetTriggerMode(int nMode) override;

private:
    //////////////////////////////////////////////////////////////////////////
    long m_profileNumber;

    int m_iWidth;
    int m_iHeight;
    int m_iPixelBit;
    UINT m_iFrameSize;

    BYTE** m_ppRawFirstBuff;
    float* m_pReliability;
    float* m_pIntensity;

    long m_nGrabbedImageCount;
};
