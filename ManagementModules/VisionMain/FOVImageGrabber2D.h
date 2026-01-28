#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "FOVImageGrabber.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <memory>
#include <vector>

//HDR_5_________________________________ Forward declarations
struct sGrab2DErrorLogMessageFormat;

//HDR_6_________________________________ Header body
//
class FOVImageGrabber2D : public FOVImageGrabber
{
public:
    FOVImageGrabber2D(CVisionMain* visionMain);
    virtual ~FOVImageGrabber2D();

    void allocateBuffer(long totalFrameCount, long highPosFrameCount);

protected:
    struct FOVBuffer
    {
        BYTE* m_imagePtrs[LED_ILLUM_FRAME_MAX];
    };

    long m_totalFrameCount;
    long m_highPosFrameCount;
    std::vector<std::shared_ptr<FOVBuffer>> m_buffers;

    virtual bool buffer_grab(long bufferIndex, const FOVID& fovID,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT) override;
    virtual void buffer_paste(long bufferIndex, long stitchIndex, long stitchCountX, long stitchCountY,
        std::shared_ptr<VisionImageLot> inspectionImage) override;
    virtual void ClearImageGrabber_GrabTime() override;
    virtual void SaveImageGrabber_GrabTime(const CString i_strSavePath) override;

private:
    void Send_Grab2D_Error_LogMessage(
        const CString i_strCategory, const sGrab2DErrorLogMessageFormat i_sGrab2DErrorForamt);

    bool buffer_grab_2D(const int& idx_grabber, const int& idx_camera, long bufferIndex, const FOVID& fovID,
        const enSideVisionModule i_eSideVisionModule);
    bool buffer_grab_NGRV_insepction(const int& idx_grabber, const int& idx_camera, long bufferIndex,
        const FOVID& fovID, const enSideVisionModule i_eSideVisionModule);
        bool Conversion_image_green(BYTE** Grabbedimage, long bufferIndex, const int& totalFrameCount);
};
