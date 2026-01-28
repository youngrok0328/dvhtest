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
class CXTPPropertyGrid;
class ProcCommonSpecDlg;
class ProcCommonResultDlg;
class ProcCommonDetailResultDlg;
class ProcCommonLogDlg;
class ProcCommonDebugInfoDlg;
class VisionInspectionBgaBallPixelQuality;
class VisionInspectionBgaBallPixelQualityPara;
struct ProcessingDlgInfo;

//HDR_6_________________________________ Header body
//
class CDlgVisionInspectionBgaBallPixelQuality : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionInspectionBgaBallPixelQuality)

public:
    CDlgVisionInspectionBgaBallPixelQuality(const ProcessingDlgInfo& procDlgInfo,
        VisionInspectionBgaBallPixelQuality* pVisionInsp, CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgVisionInspectionBgaBallPixelQuality();

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
    VisionInspectionBgaBallPixelQuality* m_pVisionInsp;
    VisionInspectionBgaBallPixelQualityPara* m_pVisionPara;
    ImageLotView* m_imageLotView;
    CXTPPropertyGrid* m_propertyGrid;
    ProcCommonSpecDlg* m_procCommonSpecDlg;
    ProcCommonResultDlg* m_procCommonResultDlg;
    ProcCommonDetailResultDlg* m_procCommonDetailResultDlg;
    ProcCommonLogDlg* m_procCommonLogDlg;
    ProcCommonDebugInfoDlg* m_procCommonDebugInfoDlg;

    BOOL m_bShowResultDetail;
    long m_nImageID;

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};
    afx_msg void OnDestroy();
    afx_msg void OnClose();
    afx_msg void OnBnClickedButtonClose();
    afx_msg void OnBnClickedButtonInspect();
    afx_msg LRESULT OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/);

public:
    CTabCtrl m_TabResult;
    LRESULT OnShowDetailResult(WPARAM wparam, LPARAM lparam);

    void UpdatePropertyGrid();
    long GetTotalResult();
    void OnTcnSelchangeTabResult(NMHDR* pNMHDR, LRESULT* pResult);
    LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
    void SelchangeComboDebugView(int nSelectedID);
    void ClickedButtonImageCombine();
    void UpdateThresholdImage(long nRingID);
};
