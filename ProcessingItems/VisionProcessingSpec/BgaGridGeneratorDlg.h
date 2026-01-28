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
class BgaGridGeneratorDlg : public CDialog
{
    // Construction

public:
    BgaGridGeneratorDlg(CWnd* pParent = NULL); // standard constructor

    // Dialog Data
    //{{AFX_DATA(BgaGridGeneratorDlg)
    enum
    {
        IDD = IDD_BGA_GRID_GENERATOR
    };

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(BgaGridGeneratorDlg)

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation

protected:
    // Generated message map functions
    //{{AFX_MSG(BgaGridGeneratorDlg)
    virtual void OnOK();
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

public:
    long m_gridCountX;
    long m_gridCountY;
    double m_gridPitchX_um;
    double m_gridPitchY_um;
    double m_ballDiameter_um;
    double m_ballHeight_um;
    bool m_bIgnore;
};