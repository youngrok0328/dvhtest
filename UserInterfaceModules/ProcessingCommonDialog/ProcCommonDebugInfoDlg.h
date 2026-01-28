#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CGridCtrl;
class ImageLotView;
class ProcCommonLogDlg;
class VisionDebugInfoGroup;

//HDR_6_________________________________ Header body
//
class DPI_PROCESSING_COMMON_DLG_API ProcCommonDebugInfoDlg : public CDialog
{
    DECLARE_DYNAMIC(ProcCommonDebugInfoDlg)

public:
    ProcCommonDebugInfoDlg() = delete;
    ProcCommonDebugInfoDlg(CWnd* pParent, const CRect& rtPositionOnParent, VisionDebugInfoGroup& debugInfoGroup,
        ImageLotView& imageLotView, ProcCommonLogDlg& logDlg);
    virtual ~ProcCommonDebugInfoDlg();

    void Display(long key);
    void Refresh();

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnGridSelChanged(NMHDR* pNMHDR, LRESULT* pResult);

private:
    CGridCtrl* m_pGridCtrl;
    const CRect m_rtInitPosition;
    VisionDebugInfoGroup& m_debugInfoGroup;
    ImageLotView& m_imageLotView;
    ProcCommonLogDlg& m_logDlg;
};
