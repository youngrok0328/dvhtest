#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"

//HDR_3_________________________________ External library headers
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class ImageLotView;
class VisionInspectionComponent2D;
class VisionInspectionComponent2DPara;
class CDlgVisionInspectionComponent2DPara;
class ProcCommonSpecDlg;
class ProcCommonResultDlg;
class ProcCommonDetailResultDlg;
class ProcCommonLogDlg;
class ProcCommonDebugInfoDlg;
class VisionProcessing;
class CDlgSpecDB;
class CDlgPassiveAlgorithm;
class CGridCtrl;
class Component;
class ComponentCollection;
class VisionMapDataEditorUI;
struct ProcessingDlgInfo;

//HDR_6_________________________________ Header body
//
class CDlgVisionInspectionComponent2D : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionInspectionComponent2D)

public:
    CDlgVisionInspectionComponent2D(
        const ProcessingDlgInfo& procDlgInfo, VisionInspectionComponent2D* pVisionInsp, CWnd* pParent = NULL);

    afx_msg void OnBnClickedButtonInspect();
    virtual ~CDlgVisionInspectionComponent2D();

    enum
    {
        IDD = IDD_DIALOG
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()

public:
    const ProcessingDlgInfo& m_procDlgInfo;
    CXTPPropertyGrid* m_propertyGrid;
    ImageLotView* m_imageLotView;
    ProcCommonSpecDlg* m_procCommonSpecDlg;
    ProcCommonResultDlg* m_procCommonResultDlg;
    ProcCommonDetailResultDlg* m_procCommonDetailResultDlg;
    ProcCommonLogDlg* m_procCommonLogDlg;
    ProcCommonDebugInfoDlg* m_procCommonDebugInfoDlg;

    VisionInspectionComponent2D* m_pVisionInsp;
    CDlgVisionInspectionComponent2DPara* m_pVisionInspParaDlg;
    VisionInspectionComponent2DPara* m_pVisionPara;
    CDlgPassiveAlgorithm* m_pDlgPassiveAlgorithm;
    CDlgSpecDB* m_pDlgSpecDB;

    CXTPGridControl m_ChipListControl;
    CComboBox m_cmbSrcImage;

    CPackageSpec& m_packageSpec;

    sPassive_InfoDB* m_pCurPassiveInfoDB;

    long m_nImageID;

public:
    void ShowImage(BOOL bChange = FALSE);

    void UpdateChipList();
    void HideItemDialog();

    void SetPropertyGrid();
    void SetpropertyList();

    BOOL CreatePackageInfo();
    void SaveAsAlgorithmPara(eComponentTypeDefine i_eComponentType, CString strOriName);
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);

    void ResetSpecAndResultDlg();

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};

private: // List, Grid Function
    CTabCtrl m_TabResult;
    afx_msg void OnReportItemClick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnReportRClick(NMHDR* pNotifyStruct, LRESULT* pResult);
    afx_msg void OnReportKeyDown(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnReportItemDblClick(NMHDR* pNotifyStruct, LRESULT* result);

    afx_msg void OnMenuDelete();
    afx_msg void OnMenuChangeSpec();
    afx_msg void OnMenuDmsInfo();

private:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnClose();
    afx_msg void OnTcnSelchangeTabResult(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnShowDetailResult(WPARAM wparam, LPARAM lparam);
    afx_msg void OnBnClickedBtnChipSpecdb();
    afx_msg LRESULT OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/);
};
