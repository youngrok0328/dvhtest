#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <memory>
#include <vector>

//HDR_5_________________________________ Forward declarations
class VisionMainTR;
class VisionImageLot;

//HDR_6_________________________________ Header body
//
class FOVID
{
public:
    FOVID();
    FOVID(LPCTSTR trayKey, long runtimeIndex, long trayIndex, long fovIndex, long stitchIndex = -1);

    CString m_trayKey;
    long m_runtimeIndex;
    long m_trayIndex;
    long m_fovIndex;
    long m_stitchIndex;
};

class FOVImageGrabber
{
public:
    FOVImageGrabber(VisionMainTR* visionMain);
    virtual ~FOVImageGrabber();

    void setAllBuffersToIdle();
    FOVID getBufferFovID(long bufferIndex);
    void getValidBufferList(std::vector<long>& o_validList);
    bool getIdleBufferIndex(HANDLE signalForOut1, HANDLE signalForOut2, long& o_bufferIndex);
    void setIdleBuffer(long bufferIndex);
    void linkDecrease(long bufferIndex);
    bool grab(long bufferIndex, long linkCount, const FOVID& fovID,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    void paste(long bufferIndex, long stitchIndex, long stitchCountX, long stitchCountY,
        std::shared_ptr<VisionImageLot> inspectionImage);
    void pasteforSide(long i_nBufferIndex, long i_nStitchIndex, long i_nScanID, long i_nStitchCountX,
        long i_nStitchCountY, std::shared_ptr<VisionImageLot> inspectionImage);

    //{{ mc_GPU Calc. LOG
    virtual void ClearImageGrabber_GrabTime() = 0;
    virtual void SaveImageGrabber_GrabTime(const CString i_strSavePath) = 0;
    //}}

    VisionMainTR& GetVisionMainInfo();

protected:
    VisionMainTR* m_visionMain;

    struct FOVBaseBuffer
    {
        FOVID m_fovID;
        long m_linkCount;

        HANDLE m_handleReady;
        bool m_grabed;
        bool m_isReady;
    };

    static const long m_bufferMaxCount = 62;
    CCriticalSection m_csLock;
    std::shared_ptr<FOVBaseBuffer> m_baseBuffers[m_bufferMaxCount];

    void _setIdleBuffer(long bufferIndex);
    void _writeLog_BufferState();

    virtual bool buffer_grab(long bufferIndex, const FOVID& fovID, const enSideVisionModule i_eSideVisionModule) = 0;
    virtual void buffer_paste(long bufferIndex, long stitchIndex, long stitchCountX, long stitchCountY,
        std::shared_ptr<VisionImageLot> inspectionImage)
        = 0;
};
