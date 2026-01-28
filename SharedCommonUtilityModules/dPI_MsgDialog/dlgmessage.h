#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "Resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedComponent/Label/Label.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class CdlgMessage : public CDialog
{
    // Construction

public:
    CdlgMessage(CWnd* pParent = NULL, LPCTSTR lpStr = _T("")); // standard constructor

    // Dialog Data
    //{{AFX_DATA(CdlgMessage)
    enum
    {
        IDD = IDD_MESSAGE
    };
    CLabel m_lblMsg;
    CString m_strMsg;
    //}}AFX_DATA

    int m_nResult;
    int m_nMBType;

    // Overrides
    virtual int DoModal(int nType = MB_OK);
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CdlgMessage)

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation

protected:
    // Generated message map functions
    //{{AFX_MSG(CdlgMessage)
    virtual BOOL OnInitDialog();
    afx_msg void OnNo();
    virtual void OnOK();
    virtual void OnCancel();
    afx_msg void OnYes();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
