#pragma once
//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../SharedComponent/Label/NewLabel.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"

//HDR_3_________________________________ External library headers
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionInspectionGeometryPara;
class ImageLotView;
class ProcCommonSpecDlg;
class ProcCommonResultDlg;
class ProcCommonDetailResultDlg;
class ProcCommonLogDlg;
class ProcCommonDebugInfoDlg;
class VisionInspectionGeometry;
class VisionInspectionSpec;
struct ProcessingDlgInfo;

//HDR_6_________________________________ Header body
//
class CDlgVisionInspectionGeometry : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionInspectionGeometry)

public:
    CDlgVisionInspectionGeometry(
        const ProcessingDlgInfo& procDlgInfo, VisionInspectionGeometry* pVisionInsp, CWnd* pParent = NULL);
    virtual ~CDlgVisionInspectionGeometry();

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
    ImageLotView* m_imageLotViewRearSide;
    ProcCommonSpecDlg* m_procCommonSpecDlg;
    ProcCommonResultDlg* m_procCommonResultDlg;
    ProcCommonResultDlg* m_procCommonResultDlgRear;
    ProcCommonDetailResultDlg* m_procCommonDetailResultDlg;
    ProcCommonDetailResultDlg* m_procCommonDetailResultDlgRear;
    ProcCommonLogDlg* m_procCommonLogDlg;
    ProcCommonLogDlg* m_procCommonLogDlgRear;
    ProcCommonDebugInfoDlg* m_procCommonDebugInfoDlg;
    ProcCommonDebugInfoDlg* m_procCommonDebugInfoDlgRear;

    VisionInspectionGeometry* m_pVisionInsp;
    VisionInspectionGeometryPara* m_pVisionPara;
    std::vector<VisionInspectionSpec>* m_pvecSpec;

    CXTPPropertyGrid* m_propertyGrid;
    CXTPGridControl m_ParameterList;

    BOOL m_bShowResultDetail;
    long m_nImageID;

public:
    void ResetListControl();
    void UpdatePropertyGrid();

    afx_msg void OnItemButtonClick(NMHDR* pNotifyStruct, LRESULT*);
    afx_msg void OnReportValueChanged(NMHDR* pNotifyStruct, LRESULT* /*result*/);
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
    void btnClickImageCombine();
    void btnClickInspectionAdd();
    BOOL CheckData(long nDataNum);

    void SetInitButton();
    void ShowImage(BOOL bChange = FALSE);
    void ResetSpecAndResultDlg();

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
    afx_msg void OnStnClickedStaticSideFrontGeometry();
    afx_msg void OnStnClickedStaticSideRearGeometry();

public:
    CTabCtrl m_TabResult;
    CTabCtrl m_TabLog;
    CButton m_buttonApply;
    CButton m_buttonInspect;
    CButton m_buttonClose;
    afx_msg void OnCbnSelchangeComboCurrpane();
    CComboBox m_ComboCurrPane;
    afx_msg LRESULT OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/);

private:
    long m_nCurCommonTab;
    void UpdateCommonTabShow();

    CNewLabel m_Label_Side_Front_Status;
    CNewLabel m_Label_Side_Rear_Status;

private:
    void SetInitialSideVisionSelector();
    void SetSideUI(const CRect& DataArea);
    void SetNonSideUI(const CRect& DataArea);
    void SetChangeSideUI(const long& SelectOnlySIdeType);
};
