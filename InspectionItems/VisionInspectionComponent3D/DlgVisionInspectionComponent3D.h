#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"

//HDR_3_________________________________ External library headers
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class ChipInfo;
class ImageLotView;
class VisionInspectionComponent3D;
class VisionInspectionComponent3DPara;
class ProcCommonSpecDlg;
class ProcCommonResultDlg;
class ProcCommonDetailResultDlg;
class ProcCommonLogDlg;
class ProcCommonDebugInfoDlg;
class CDlg3DSpecDB;
class CDlgComponentAlign;
class CDlg3DView;
struct ProcessingDlgInfo;
struct sPassive_InfoDB;

//HDR_6_________________________________ Header body
//
class CDlgVisionInspectionComponent3D : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionInspectionComponent3D)

public:
    CDlgVisionInspectionComponent3D(
        const ProcessingDlgInfo& procDlgInfo, VisionInspectionComponent3D* pVisionInsp, CWnd* pParent = NULL);
    virtual ~CDlgVisionInspectionComponent3D();

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

    VisionInspectionComponent3D* m_pVisionInsp;
    VisionInspectionComponent3DPara* m_pVisionPara;
    CDlgComponentAlign* m_pDlgCompAlign;
    CDlg3DSpecDB* m_pDlg3DSpecDB;

    CXTPGridControl m_chipListControl;

    sPassive_InfoDB* m_pCurPassiveInfoDB;
    //아래꺼는 상관없으면 다 짜른다
    BOOL m_bShowResultDetail;
    long m_nImageID;

    long m_nAddNum;
    CDlg3DView* m_p3DViewer;
    ChipInfo* m_pSelChip; // Chip List에서 선택된 Chip
    long m_nCmbSelImage;

public:
    void SetInitButton();
    void ShowImage();

    void SetPropertyGrid();
    void SetpropertyList();

    void UpdateChipList();

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

    // List, Grid 관련 함수
    afx_msg void OnReportItemClick(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnReportRClick(NMHDR* pNotifyStruct, LRESULT* pResult);
    afx_msg void OnReportItemDblClick(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnReportKeyDown(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);

    afx_msg void OnMenuChangeSpec();
    afx_msg void OnMenuChangeDMSInfo();
    afx_msg void OnMenuDmsInfo();

    void SaveAsAlgorithmPara(eComponentTypeDefine i_eComponentType, CString strOriName);
    void OnBnClickedButtonSpecdb();
    void HideItemDialog();

public:
    CTabCtrl m_TabResult;
    CTabCtrl m_TabLog;
    CButton m_buttonApply;
    CButton m_buttonInspect;
    CButton m_buttonClose;
    afx_msg void OnCbnSelchangeComboCurrpane();
    afx_msg LRESULT OnRoiChanged(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/);

    CComboBox m_ComboCurrPane;
    long m_nCurID;
};
