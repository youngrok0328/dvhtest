//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "dPI_ImageCombine.h"

//CPP_2_________________________________ This project's headers
#include "DlgImageGrayProcManager.h"
#include "dPI_ImageCombineApp.h"

//CPP_3_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Rect32s.h>

//CPP_5_________________________________ Standard library headers
#include <vector>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BEGIN_MESSAGE_MAP(CdPI_ImageCombineApp, CWinApp)
END_MESSAGE_MAP()

CdPI_ImageCombineApp::CdPI_ImageCombineApp()
{
}

CdPI_ImageCombineApp theApp;

BOOL CdPI_ImageCombineApp::InitInstance()
{
    CWinApp::InitInstance();

    return TRUE;
}

INT_PTR Combine_SetParameter(
    VisionProcessing& proc, long baseFrameIndex, bool isRaw, ImageProcPara* pProcManagePara, Ipvm::Rect32s rtPaneROI)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CDlgImageGrayProcManager dlg(proc, isRaw, baseFrameIndex);

    dlg.SetParameter(pProcManagePara, rtPaneROI);

    return dlg.DoModal();
}

INT_PTR Combine_SetParameter(VisionProcessing& proc, long baseFrameIndex, bool isRaw, ImageProcPara* pProcManagePara)
{
    Ipvm::Rect32s rtPaneROI(0, 0, 0, 0);
    if (isRaw)
    {
        rtPaneROI = proc.getImageLot().GetImageRect();
    }
    else
    {
        if (proc.getImageLotInsp().m_vecImages[proc.GetCurVisionModule_Status()].size())
        {
            rtPaneROI = Ipvm::Rect32s(proc.getImageLotInsp().m_vecImages[proc.GetCurVisionModule_Status()][0]);
        }
    }

    return Combine_SetParameter(proc, baseFrameIndex, isRaw, pProcManagePara, rtPaneROI);
}
