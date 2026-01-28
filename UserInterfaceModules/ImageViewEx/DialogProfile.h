#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/LineSeg32r.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class ImageViewExImpl;

//HDR_6_________________________________ Header body
//
class DialogProfile : public CDialogEx
{
    DECLARE_DYNAMIC(DialogProfile)

public:
    DialogProfile(ImageViewExImpl* pParent); // standard constructor
    virtual ~DialogProfile();

    void UpdateProfile();
    void UpdateProfile(const Ipvm::LineSeg32r& line);

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DIALOG_PROFILE
    };
#endif

public:
    Ipvm::LineSeg32r m_line;

    ImageViewExImpl* m_parent;
    Ipvm::ProfileView* m_view;
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnOK();
    afx_msg void OnCancel();
    afx_msg void OnDestroy();
};
