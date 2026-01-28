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
//HDR_6_________________________________ Header body
//
class FOVImageGrabberNGRV : public FOVImageGrabber
{
public:
    FOVImageGrabberNGRV(CVisionMain* visionMain);
    virtual ~FOVImageGrabberNGRV();

    void allocateBuffer(long totalFrameCount, long highPosFrameCount);

    BOOL GetPackageID(long& o_nPackageID, CString& o_strPackageID); //kircheis_NGRV
    BOOL GetGrabPointInfo(long i_nPackageID, long& o_nGrabPointID, long& o_nStitchID, long& o_nStitchX,
        long& o_nStitchY, std::vector<long>& o_vecnFrameID, BOOL& o_bUseIR, BOOL& o_bIsFinalGrab); //kircheis_NGRV
    BOOL GetGrabPointInfo(long i_nPackageID, long& o_nGrabPointID, long& o_nStitchID, long& o_nStitchX,
        long& o_nStitchY, std::vector<long>& o_vecnFrameID, std::vector<long>& o_vecnUVIRFrameID, BOOL& o_bUseIR,
        BOOL& o_bUseUV, BOOL& o_bIsFinalGrab); //kircheis_NGRVGain
    void SetVisionAcquisitionNGRV(BOOL bActive); //kircheis_NGRV
    void SetNGRVSaveImageBuffer(BYTE** i_pbyGrabbedImage, long i_nTotalFrameCount, long i_nCameraImageSizeX,
        long i_nCameraImageSizeY, long i_nStitchID, BOOL i_bFinalGrab, long i_nReverseFrameID);

    bool buffer_grab_SingleRun(long bufferIndex, const FOVID& fovID,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);

protected:
    struct FOVBuffer
    {
        BYTE* m_imagePtrs[LED_ILLUM_NGRV_CHANNEL_MAX];
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
    bool grab_retry(const long& retry_count, const short& total_grab_frame, BYTE** buffer);
};