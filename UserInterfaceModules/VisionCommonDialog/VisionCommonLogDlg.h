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
#define UM_ADD_LOG_WARNING (WM_USER + 9001) //TODO: 메시지 아이디를 외부에서 지정하는 것으로 바꿀 것

class DPI_VISION_COMMON_DLG_API VisionCommonLogDlg : public CDialog
{
    DECLARE_DYNAMIC(VisionCommonLogDlg)

public:
    VisionCommonLogDlg() = delete;
    VisionCommonLogDlg(CWnd* pParent, const CRect& rtPositionOnParent); // 표준 생성자입니다.
    virtual ~VisionCommonLogDlg();

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:

private:
    const CRect m_rtInitPosition;

    CButton m_chkWarning;
    CButton m_chkEvent;
    CButton m_chkThread;
    CButton m_chkTCP;
    CButton m_chkDevelop;
    CButton m_chkNotice;
    CListBox m_logBox;

    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedCheckWarning();
    afx_msg void OnBnClickedCheckEvent();
    afx_msg void OnBnClickedCheckThread();
    afx_msg void OnBnClickedCheckTcp();
    afx_msg void OnBnClickedCheckDevelop();
    afx_msg void OnBnClickedCheckNotice();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg LRESULT OnUpdateLog(WPARAM, LPARAM);
};
