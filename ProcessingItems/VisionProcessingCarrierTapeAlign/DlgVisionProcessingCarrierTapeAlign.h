#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedComponent/Label/NewLabel.h"

//HDR_3_________________________________ External library headers
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class ImageLotView;
class VisionProcessingCarrierTapeAlign;
class ProcCommonGridGroupDlg;
class ProcCommonLogDlg;
class ProcCommonDebugInfoDlg;
struct ProcessingDlgInfo;

//HDR_6_________________________________ Header body
//
class DlgVisionProcessingCarrierTapeAlign : public CDialog
{
    DECLARE_DYNAMIC(DlgVisionProcessingCarrierTapeAlign)

public:
    DlgVisionProcessingCarrierTapeAlign(const ProcessingDlgInfo& procDlgInfo,
        VisionProcessingCarrierTapeAlign* pVisionInsp,
        CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~DlgVisionProcessingCarrierTapeAlign();

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
    ImageLotView* m_imageLotView;
    VisionProcessingCarrierTapeAlign* m_pVisionInsp;
    ProcCommonGridGroupDlg* m_procCommonGridGroup;
    ProcCommonLogDlg* m_procCommonLogDlg;
    ProcCommonDebugInfoDlg* m_procCommonDebugInfoDlg;

public:
    CTabCtrl m_TabResult;

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};

    afx_msg void OnDestroy();
    afx_msg void OnClose();
    afx_msg void OnTcnSelchangeTabResult(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnImageLotViewPaneSelChanged(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnImageLotViewRoiChanged(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnImageLotViewSelChangedRaw(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnImageLotViewSelChangedImage(WPARAM wparam, LPARAM lparam);

    void SetAlign2D_NormalVision();

private:
    void ShowAlign2DNormalUI();
};
