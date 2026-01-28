#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "CallBackList_InspectionEnd.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <memory>

//HDR_5_________________________________ Forward declarations
class ImageClassifier;
class ImageClassifier_Buffer;
class CVisionInlineUI;
class VisionMainTR;
class VisionTrayScanSpec;
class VisionUnit;
class FOVID;

//HDR_6_________________________________ Header body
//
class InspectionOrderer
{
public:
    InspectionOrderer(VisionMainTR* main, ImageClassifier* imageClassifier, VisionUnit** inlineUnits, long threadNum);
    ~InspectionOrderer();

    VisionUnit& getVisionUnit(long visionUnitIndex);
    bool isAllInspectionThreadsIdle();
    void setInlineUI(long visionUnitIndex, CVisionInlineUI& ui, HWND parent);
    void update(const VisionTrayScanSpec& trayScanSpec, const long InspectionRepeatIndex);
    void updateForSide(const VisionTrayScanSpec& trayScanSpec, const long InspectionRepeatIndex, const FOVID& fovID);
    bool copyFromLastestCalculationImage(VisionImageLot& imageLot);

    void registerCallBack_InspectionEnd(void* userData, VisionMainAgent::FUNC_INSPECTION_END_CALLBACK* callback);
    void unRegisterCallBack_InspectionEnd(VisionMainAgent::FUNC_INSPECTION_END_CALLBACK* callback);

private:
    VisionMainTR* m_main;
    ImageClassifier* m_imageClassifier;
    long m_threadNum;
    long m_nInspectionRepeatIndex;

    struct SThreadControl
    {
        std::shared_ptr<ImageClassifier_Buffer> m_imageBuffer;
        long m_batchKey;
        CString m_batchBarcode;
        long m_paneIndex;
        VisionUnit* m_visionUnit;
        CVisionInlineUI* m_inlineUI;

        HANDLE m_Signal_KillThread; // 인라인 검사 쓰레드를 종료하는 명령을 내림
        HANDLE m_Signal_ThreadStarted; // 쓰레드가 시작되었다는 이벤트
        HANDLE m_Signal_Ready; // 검사가 준비되었음을 나타냄
        HANDLE m_Signal_StartInspection; // 인라인 상태에서 검사 명령을 내림
        HANDLE m_Signal_Idle; // Idle 상태임을 나타내는 플래그
        HANDLE m_Handle_Thread; // 쓰레드의 핸들을 가지고 있는다. 종료 대기시 사용.
    };

    CCriticalSection m_csUpdate;
    SThreadControl* m_items;
    CallBackList_InspectionEnd* m_callback_inspectionEnd;

    // 가장 마지막에 영상을 Grab 한 검사 Thread의 Index, Inline 종료후 Primary UI로 복사해준다
    long m_lastestGrabBufferIndex;

    static UINT ThreadInspectionFunc(LPVOID pParam);
    UINT ThreadInspection();

    long getIdleControl();
};
