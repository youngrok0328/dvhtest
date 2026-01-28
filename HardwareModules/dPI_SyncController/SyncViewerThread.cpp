//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SyncViewerThread.h"

//CPP_2_________________________________ This project's headers
#include "CNGRVDioView.h"
#include "DioView.h"
#include "SyncController.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
static long g_count = 0;

IMPLEMENT_DYNCREATE(SyncViewerThread, CWinThread)

SyncViewerThread::SyncViewerThread()
{
    g_count++;
}

SyncViewerThread::~SyncViewerThread()
{
    g_count--;
}

BOOL SyncViewerThread::InitInstance()
{
    if (g_count > 1)
    {
        return FALSE;
    }

    CWinThread::InitInstance();

    if (SystemConfig::GetInstance().m_nVisionType != VISIONTYPE_NGRV_INSP)
    {
        CDioView dlg(SyncController::GetInstance().GetSync());

        m_pMainWnd = &dlg;

        dlg.DoModal();
    }
    else
    {
        CNGRVDioView dlg(SyncController::GetInstance().GetSync());

        m_pMainWnd = &dlg;

        dlg.DoModal();
    }

    return FALSE;
}

int SyncViewerThread::ExitInstance()
{
    delete m_pMainWnd;
    m_pMainWnd = nullptr;

    return CWinThread::ExitInstance();
}

BEGIN_MESSAGE_MAP(SyncViewerThread, CWinThread)
//{{AFX_MSG_MAP(SyncViewerThread)
// NOTE - the ClassWizard will add and remove mapping macros here.
//}}AFX_MSG_MAP
END_MESSAGE_MAP()