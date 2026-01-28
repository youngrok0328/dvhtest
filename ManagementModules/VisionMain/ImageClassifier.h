#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <map>
#include <memory>
#include <vector>

//HDR_5_________________________________ Forward declarations
class FOVID;
class FOVImageGrabber;
class InspectionAreaInfo;
class VisionImageLot;
class VisionTrayScanSpec;
class CVisionMain; // side deviceresult

//HDR_6_________________________________ Header body
//
class ImageClassifier_Info
{
public:
    ImageClassifier_Info(LPCTSTR trayKey, const std::vector<long>& needStitchList, long runtimeIndex, long trayIndex,
        long inspectionID, long paneCount);

    std::map<long, BOOL> m_needStitchList;
    CString m_trayKey;

    long m_runtimeIndex;
    long m_trayIndex;
    long m_inspectionID; // 검사 ID
    long m_paneCount; // Pane Count
};

class ImageClassifier_Buffer
{
public:
    ImageClassifier_Buffer();
    ~ImageClassifier_Buffer();

    void initialize(LPCTSTR trayKey, const std::map<long, BOOL>& needStitchList, const long paneCount);
    bool initialize(LPCTSTR filePath, long paneIndex, long batchKey = -1, LPCTSTR batchBarcode = _T(""));

    bool needFOV(const InspectionAreaInfo& inspAreaInfo, const FOVID& fovID);
    void eraseNeedStitchList(long stitchIndex);
    bool isCalculationReady();
    bool isBufferCalculating();
    long popCalculationPaneIndex(bool& o_isLastPaneIndex);
    void calculatationCompleted(long panelIndex);
    void setIdle();
    bool isIdle();

    long getBatchKey();
    CString getBatchBarcode();

    std::shared_ptr<VisionImageLot> m_imageLot;

private:
    CCriticalSection m_csBuffer;
    CString m_trayKey;
    std::map<long, BOOL> m_needStitchList;
    long m_batchKey;
    CString m_batchBarcode;

    std::vector<long> m_paneList_ready;
    std::vector<long> m_paneList_finished;
    long m_paneCount;
};

class ImageClassifier
{
public:
    ImageClassifier(CVisionMain* main, FOVImageGrabber* fovImageGrabber, long bufferCount);
    ~ImageClassifier();

    void removeIncompleteImages(const VisionTrayScanSpec& trayScanSpec);
    void setAllBuffersToIdle();
    void getValidBufferList(std::vector<long>& o_validList);
    void addCandidateDevice(std::shared_ptr<ImageClassifier_Info> candidate);
    bool update(const VisionTrayScanSpec& trayScanSpec);
    bool updateForSide(const VisionTrayScanSpec& trayScanSpec, const FOVID& fovID);
    bool isAllIdle();
    bool isAllIdleExceptCandidate();

    std::shared_ptr<ImageClassifier_Buffer> getBuffer(long bufferIndex);
    std::shared_ptr<ImageClassifier_Buffer> getIdleBuffer();

private:
    FOVImageGrabber* m_fovImageGrabber;
    CVisionMain* m_main;

    CCriticalSection m_csLock;
    std::vector<std::shared_ptr<ImageClassifier_Info>> m_candidates;
    std::vector<std::shared_ptr<ImageClassifier_Buffer>> m_buffers;

    void candiate_to_buffer(const VisionTrayScanSpec& trayScanSpec, const std::vector<long>& validFovBufferIndex);
    void removeIncompleteBufferImages();
    void removeIncompleteCandidates(const VisionTrayScanSpec& trayScanSpec);
    void removeGarbageFovImageBuffer(const VisionTrayScanSpec& trayScanSpec);
};
