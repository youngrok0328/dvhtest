//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgSelectVision.h"

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
IMPLEMENT_DYNAMIC(DlgSelectVision, CDialogEx)

DlgSelectVision::DlgSelectVision(CWnd* pParent /*=nullptr*/)
    : CDialogEx(IDD_SELECT_VISION, pParent)
    , m_visionIndex(0)
{
}

DlgSelectVision::~DlgSelectVision()
{
}

void DlgSelectVision::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_CBIndex(pDX, IDC_COMBO_VISION, m_visionIndex);
}

BEGIN_MESSAGE_MAP(DlgSelectVision, CDialogEx)
END_MESSAGE_MAP()

// DlgSelectVision message handlers
