#pragma once
//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/PI_RECT.h"

//HDR_3_________________________________ External library headers
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class ImageLotView;
class ProcCommonSpecDlg;
class ProcCommonResultDlg;
class ProcCommonDetailResultDlg;
class ProcCommonLogDlg;
class ProcCommonDebugInfoDlg;
struct ProcessingDlgInfo;
class VisionInspectionDie2D;
class VisionInspectionDie2DPara;

//HDR_6_________________________________ Header body
//
class CDlgVisionInspectionDie2D : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionInspectionDie2D)

public:
    CDlgVisionInspectionDie2D(const ProcessingDlgInfo& procDlgInfo, VisionInspectionDie2D* pVisionInsp,
        CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgVisionInspectionDie2D();

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
    ProcCommonSpecDlg* m_procCommonSpecDlg;
    ProcCommonResultDlg* m_procCommonResultDlg;
    ProcCommonDetailResultDlg* m_procCommonDetailResultDlg;
    ProcCommonLogDlg* m_procCommonLogDlg;
    ProcCommonDebugInfoDlg* m_procCommonDebugInfoDlg;
    CXTPPropertyGrid* m_propertyGrid;

    VisionInspectionDie2D* m_pVisionInsp;
    VisionInspectionDie2DPara* m_pVisionPara;

    BOOL m_bShowResultDetail;
    long m_nImageID;
    long m_nBarcodeImage;

public:
    void SetInitDialog();
    void SetInitParaWindow();
    void SetInitButton();
    void ShowPrevImage();
    void ShowNextImage();
    void ShowImage(BOOL bChange = FALSE);
    BOOL ShowCharBinImage();

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnClose();
    afx_msg void OnBnClickedButtonClose();
    afx_msg void OnBnClickedButtonInspect();
    afx_msg void OnTcnSelchangeTabResult(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnShowDetailResult(WPARAM wparam, LPARAM lparam);

public:
    CTabCtrl m_TabResult;
    CTabCtrl m_TabLog;
    CButton m_buttonApply;
    CButton m_buttonInspect;
    CButton m_buttonClose;
    afx_msg void OnCbnSelchangeComboCurrpane();
    afx_msg void OnBnClickedButtonPreimage();
    afx_msg void OnBnClickedButtonNextimage();
    CComboBox m_ComboCurrPane;
    afx_msg LRESULT OnRoiChanged(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/);

    void GetROI();
    void SetROI();
    void DrawEdgeSearchROI();

    void ClickedButtonImageCombine();
    void UpdatePropertyGrid();
    LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
};
