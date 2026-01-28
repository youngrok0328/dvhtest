#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedComponent/Label/NewLabel.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Gadget/SocketMessaging.h>
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;
class CDlgReviewImageViewer;
class VisionUnit;
class VisionDeviceResult;
class CDlgMarkTeachTotalResultViewer; //kircheis_201611XX //TeachResultView
class VisionImageLot;
class SyncController;
class SurfaceSpec;
class ImageViewEx;
class ImageLotView;
class VisionCommonSpecDlg;
class VisionCommonResultDlg;
class VisionCommonDetailResultDlg;
class VisionCommonTextResultDlg;
class VisionCommonLogDlg;
class VisionCommonDebugInfoDlg;
class VisionMainAgent;

//HDR_6_________________________________ Header body
//
enum TimerDef
{
    Timer_Send_Host_Live_Image,
    Timer_Check_Installer_Monitor,
    Timer_iGrab_Board_Temperature_Check,
};

enum PixelCalUI_Tab
{
    Tab_Pixel_Calibration_Start = 0,
    Tab_Pixel_Calibration_2D_PixelCalibration = Tab_Pixel_Calibration_Start,
    Tab_Pixel_Calibration_2D_ManualAutoFocusing,
    Tab_Pixel_Calibration_END,
};

class CDlgVisionPrimaryUI : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionPrimaryUI)

public:
    CDlgVisionPrimaryUI(
        VisionMainAgent& visionMainAgent, VisionUnit& visionUnit, CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgVisionPrimaryUI();

    VisionMainAgent& m_visionMainAgent;
    VisionUnit& m_visionUnit;

    void SetLiveMain();
    void ResetLiveMain();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG_PRIMARY
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnBnClickedButtonInspect();
    afx_msg LRESULT OnGrabEndEvent(WPARAM wparam, LPARAM lparam);

    // 함수 정의

public:
    void OnJobChanged();

    BOOL ImageFileOpen();

    void OnMessageSocketConnectionUpdated(const Ipvm::SocketMessaging::EVT_INFO& evtInfo);
    void OnMessageSocketDataReceived(const Ipvm::SocketMessaging::MSG_INFO& msgInfo);

    void ShowCurrentImage(bool showRaw);

    // 변수 정의

public:
    ImageLotView* m_imageLotView;
    ImageLotView* m_imageLotViewRearSide; // Rear Side UI Divide 2024.03.29
    VisionCommonSpecDlg* m_visionCommonSpecDlg;
    VisionCommonResultDlg* m_visionCommonResultDlg;
    VisionCommonDetailResultDlg* m_visionCommonDetailResultDlg;
    VisionCommonDebugInfoDlg* m_visionCommonDebugInfoDlg;
    VisionCommonTextResultDlg* m_visionCommonTextResultDlg;
    VisionCommonLogDlg* m_visionCommonLogDlg;
    //Rear
    VisionCommonResultDlg* m_visionCommonResultDlgRear;
    VisionCommonDetailResultDlg* m_visionCommonDetailResultDlgRear;
    VisionCommonDebugInfoDlg* m_visionCommonDebugInfoDlgRear;
    VisionCommonTextResultDlg* m_visionCommonTextResultDlgRear;

public:
    CTabCtrl m_tabResult;

    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnTcnSelchangeTabResult(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnAddLogWarning(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnShowDetailResult(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnImageLotViewPaneSelChanged(WPARAM wparam, LPARAM lparam);

    CNewLabel m_Label_JobFile;
    CNewLabel m_Label_JobFileName;
    CNewLabel m_Label_Connect1;
    CNewLabel m_Label_Connect2;
    CNewLabel m_Label_AccessMode1;
    CNewLabel m_Label_AccessMode2;
    CNewLabel m_Label_Version1;
    CNewLabel m_Label_Version2;
    CNewLabel m_LabelResult;
    CNewLabel m_Label_Side_Front_Status;
    CNewLabel m_Label_Side_Rear_Status;
    CString m_strJobName;
    CString m_strVersion;

    CString m_str3DGrabTime;

    BOOL m_bMarkTeachFlag;
    bool m_bJobTeachEnable;

    long MarkAutoTeaching();

    // 영훈 20151214 : Mark Skip Button을 맨 처음 Mark Teaching때만 사용할 수 있도록 해준다.
    BOOL m_bMarkSkipMode;

    CDlgReviewImageViewer* m_pReviewImageViewer;
    void ReviewImageViewer();
    void SystemSetup();

    void SetTimerFlag(long nFlag, long nTime);

    //{{ kircheis_AutoTeach
    BOOL AutoMarkInspSeq();
    BOOL AutoMarkTeachSeq();
    BOOL InspectSingleVisionProcess(long nID);
    //}}

    long GetValidPaneID(); //kircheis_201611XX // 현 FOV내 Package가 있는 첫 Pane ID를 리턴

    //{{//kircheis_201611XX //TeachResultView
    void SetMarkTeachTotalResultView(const Ipvm::Image8u& image, const Ipvm::Rect32s& i_rtViewArea, long i_nMarkCharNum,
        Ipvm::Rect32s* i_prtEachCharROI);
    void HideMarkTeachTotalResultView();
    CDlgMarkTeachTotalResultViewer* m_pDlgMarkTeachTotalResultView;
    //}}

    ImageViewEx* m_imageView;
    void DrawCenterCrossLine(BOOL bIDOverlayCenterLine);

    void CloseProfileView(); //SDY ProfileView를 닫기 위한 함수

    // General/Side Vision Main UI 분리 - JHB_2024.04.15
    void SetGeneralVisionMainUI();
    void SetSideVisionMainUI();
    void SetPrimaryUISideVisionSection();

private:
    void checkInstallerMonitorProgram();
    void check_iGrab_Board_Temperature_Check();

    long m_nSideVisionSection; // Side Vision Section Check - JHB_2024.04.15

    long m_nCurCommonTab;
    void UpdateCommonTabShow();

    void OnBnClickedButtonInspectSide();

public:
    afx_msg void OnStnClickedStaticSideFrontPrimary();
    afx_msg void OnStnClickedStaticSideRearPrimary();
};