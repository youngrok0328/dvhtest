#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u3.h>
#include <Ipvm/Base/Size32s2.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class FOVImageGrabber;
class ImageClassifier;
class ImageClassifier_Buffer;
class InspectionOrderer;
class VisionImageLot;
class VisionUnit;
class CVisionInlineUI;
class CVisionMain;
class NgrvGrabbedImageInfo;

//HDR_6_________________________________ Header body
//
class InlineGrab
{
public:
    InlineGrab(CVisionMain* main, long grabBufferNum, VisionUnit** unitBuffers, long unitNum);
    ~InlineGrab();

    void initialize2D(long totalFrameCount, long highPosFrameCount);
    void initialize3D(long logicalScanLength_px);
    void initializeColorVision(long totalFrameCount, long highPosFrameCount);
    void initializeSide(long totalFrameCount, long highPosFrameCount);

    InspectionOrderer& getInspectionOrderer();

    long GetTrayIndex() const;
    long GetInlineRunCnt() const;
    bool CopyLastGrabBufferTo(VisionImageLot& imageLot);

    bool PushBatchImage(long batchKey, LPCTSTR imageFilePath, long paneIndex, LPCTSTR batchBarcode);

    void SetScanTrayIndex(long trayIndex);
    void SetInlineRunCnt(long inlineRunCnt);
    void Start(bool isBatchInspection);
    void Stop();
    bool IsInlineMode();
    bool isIdleForBatch();

    void ResetInline_GrabTimeLOG();
    void SaveInline_GrabTimeLOG();

    const Ipvm::Size32s2& GetImageBufferSize() const;

private:
    Ipvm::Size32s2 m_imageBufferSize;
    FOVImageGrabber* m_fovImageGrabber;
    ImageClassifier* m_imageClassifier;
    InspectionOrderer* m_inspectionOrderer;
    VisionTrayScanSpec m_visionTrayScanSpec;

    CVisionMain* m_main;

    long m_grabBufferNum;
    long m_trayIndexFromHost;
    CString m_trayKey;
    long m_runCntFromHost;

    std::vector<int> m_needCountForFOV;

    CCriticalSection m_csBatchFile;
    bool m_isBatchInspection;

    struct BatchInfo
    {
        CString m_filePath;
        long m_paneIndex;
        long m_batchKey;
        CString m_batchBarcode;

        BatchInfo()
        {
        }
    };

    BatchInfo m_batchFilePath;

    HANDLE m_Signal_KillThread; // 인라인 그랩 쓰레드를 종료하는 명령을 내림
    HANDLE m_Flag_thread_idle; // Idle 상태임을 나타내는 플래그

    HANDLE m_Flag_thread_inlineMode; // 인라인임을 나타내는 이벤트 플래그 (Set만 하고 Thread에서 알아서 Reset되게 한다)
    HANDLE m_Flag_thread_offline; // 오프라인임을 나타내는 이벤트 플래그 (Set만 하고 Thread에서 알아서 Reset되게 한다)

    HANDLE m_Handle_Thread; // 쓰레드의 핸들을 가지고 있는다. 종료 대기시 사용.

    static UINT ThreadGrabFunc(LPVOID pParam);
    UINT ThreadGrabFunc();
    UINT ThreadGrabFunc_NGRV();
    bool ThreadGrabFunc_Batch();
    void ImageGrab(long grabBufferIndex);
    void ImageGrab2D(long grabBufferIndex);
    void ImageGrab3D(long grabBufferIndex);
    void ImageGrabColor(long grabBufferIndex);
    void ImageGrabSide(long grabBufferIndex);
    void CheckChangedVisionTrayScanSpec();

    //	NGRV Grab Thread
    static UINT ThreadNGRVSaveFunc(LPVOID pParam);
    UINT ThreadNGRVSaveFunc();
    HANDLE m_Handle_Save_Thread; // 쓰레드의 핸들을 가지고 있는다. 종료 대기시 사용.
    CMutex m_mutNGRVSave;
    void ImageSaveForNGRV(CString strImageSaveFullFilePath, Ipvm::Image8u3& ColorImage, long nImageSaveType);
    void ImageSaveNGRV();
    long GetDemosaicSizeY(long nCameraSizeX, long nCameraSizeY, long nStitchSizeX,
        long nStitchSizeY); // Color Conversion Image Stitching Region Division
    void SplitColorImageToSendHost(
        long i_nSrcImageSizeX, long i_nSrcImageSizeY, Ipvm::Image8u3 i_ColorImage); // Host로 보낼 이미지 Split
    void SaveEachNGRVImage(std::vector<Ipvm::Image8u3> i_vecGrabbedImage, NgrvGrabbedImageInfo GrabbedImageInfo,
        CString i_strInspItemName, CString i_strReviewSideName, const BOOL& isNGRV_Single_Run,
        const std::vector<long>& vecSingleRunUsingFrameID); // NGRV Image Save
};
