#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxdialogex.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class DlgLayerName : public CDialogEx
{
    DECLARE_DYNAMIC(DlgLayerName)

public:
    DlgLayerName(CWnd* pParent = nullptr); // standard constructor
    virtual ~DlgLayerName();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DIALOG_LAYERNAME
    };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    DECLARE_MESSAGE_MAP()

public:
    CString m_name;
    virtual BOOL OnInitDialog();
};
