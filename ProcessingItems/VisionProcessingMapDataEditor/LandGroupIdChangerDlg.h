#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class CLandGroupIdChangerDlg : public CDialog
{
    DECLARE_DYNAMIC(CLandGroupIdChangerDlg)

public:
    CLandGroupIdChangerDlg(CWnd* pParent /*= nullptr*/, long& nGroupID, double& nLandHeight); // 표준 생성자입니다.
    virtual ~CLandGroupIdChangerDlg();
    virtual BOOL OnInitDialog();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DLG_LANDGROUP_CHANGER
    };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    CXTPPropertyGrid* m_propertyGrid;

    long* m_nLandGroup;
    double* m_LandHeight;

public:
    void SetPropertyGrid();

public:
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
    afx_msg void OnBnClickedOk();
};
