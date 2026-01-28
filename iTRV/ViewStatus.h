#pragma once
//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class DialogITRV;

//HDR_6_________________________________ Header body
//
class ViewStatus : public CDialogEx
{
    DECLARE_DYNAMIC(ViewStatus)

public:
    ViewStatus(DialogITRV* parent);
    virtual ~ViewStatus();

    void UpdateAccessModeText();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_VIEW_STATUS
    };
#endif

protected:
    DialogITRV* m_parent;
    CStatic m_titleVersion;
    CStatic m_valueVersion;
    CStatic m_titleAccessMode;
    CStatic m_valueAccessMode;

    void UpdateLayout();
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.
    virtual BOOL OnInitDialog();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnStnDblclickValueAccessMode();

    DECLARE_MESSAGE_MAP()
};
