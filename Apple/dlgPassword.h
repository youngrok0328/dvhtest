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
class CdlgPassword : public CDialog
{
    DECLARE_DYNAMIC(CdlgPassword)

public:
    CdlgPassword(CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CdlgPassword();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG1
    };

    CString m_strPassword;

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    virtual BOOL OnInitDialog();
};
