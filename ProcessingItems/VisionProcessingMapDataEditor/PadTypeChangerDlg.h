#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class CPadTypeChangerDlg : public CDialog
{
    DECLARE_DYNAMIC(CPadTypeChangerDlg)

public:
    CPadTypeChangerDlg(CWnd* pParent /* = nullptr*/, long& nSelectedType); // 표준 생성자입니다.
    virtual ~CPadTypeChangerDlg();
    virtual BOOL OnInitDialog();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DLG_MAPDATA_PADTYPE_CHANGER
    };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    CComboBox m_cmbPadType;
    long* m_nSelectedPadType;

public:
    afx_msg void OnCbnSelchangeComboPadtype();
    afx_msg void OnBnClickedOk();
};
