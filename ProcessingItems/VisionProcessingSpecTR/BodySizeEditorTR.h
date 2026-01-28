#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "Resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CPackageSpec;

//HDR_6_________________________________ Header body
//
class BodySizeEditorTR : public CDialog
{
    // Construction

public:
    BodySizeEditorTR(CPackageSpec& packageSpec, CWnd* pParent = NULL); // standard constructor

    // Dialog Data
    //{{AFX_DATA(BodySizeEditorTR)
    enum
    {
        IDD = IDD_DIALOG_TR_BODY_SIZE
    };
    float m_fBodySizeX;
    float m_fBodySizeY;
    float m_fBodySizeZ;

    int m_nUnit;

    BOOL m_bDeadBug; //kircheis_20160622
    BOOL m_bChangedDeadBug;
    //}}AFX_DATA

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(BodySizeEditorTR)

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation

protected:
    CPackageSpec& m_packageSpec;

    // Generated message map functions
    //{{AFX_MSG(BodySizeEditorTR)
    virtual void OnOK();
    afx_msg void OnRadioUnitMil();
    afx_msg void OnRadioUnitMm();
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

public:
};