#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class CSimulatorForTestDlg : public CDialogEx
{
    // Construction

public:
    CSimulatorForTestDlg(CWnd* pParent = nullptr); // standard constructor
    virtual ~CSimulatorForTestDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_SIMULATORFORTEST_DIALOG
    };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    // Implementation

protected:
    Ipvm::SocketMessaging* m_socket;
    HICON m_hIcon;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedBtnConnect();
    afx_msg void OnBnClickedBtnDisconnect();
    afx_msg LRESULT OnSocketConnection(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSocketReceived(WPARAM wParam, LPARAM lParam);
    afx_msg void OnBnClickedBtnSetInlineStart();
    afx_msg void OnBnClickedBtnSetInlineStop();
    afx_msg void OnBnClickedBtnSendTestJob();
};
