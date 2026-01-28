#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class CDlgChipNameEdit : public CDialog
{
    DECLARE_DYNAMIC(CDlgChipNameEdit)

public:
    CDlgChipNameEdit(CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgChipNameEdit();

#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DIALOG_CHIP_NAME
    };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    CString m_strChipName;

    afx_msg void OnBnClickedOk();
};
