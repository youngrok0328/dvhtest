#pragma once
#pragma once
//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionMainTR;

//HDR_6_________________________________ Header body
//
class ViewVisionBody : public CDialogEx
{
    DECLARE_DYNAMIC(ViewVisionBody)

public:
    ViewVisionBody(VisionMainTR& visionMain, CWnd* parent);
    virtual ~ViewVisionBody();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_VIEW_VISION_BODY
    };
#endif

protected:
    VisionMainTR& m_visionMain;
    bool m_initialized = false;
    void UpdateLayout();

    void OnOK() override;
    void OnCancel() override;
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.
    virtual BOOL OnInitDialog();
    afx_msg void OnSize(UINT nType, int cx, int cy);

    DECLARE_MESSAGE_MAP()
};
