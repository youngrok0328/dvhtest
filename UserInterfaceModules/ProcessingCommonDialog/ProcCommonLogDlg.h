#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class DPI_PROCESSING_COMMON_DLG_API ProcCommonLogDlg : public CDialog
{
    DECLARE_DYNAMIC(ProcCommonLogDlg)

public:
    ProcCommonLogDlg() = delete;
    ProcCommonLogDlg(CWnd* pParent, const CRect& rtPositionOnParent); // 표준 생성자입니다.
    virtual ~ProcCommonLogDlg();

    void SetLogBoxText(LPCTSTR text);

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnSize(UINT nType, int cx, int cy);

private:
    const CRect m_rtInitPosition;
    CRichEditCtrl m_RE_Watch;
};
