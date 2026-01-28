#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "SimpleRibbonBar.h"

//HDR_2_________________________________ Other projects' headers
#include "../ManagementModules/VisionMain/VisionMain.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class SlitBeam3DParameters;

//HDR_6_________________________________ Header body
//
enum
{
    UM_SOCKET_CONNECTION_EVENT = (WM_USER + 1010),
    UM_SOCKET_DATA_RECEIVED,
    UM_TUNING_SOCKET_CONNECTION_EVENT,
    UM_TUNING_SOCKET_DATA_RECEIVED,
    UM_SHOW_TEACH,
    UM_SHOW_MAIN,
    UM_TUNINGSOCKET_CONNECTION_EVENT,
    UM_TUNINGSOCKET_DATA_RECEIVED,
    UM_VISION_TUNING_LIVE_2D,
    UM_VISION_TUNING_LIVE_3D,

    UM_AUTOMATION_SOCKET_CONNECTION_EVENT,
    UM_AUTOMATION_SOCKET_DATA_RECEIVED,
};

class CMainFrame : public CFrameWndEx
{
protected: // serialization에서만 만들어집니다.
    CMainFrame();
    DECLARE_DYNCREATE(CMainFrame)

    // 특성입니다.

public:
    CVisionMain m_visionMain;

    // 영훈 20140122 : Job Save시 Detail Setup Mode일 경우 밖으로 나와서 하도록 유도한다.
    BOOL m_bNeedToExitOfDetailSetup;

public:
    void SetCurrentFrameString();
    void InitializeMessageSocket();

    // 재정의입니다.

public:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

    // 구현입니다.

public:
    virtual ~CMainFrame();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected: // 컨트롤 모음이 포함된 멤버입니다.
    SimpleRibbonBar m_wndRibbonBar;
    SlitBeam3DParameters* m_tuningSlitBeamParameters;

    // 생성된 메시지 맵 함수

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnJobNewjob();
    afx_msg void OnJobJobopen();
    afx_msg void OnJobJobsave();
    afx_msg void OnImageImageopen();
    afx_msg void OnImageImagesave();
    afx_msg void OnImageGrab();
    afx_msg void OnImageLive();
    afx_msg void OnJobBatchinspection();
    afx_msg void OnJobInspectionitemsetup();
    afx_msg void OnJobIllumsetup();
    afx_msg void OnInspectionRun();
    afx_msg void OnInspectionTeach();
    afx_msg void OnInspectionQuit();
    afx_msg void OnMarkteachSkip();
    afx_msg void OnMarkteachDone();
    afx_msg void OnHostGotohost();
    afx_msg void OnJobInlinemode();
    afx_msg void OnJobLogin();
    afx_msg void OnUpdateJobNewjob(CCmdUI* pCmdUI);
    afx_msg void OnUpdateJobJobopen(CCmdUI* pCmdUI);
    afx_msg void OnUpdateJobJobsave(CCmdUI* pCmdUI);
    afx_msg void OnUpdateJobJobsaveas(CCmdUI* pCmdUI);
    afx_msg void OnUpdateImageImageopen(CCmdUI* pCmdUI);
    afx_msg void OnUpdateImageImagesave(CCmdUI* pCmdUI);
    afx_msg void OnUpdateImageGrab(CCmdUI* pCmdUI);
    afx_msg void OnUpdateImageLive(CCmdUI* pCmdUI);
    afx_msg void OnUpdateJobBatchinspection(CCmdUI* pCmdUI);
    afx_msg void OnUpdateJobInspectionitemsetup(CCmdUI* pCmdUI);
    afx_msg void OnUpdateJobIllumsetup(CCmdUI* pCmdUI);
    afx_msg void OnUpdateInspectionRun(CCmdUI* pCmdUI);
    afx_msg void OnUpdateInspectionTeach(CCmdUI* pCmdUI);
    afx_msg void OnUpdateInspectionQuit(CCmdUI* pCmdUI);
    afx_msg void OnUpdateMarkteachSkip(CCmdUI* pCmdUI);
    afx_msg void OnUpdateMarkteachDone(CCmdUI* pCmdUI);
    afx_msg void OnUpdateHostGotohost(CCmdUI* pCmdUI);
    afx_msg void OnUpdateJobInlinemode(CCmdUI* pCmdUI);
    afx_msg void OnUpdateJobLogin(CCmdUI* pCmdUI);
    afx_msg void OnJobSystemSetup();
    afx_msg void OnUpdateJobSystemSetup(CCmdUI* pCmdUI);
    afx_msg void OnJobIocheck();
    afx_msg void OnUpdateJobIocheck(CCmdUI* pCmdUI);
    afx_msg void OnJobReviewimageviewer();
    afx_msg void OnUpdateJobReviewimageviewer(CCmdUI* pCmdUI);
    afx_msg void OnJobOverlayManager();
    afx_msg void OnUpdateJobOverlayManager(CCmdUI* pCmdUI);
    afx_msg void OnJobJobsaveas();

    afx_msg LRESULT OnShowTeach(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnShowMain(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnDataReceivedFromHost(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnConnectionUpdatedFromHost(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnTuningSocketDataReceivedFromHost(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnTuningSocketConnectionUpdatedFromHost(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnTuningLive2DEvent(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnTuningLive3DEvent(WPARAM wparam, LPARAM lparam);
    afx_msg void OnButtonRunIdl();
    afx_msg void OnUpdateButtonRunIdl(CCmdUI* pCmdUI);
    // iPack과 통신 메시지 Callback 함수
    afx_msg LRESULT OnMsgSendFinalREsultDataVsToMv(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnMsgSendDataCheck(WPARAM wParam, LPARAM lParam);

    // Automation과 통신 함수
    afx_msg LRESULT OnDataReceivedFromAutomation(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnConnectionUpdatedFromAutomation(WPARAM wParam, LPARAM lParam);
};
