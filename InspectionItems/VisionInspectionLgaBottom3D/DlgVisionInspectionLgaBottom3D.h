#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
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
class VisionInspectionLgaBottom3D;
class VisionInspectionLgaBottom3DPara;
class VisionInspectionSpec;
class CDlg3DView;
struct ProcessingDlgInfo;
struct PI_RECT;

//HDR_6_________________________________ Header body
//
class CDlgVisionInspectionLgaBottom3D : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionInspectionLgaBottom3D)

public:
    CDlgVisionInspectionLgaBottom3D(const ProcessingDlgInfo& procDlgInfo, VisionInspectionLgaBottom3D* pVisionInsp,
        CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgVisionInspectionLgaBottom3D();

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

    VisionInspectionLgaBottom3D* m_pVisionInsp;
    VisionInspectionLgaBottom3DPara* m_pVisionPara;
    std::vector<VisionInspectionSpec>* m_pvecSpec;

    CDlg3DView* m_p3DViewer;
    CXTPPropertyGrid* m_propertyGrid;
    long m_nCmbSelImage;
    CXTPPropertyGridItem* m_LUCPCategory;

    BOOL m_bShowResultDetail;
    long m_nImageID;
    long m_nSelectGroupID; //kk Group View 용 변수

public:
    void SetInitButton();
    void ShowImage();

    void UpdatePropertyGrid();
    afx_msg void OnBnClickedBtnOpen3dViewer();
    void ResetSpecAndResultDlg();
    void btnClickInspectionAdd();
    void btnClickInspectionDel(long Targetindex);
    void btnClickInspectionSetup(long Targetindex);
    void UpdateLUCPGroup();
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
    afx_msg void OnCbnSelchangeComboCurrpane();

public:
    CTabCtrl m_TabResult;
    CTabCtrl m_TabLog;
    CButton m_buttonApply;
    CButton m_buttonInspect;
    CButton m_buttonClose;
    CComboBox m_ComboCurrPane;
};
