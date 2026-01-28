#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class DlgSelectVision : public CDialogEx
{
    DECLARE_DYNAMIC(DlgSelectVision)

public:
    DlgSelectVision(CWnd* pParent = nullptr); // standard constructor
    virtual ~DlgSelectVision();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_SELECT_VISION
    };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    DECLARE_MESSAGE_MAP()

public:
    int m_visionIndex;
};
