//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingGlobalMaskMaker.h"

//CPP_2_________________________________ This project's headers
#include "DlgSetupUI.h"
#include "ResultData.h"
#include "VisionProcessingGlobalMaskMakerPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionProcessingGlobalMaskMaker::VisionProcessingGlobalMaskMaker(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionProcessing(_VISION_INSP_GUID_GLOBAL_MASK_MAKER, _T("Global Mask Maker"), visionUnit, packageSpec)
    , m_VisionPara(new CVisionProcessingGlobalMaskMakerPara)
    , m_sEdgeAlignResult(new VisionAlignResult)
    , m_result(new ResultData)
{
    m_pVisionInspDlg = NULL;

    m_DebugInfoGroup.Add(_T("Total Mask"), enumDebugInfoType::Image_8u_C1);
}

void VisionProcessingGlobalMaskMaker::ResetSpecAndPara()
{
    m_VisionPara->Init();
}

void VisionProcessingGlobalMaskMaker::ResetResult()
{
    if (m_pVisionInspDlg)
    {
        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
    }

    // Get Algorithm...
    m_pEdgeDetect = getReusableMemory().GetEdgeDetect();
    m_pBlob = getReusableMemory().GetBlob();

    m_result->Reset();

    //-----------------------------------------------------------------------
    // Global Mask Maker 가 만들어 낸 Layer들을 지우고 시작하자
    //-----------------------------------------------------------------------
    CString maskHeader = GetSurfaceMaskNameHeader();

    for (long layerIndex = getReusableMemory().GetSurfaceLayerMaskCount() - 1; layerIndex >= 0; layerIndex--)
    {
        CString name = getReusableMemory().GetSurfaceLayerMaskName(layerIndex);
        name = name.Left(maskHeader.GetLength());
        if (name == maskHeader)
        {
            getReusableMemory().DelSurfaceLayerMask(layerIndex);
        }

        name.Empty();
    }

    maskHeader.Empty();
}

VisionProcessingGlobalMaskMaker::~VisionProcessingGlobalMaskMaker(void)
{
    CloseDlg();

    delete m_sEdgeAlignResult;
    delete m_result;
    delete m_VisionPara;
}

void VisionProcessingGlobalMaskMaker::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionProcessingGlobalMaskMaker::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    m_visionUnit.RunInspection(m_visionUnit.GetPreviousVisionProcessing(this));

    GetEdgeAlignResult();

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgSetupUI(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgSetupUI::IDD, parent);

    m_pVisionInspDlg->ShowWindow(SW_SHOW);

    return 0;
}

BOOL VisionProcessingGlobalMaskMaker::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    if (!m_VisionPara->LinkDataBase(bSave, db[_T("{B3CC00DF-9157-4583-8203-0B9A9A0F7AD2}")]))
        return FALSE;

    return TRUE;
}

void VisionProcessingGlobalMaskMaker::GetOverlayResult(
    VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (overlayResult == nullptr)
    {
        return;
    }

    if (detailSetupMode)
        overlayResult->OverlayReset();
}

void VisionProcessingGlobalMaskMaker::AppendTextResult(CString& textResult)
{
    textResult.AppendFormat(_T("\n< %s Result >\n"), (LPCTSTR)m_strModuleName);
    textResult.AppendFormat(_T("  [ Inspection Time : %.2f ]\n"), m_fCalcTime);
    if (m_bInvalid)
    {
        textResult.AppendFormat(_T("  Invalid - check global masker\n"));
        textResult += m_inspErrorMessage;
    }
}

std::vector<CString> VisionProcessingGlobalMaskMaker::ExportRecipeToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrResult(0);
    if (IsEnabled() == FALSE)
        return vecStrResult;

    const static CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;
    vecStrResult = m_VisionPara->ExportAlgoParaToText(strVisionName, m_strModuleName);

    return vecStrResult;
}