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
class CDlgSpecMSGBox : public CDialog
{
    DECLARE_DYNAMIC(CDlgSpecMSGBox)

public:
    CDlgSpecMSGBox(CWnd* pParent = NULL);
    virtual ~CDlgSpecMSGBox();

    enum
    {
        IDD = IDD_DLG_SPECDB_MSGBOX
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()

public:
    CString m_strSpecName;
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
