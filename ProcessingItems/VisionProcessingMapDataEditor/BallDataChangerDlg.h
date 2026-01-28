#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class CBallDataChangerDlg : public CDialog
{
    DECLARE_DYNAMIC(CBallDataChangerDlg)

public:
    CBallDataChangerDlg(CWnd* pParent /*= nullptr*/, double& nBallHeight); // 표준 생성자입니다.
    virtual ~CBallDataChangerDlg();
    virtual BOOL OnInitDialog();

    // 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DLG_BALLDATA_CHANGER
    };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    CXTPPropertyGrid* m_propertyGrid;

    double* m_BallHeight;

public:
    void SetPropertyGrid();

public:
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
    afx_msg void OnBnClickedOk();
};
