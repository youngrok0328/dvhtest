//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgLayerName.h"

//CPP_2_________________________________ This project's headers
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <afxdialogex.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(DlgLayerName, CDialogEx)

DlgLayerName::DlgLayerName(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_DIALOG_LAYERNAME, pParent)
    , m_name(_T("Unknown"))
{
}

DlgLayerName::~DlgLayerName()
{
}

void DlgLayerName::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_NAME, m_name);
}

BEGIN_MESSAGE_MAP(DlgLayerName, CDialogEx)
END_MESSAGE_MAP()

// DlgLayerName message handlers

BOOL DlgLayerName::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    return TRUE; // return TRUE unless you set the focus to a control
        // EXCEPTION: OCX Property Pages should return FALSE
}
