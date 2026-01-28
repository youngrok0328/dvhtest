#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class ImageLotView;
class VisionInspectionPackageSize;
class VisionInspectionPackageSizePara;
class ProcCommonSpecDlg;
class ProcCommonResultDlg;
class ProcCommonDetailResultDlg;
class ProcCommonLogDlg;
class ProcCommonDebugInfoDlg;
struct ProcessingDlgInfo;

//HDR_6_________________________________ Header body
//
class CDlgVisionInspectionPackageSize : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionInspectionPackageSize)

public:
    CDlgVisionInspectionPackageSize(const ProcessingDlgInfo& procDlgInfo, VisionInspectionPackageSize* pVisionInsp,
        CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgVisionInspectionPackageSize();

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
    VisionInspectionPackageSize* m_pVisionInsp;
    VisionInspectionPackageSizePara* m_pVisionPara;
    ImageLotView* m_imageLotView;
    CXTPPropertyGrid* m_propertyGrid;
    ProcCommonSpecDlg* m_procCommonSpecDlg;
    ProcCommonResultDlg* m_procCommonResultDlg;
    ProcCommonDetailResultDlg* m_procCommonDetailResultDlg;
    ProcCommonLogDlg* m_procCommonLogDlg;
    ProcCommonDebugInfoDlg* m_procCommonDebugInfoDlg;

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
};
