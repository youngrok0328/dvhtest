#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedComponent/Label/NewLabel.h"

//HDR_3_________________________________ External library headers
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class ImageLotView;
class VisionInspectionSurface;
class VisionInspectionSurfacePara;
class ProcCommonSpecDlg;
class ProcCommonResultDlg;
class ProcCommonDetailResultDlg;
class ProcCommonLogDlg;
class ProcCommonDebugInfoDlg;
class VisionProcessing;
struct ProcessingDlgInfo;

//HDR_6_________________________________ Header body
//
class CDlgVisionInspectionSurface : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionInspectionSurface)

public:
    CDlgVisionInspectionSurface(const ProcessingDlgInfo& procDlgInfo, VisionInspectionSurface* pVisionInsp,
        CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgVisionInspectionSurface();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    const ProcessingDlgInfo& m_procDlgInfo;
    ImageLotView* m_imageLotView;
    ImageLotView* m_imageLotViewRearSide;
    CXTPPropertyGrid* m_propertyGrid;
    ProcCommonSpecDlg* m_procCommonSpecDlg;
    ProcCommonResultDlg* m_procCommonResultDlg;
    ProcCommonDetailResultDlg* m_procCommonDetailResultDlg;
    ProcCommonLogDlg* m_procCommonLogDlg;
    ProcCommonDebugInfoDlg* m_procCommonDebugInfoDlg;
    ProcCommonResultDlg* m_procCommonResultDlgRear;
    ProcCommonDetailResultDlg* m_procCommonDetailResultDlgRear;
    ProcCommonLogDlg* m_procCommonLogDlgRear;
    ProcCommonDebugInfoDlg* m_procCommonDebugInfoDlgRear;
    VisionInspectionSurface* m_pVisionInsp;
    VisionInspectionSurfacePara* m_pVisionPara;

    BOOL m_bShowResultDetail;
    long m_nImageID;
    long m_nUsed3DImageType; // 0: ZMap   1: VMap

public:
    void SetInitDialog();
    void SetInitSetupWindow();
    void UpdatePropertyGrid();
    void ShowImage(BOOL bChange = FALSE);

public:
    CTabCtrl m_TabResult;

    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};

    afx_msg void OnDestroy();
    afx_msg void OnTcnSelchangeTabResult(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnShowDetailResult(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg LRESULT OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/);

    void OnBnClickedButtonInspect();
    void ConvertUm2Px(long i_nSelectIndex);
    void ConvertPx2Um(long i_nSelectIndex);

private:
    //{{View Um to Px
    double m_dMinAreaBrightContrast_Px;
    double m_dMinAreaDarkContrast_Px;
    double m_dMergeDistance_Px;
    double m_dfLocalContrastAreaSpan_Px;
    double m_dbackground_window_size_x_Px;
    double m_dbackground_window_size_y_Px;
    double m_dMinimumBrightBlobArea_Px;
    double m_dMininumDarkBlobArea_Px;
    //}}

    long m_nCurCommonTab;
    void UpdateCommonTabShow();

public:
    CNewLabel m_Label_Side_Front_Status;
    CNewLabel m_Label_Side_Rear_Status;

    void SetInitialSideVisionSelector();

    afx_msg void OnStnClickedStaticSideFrontSurface();
    afx_msg void OnStnClickedStaticSideRearSurface();
};
