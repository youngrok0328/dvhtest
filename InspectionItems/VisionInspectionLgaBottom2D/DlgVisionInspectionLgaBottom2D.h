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
#include <vector>

//HDR_5_________________________________ Forward declarations
class ImageLotView;
class ProcCommonSpecDlg;
class ProcCommonResultDlg;
class ProcCommonDetailResultDlg;
class ProcCommonLogDlg;
class ProcCommonDebugInfoDlg;
class VisionInspectionLgaBottom2D;
class VisionInspectionLgaBottom2DPara;
struct ProcessingDlgInfo;

//HDR_6_________________________________ Header body
//
class CDlgVisionInspectionLgaBottom2D : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionInspectionLgaBottom2D)

public:
    CDlgVisionInspectionLgaBottom2D(
        const ProcessingDlgInfo& procDlgInfo, VisionInspectionLgaBottom2D* pVisionInsp, CWnd* pParent = NULL);
    virtual ~CDlgVisionInspectionLgaBottom2D();

    enum
    {
        IDD = IDD_DIALOG
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()

public:
    const ProcessingDlgInfo& m_procDlgInfo;
    ImageLotView* m_imageLotView;
    ProcCommonSpecDlg* m_procCommonSpecDlg;
    ProcCommonResultDlg* m_procCommonResultDlg;
    ProcCommonDetailResultDlg* m_procCommonDetailResultDlg;
    ProcCommonLogDlg* m_procCommonLogDlg;
    ProcCommonDebugInfoDlg* m_procCommonDebugInfoDlg;

    VisionInspectionLgaBottom2D* m_pVisionInsp;
    VisionInspectionLgaBottom2DPara* m_pVisionPara;

    CXTPPropertyGrid* m_propertyGrid;

    BOOL m_bShowResultDetail;
    long m_nImageID;
    long m_nSelectGroupID;

public:
    void ShowPrevImage();
    void ShowNextImage();
    void ShowImage(BOOL bChange = FALSE);

    void UpdatePropertyGrid();
    void ResetSpecAndResultDlg();
    BOOL MakeSpecROI(long i_nGroupID, std::vector<PI_RECT>& o_vecpirtSpecLand);

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
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/);

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
};
