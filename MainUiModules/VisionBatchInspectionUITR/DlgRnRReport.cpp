//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgRnRReport.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CDlgRnRReport, CDialog)

CDlgRnRReport::CDlgRnRReport(CWnd* pParent /*=NULL*/)
    : CDialog(CDlgRnRReport::IDD, pParent)
{
}

CDlgRnRReport::~CDlgRnRReport()
{
}

void CDlgRnRReport::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CUSTOM_GRID_RNR, m_gridRnR);
}

BEGIN_MESSAGE_MAP(CDlgRnRReport, CDialog)
END_MESSAGE_MAP()

// CDlgRnRReport 메시지 처리기입니다.
