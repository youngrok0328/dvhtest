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
class VisionInspectionBgaBottom3D;
class ProcCommonSpecDlg;
class ProcCommonResultDlg;
class ProcCommonDetailResultDlg;
class ProcCommonLogDlg;
class ProcCommonDebugInfoDlg;
struct ProcessingDlgInfo;

//HDR_6_________________________________ Header body
//
class CDlgVisionInspectionBgaBottom3D : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionInspectionBgaBottom3D)

public:
    CDlgVisionInspectionBgaBottom3D(const ProcessingDlgInfo& procDlgInfo, VisionInspectionBgaBottom3D* pVisionInsp,
        CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgVisionInspectionBgaBottom3D();

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
    VisionInspectionBgaBottom3D* m_pVisionInsp;
    ImageLotView* m_imageLotView;
    CXTPPropertyGrid* m_propertyGrid;
    ProcCommonSpecDlg* m_procCommonSpecDlg;
    ProcCommonResultDlg* m_procCommonResultDlg;
    ProcCommonDetailResultDlg* m_procCommonDetailResultDlg;
    ProcCommonLogDlg* m_procCommonLogDlg;
    ProcCommonDebugInfoDlg* m_procCommonDebugInfoDlg;
    CXTPPropertyGridItem* m_BUCPCategory;

    long m_nSelectGroupID;

private:
    void SetPropertyCommonParam();
    void SetPropertyGroupParam();
    void SetupGroupParam(long i_nSelectGroupID);

public:
    void ResetSpecAndResultDlg();
    void btnClickInspectionAdd();
    void btnClickInspectionDel(long Targetindex);
    void btnClickInspectionSetup(long Targetindex);
    void UpdateBUCPGroup();
    BOOL MakeSpecROI(long i_nGroupID, std::vector<PI_RECT>& o_vecpirtSpecROI);

public:
    CTabCtrl m_TabResult;
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};
    afx_msg void OnDestroy();
    afx_msg void OnClose();
    afx_msg void OnTcnSelchangeTabResult(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnShowDetailResult(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/);
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);

public:
    void ViewBallSearchROI(long i_nBallGroupidx);
};
