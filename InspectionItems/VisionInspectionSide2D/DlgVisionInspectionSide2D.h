#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../SharedComponent/Label/NewLabel.h"

//HDR_3_________________________________ External library headers
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class ImageLotView;
class VisionInspectionSide2D;
class VisionInspectionSide2DPara;
class ProcCommonSpecDlg;
class ProcCommonResultDlg;
class ProcCommonDetailResultDlg;
class ProcCommonLogDlg;
class ProcCommonDebugInfoDlg;
struct ProcessingDlgInfo;

//HDR_6_________________________________ Header body
//
class CDlgVisionInspectionSide2D : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionInspectionSide2D)

public:
    CDlgVisionInspectionSide2D(const ProcessingDlgInfo& procDlgInfo, VisionInspectionSide2D* pVisionInsp,
        CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgVisionInspectionSide2D();

    void OnBnClickedButtonInspect();

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
    VisionInspectionSide2D* m_pVisionInsp;
    VisionInspectionSide2DPara* m_pVisionPara;
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

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};
    afx_msg void OnDestroy();
    afx_msg void OnClose();
    afx_msg void OnTcnSelchangeTabResult(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnShowDetailResult(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/);

    void UpdatePropertyGrid();

public:
    CTabCtrl m_TabResult;

    // Side Selector - 2024.05.29_JHB
    CNewLabel m_Label_Side_Front_Status;
    CNewLabel m_Label_Side_Rear_Status;

    void SetInitialSideVisionSelector();

    afx_msg void OnStnClickedStaticSideFrontSide2d();
    afx_msg void OnStnClickedStaticSideRearSide2d();
    ////////////////////////////////////////////

private:
    long m_nCurCommonTab;
    void UpdateCommonTabShow();
};
