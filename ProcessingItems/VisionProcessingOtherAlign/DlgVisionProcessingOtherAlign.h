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
class VisionProcessingOtherAlign;
class CVisionProcessingOtherAlignPara;
class ImageLotView;
class ProcCommonSpecDlg;
class ProcCommonResultDlg;
class ProcCommonDetailResultDlg;
class ProcCommonLogDlg;
class ProcCommonDebugInfoDlg;
class VisionAlignResult;
struct ProcessingDlgInfo;

//HDR_6_________________________________ Header body
//
class CDlgVisionProcessingOtherAlign : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionProcessingOtherAlign)

public:
    CDlgVisionProcessingOtherAlign(const ProcessingDlgInfo& procDlgInfo, VisionProcessingOtherAlign* pVisionInsp,
        CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgVisionProcessingOtherAlign();

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
    VisionProcessingOtherAlign* m_pVisionInsp;
    CVisionProcessingOtherAlignPara* m_pVisionPara;
    ImageLotView* m_imageLotView;
    CXTPPropertyGrid* m_propertyGrid;
    ProcCommonLogDlg* m_procCommonLogDlg;
    ProcCommonDebugInfoDlg* m_procCommonDebugInfoDlg;

    long m_nImageID;

    //-----------------------------
    //DLG 초기화를 위한 변수 & 함수
    //-----------------------------

public:
    CTabCtrl m_TabResult;

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};

    afx_msg void OnDestroy();
    afx_msg void OnClose();
    afx_msg void OnBnClickedButtonInspect();
    afx_msg void OnTcnSelchangeTabResult(NMHDR* pNMHDR, LRESULT* pResult);

    afx_msg LRESULT OnImageLotViewPaneSelChanged(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnImageLotViewRoiChanged(WPARAM wparam, LPARAM lparam);
    LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
    void UpdatePropertyGrid();
    void ClickedButtonImageCombine();
    void GetROI();
    void SetROI();
    void UpdateROI();
};
