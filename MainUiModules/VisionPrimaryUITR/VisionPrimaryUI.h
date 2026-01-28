#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Gadget/SocketMessaging.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;
class VisionUnit;
class CDlgVisionPrimaryUI;
class CDlgReviewImageViewer;
class CSyncContrllerSlit;
class VisionInspectionSpec;
class VisionDeviceResult;
class VisionInspectionResult;
class VisionDebugInfo;
class VisionMainAgent;
class VisionImageLot;
class ImageViewEx;

//HDR_6_________________________________ Header body
//
class __VISION_PRIMARY_UI_CLASS__ CVisionPrimaryUI
{
public:
    CVisionPrimaryUI(void) = delete;
    CVisionPrimaryUI(VisionMainAgent& visionMainAgent, VisionUnit& visionUnit);
    ~CVisionPrimaryUI(void);

public:
    void OnMessageSocketConnectionUpdated(const Ipvm::SocketMessaging::EVT_INFO& evtInfo);
    void OnMessageSocketDataReceived(const Ipvm::SocketMessaging::MSG_INFO& msgInfo);

    void ShowVisionPrimaryDlg(HWND hwndParent);
    void HideVisionPrimaryDlg();
    bool IsWindowVisible();
    bool IsJobTeachEnabled();

    void OnLogin();

    void SetMarkTeachFlag(BOOL bMarkTeachFlag);
    BOOL GetMarkTeachFlag();

    void SetLiveMain();
    void ResetLiveMain();

    void OnJobChanged();

    BOOL OnFileImageOpen();

    void OnInspect();
    void ReviewImageViewer();
    void SystemSetup();

    void ShowCurrentImage(bool showRaw);

    VisionMainAgent& m_visionMainAgent;
    VisionUnit& m_visionUnit;
    CDlgReviewImageViewer* m_pReviewImageViewer;

    // 영훈 20150720 : pDlg는 이제 밖에서 사용하지 못한다.

public:
    HWND Dlg_GetsafeHwnd();
    BOOL Dlg_GetMarkSipMode();
    void Dlg_SetMarkSipMode(BOOL bData);

    //{{//kircheis_201611XX //TeachResultView
    void SetMarkTeachTotalResultView(const Ipvm::Image8u& image, const Ipvm::Rect32s& i_rtViewArea, long i_nMarkCharNum,
        Ipvm::Rect32s* i_prtEachCharROI);
    void HideMarkTeachTotalResultView();
    //}}

    void SetPrimaryUISideVisionSection(); // Side Vision 전환시 PrimaryUI 갱신을 위한 함수 - JHB_2024.04.15

public:
    CDlgVisionPrimaryUI* m_pDlg;
};
