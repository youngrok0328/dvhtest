//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingCarrierTapeAlign.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingCarrierTapeAlign.h"
#include "Inspection.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"
#include "../../VisionNeedLibrary/VisionEdgeAlign/Para.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionProcessingCarrierTapeAlign::VisionProcessingCarrierTapeAlign(
    VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionProcessing(_VISION_INSP_GUID_CARRIER_TAPE_ALIGN, _T("Carrier Tape Align"), visionUnit, packageSpec)
    , m_visionPara(*this)
    , m_constants(this)
{
    m_pVisionInspDlg = NULL;

    m_inspector = new Inspection(*this);
}

void VisionProcessingCarrierTapeAlign::ResetSpecAndPara()
{
    m_visionPara.Init();
}

VisionProcessingCarrierTapeAlign::~VisionProcessingCarrierTapeAlign(void)
{
    CloseDlg();

    delete m_inspector;
}

void VisionProcessingCarrierTapeAlign::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionProcessingCarrierTapeAlign::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();
    GetConstants();
    ResetResult();

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new DlgVisionProcessingCarrierTapeAlign(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(DlgVisionProcessingCarrierTapeAlign::IDD, parent);

    m_visionUnit.RunInspection(m_visionUnit.GetPreviousVisionProcessing(this), false, GetCurVisionModule_Status());

    m_pVisionInspDlg->ShowWindow(SW_SHOW);

    return 0;
}

BOOL VisionProcessingCarrierTapeAlign::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    if (!m_visionPara.LinkDataBase(bSave, db[_T("{CDE5DD4C-46BD-4691-B87C-0CC9441B482A}")]))
    {
        return FALSE;
    }

    return TRUE;
}

void VisionProcessingCarrierTapeAlign::GetOverlayResult(
    VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (overlayResult == nullptr)
    {
        return;
    }

    if (detailSetupMode)
        overlayResult->OverlayReset();

    m_inspector->GetOverlayResult(overlayResult, m_nOverlayMode, detailSetupMode);
}

void VisionProcessingCarrierTapeAlign::AppendTextResult(CString& textResult)
{
    textResult.AppendFormat(_T("\r\n< %s Result >\r\n"), (LPCTSTR)m_strModuleName);
    textResult.AppendFormat(_T("  [ Inspection Time : %.2f ]\r\n"), m_fCalcTime);
}

void VisionProcessingCarrierTapeAlign::ResetResult()
{
    m_inspector->resetResult();
    ResetDebugInfo();
}

std::vector<CString> VisionProcessingCarrierTapeAlign::ExportRecipeToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrResult(0);
    if (IsEnabled() == FALSE)
        return vecStrResult;

    const static CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;
    vecStrResult = m_visionPara.ExportAlgoParaToText(strVisionName, m_strModuleName);

    return vecStrResult;
}