#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class CDlgLogin : public CDialog
{
    DECLARE_DYNAMIC(CDlgLogin)

public:
    CDlgLogin(CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgLogin();

    void SetAccessMode(long nAccessLevel);
    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DLGLOGIN
    };
    int m_nMode;
    BOOL m_bHiddenView;
    BOOL m_bSystemSaveMode;

    void ShowControl();

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedOperator();
    afx_msg void OnBnClickedEngineer();
    afx_msg void OnBnClickedIntekplus();
    virtual BOOL OnInitDialog();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
};
