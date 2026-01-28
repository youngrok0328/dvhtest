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
class CSurfaceSetMaskNameDlg : public CDialog
{
    // Construction

public:
    CSurfaceSetMaskNameDlg(CWnd* pParent = NULL); // standard constructor

    // Dialog Data
    //{{AFX_DATA(CSurfaceSetMaskNameDlg)
    enum
    {
        IDD = IDD_SURFACE_SET_MASK_NAME
    };
    CString m_strName;
    //}}AFX_DATA

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSurfaceSetMaskNameDlg)

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation

protected:
    // Generated message map functions
    //{{AFX_MSG(CSurfaceSetMaskNameDlg)
    // NOTE: the ClassWizard will add member functions here
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
