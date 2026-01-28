//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgSaveMomoryLogInterval.h"

//CPP_2_________________________________ This project's headers
#include "DlgPrimaryBatchList.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CDlgSaveMomoryLogInterval, CDialog)

CDlgSaveMomoryLogInterval::CDlgSaveMomoryLogInterval(CWnd* pParent /*=NULL*/)
    : CDialog(CDlgSaveMomoryLogInterval::IDD, pParent)
    , m_strSaveMemoryLogInterval(_T("1"))
{
}

CDlgSaveMomoryLogInterval::~CDlgSaveMomoryLogInterval()
{
}

void CDlgSaveMomoryLogInterval::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_TIME_INTERVAL, m_strSaveMemoryLogInterval);
}

BEGIN_MESSAGE_MAP(CDlgSaveMomoryLogInterval, CDialog)
END_MESSAGE_MAP()

// CDlgSaveMomoryLogInterval 메시지 처리기입니다.
