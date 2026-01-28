#pragma once
//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "SimpleRibbonBar.h"
#include "ViewVisionBody.h"

//HDR_2_________________________________ Other projects' headers
#include "../ManagementModules/VisionMainTR/VisionMainTR.h"
#include "../SharedComponent/Label/Label.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <memory>

//HDR_5_________________________________ Forward declarations
class DialogITRV;
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
    UM_ACCESS_MODE_CHANGED,
    UM_TUNINGSOCKET_CONNECTION_EVENT,
    UM_TUNINGSOCKET_DATA_RECEIVED,
    UM_VISION_TUNING_LIVE_2D,
    UM_VISION_TUNING_LIVE_3D,

    UM_AUTOMATION_SOCKET_CONNECTION_EVENT,
    UM_AUTOMATION_SOCKET_DATA_RECEIVED,
};

class ViewVision : public CDialogEx
{
    DECLARE_DYNAMIC(ViewVision)

public:
    ViewVision(DialogITRV* parent, int32_t visionIndex, HANDLE flag_InlineStart, HANDLE flag_SnycStart);
    virtual ~ViewVision();

    bool IsShowProcessingTeachingUI();
    void ShowAccessModeDialog();
    void ShowIlluminationTeachUI();
    void ShowPrimaryUI();
    void ShowTeach();
    void ShowBatchInspection();
    void UpdateRibbonMenuState();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_VIEW_VISION
    };
#endif
    const int32_t m_visionIndex;
    VisionMainTR m_visionMain;

    // 영훈 20140122 : Job Save시 Detail Setup Mode일 경우 밖으로 나와서 하도록 유도한다.
    BOOL m_bNeedToExitOfDetailSetup;

    void SetCurrentFrameString();
    void InitializeMessageSocket();

protected:
    DialogITRV* m_parent;
    CLabel m_title;
    SimpleRibbonBar m_wndRibbonBar;
    std::shared_ptr<ViewVisionBody> m_wndBody;

    SlitBeam3DParameters* m_tuningSlitBeamParameters;

    BOOL m_bIsUpdateRecipeDone = FALSE; // Recipe Update가 완료되었는지를 판단하는 변수

    void UpdateLayout();
    void OnClickedMenuButton(int32_t buttonId);
    void OnJobBatchinspection();
    void OnJobIllumsetup();
    void OnInspectionTeach();
    void OnInspectionQuit();
    void OnMarkteachDone();
    void OnJobInlinemode();

    void OnUpdateMenuButton(int32_t buttonId, bool& enable);
    void OnUpdateJobInlinemode(bool& enable);
    void OnUpdateJobJobopen(bool& enable);
    void OnUpdateJobJobsave(bool& enable);
    void OnUpdateJobJobsaveas(bool& enable);
    void OnUpdateImageImageopen(bool& enable);
    void OnUpdateImageImagesave(bool& enable);
    void OnUpdateImageGrab(bool& enable);
    void OnUpdateImageLive(bool& enable);
    void OnUpdateJobBatchinspection(bool& enable);
    void OnUpdateJobIllumOrInspItemSetup(bool& enable);
    void OnUpdateInspectionRun(bool& enable);
    void OnUpdateInspectionTeach(bool& enable);
    void OnUpdateInspectionQuit(bool& enable);
    void OnUpdateMarkteachSkip(bool& enable);
    void OnUpdateMarkteachDone(bool& enable);
    void OnUpdateJobSystemSetup(bool& enable);
    void OnUpdateJobIocheck(bool& enable);
    void OnUpdateJobOverlayManager(bool& enable);

    void OnOK() override;
    void OnCancel() override;
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.
    virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG* pMsg);

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);

    DECLARE_MESSAGE_MAP()

public:
    afx_msg LRESULT OnShowTeach(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnShowMain(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnDataReceivedFromHost(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnConnectionUpdatedFromHost(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnAccessModeChanged(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnTuningSocketDataReceivedFromHost(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnTuningSocketConnectionUpdatedFromHost(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnTuningLive2DEvent(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnTuningLive3DEvent(WPARAM wparam, LPARAM lparam);
    // iPack과 통신 메시지 Callback 함수
    afx_msg LRESULT OnMsgSendFinalREsultDataVsToMv(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnMsgSendDataCheck(WPARAM wParam, LPARAM lParam);

    // Automation과 통신 함수
    afx_msg LRESULT OnDataReceivedFromAutomation(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnConnectionUpdatedFromAutomation(WPARAM wParam, LPARAM lParam);

    
};
