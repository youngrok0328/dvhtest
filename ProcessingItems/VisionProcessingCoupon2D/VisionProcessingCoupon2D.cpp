//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingCoupon2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingCoupon2D.h"
#include "VisionProcessingCoupon2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
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
VisionProcessingCoupon2D::VisionProcessingCoupon2D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionProcessing(_VISION_INSP_GUID_COUPON_2D, _T("Coupon 2D"), visionUnit, packageSpec)
    , m_VisionPara(new VisionProcessingCoupon2DPara(*this))
{
    m_pVisionInspDlg = NULL;

    m_DebugInfoGroup.Add(_T("Coupon Check ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Coupon Blob Rect"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Coupon Blob Count"), enumDebugInfoType::Long);
    m_DebugInfoGroup.Add(_T("Coupon Max Blob Size"), enumDebugInfoType::Long);
}

void VisionProcessingCoupon2D::ResetSpecAndPara()
{
}

void VisionProcessingCoupon2D::ResetResult()
{
}

VisionProcessingCoupon2D::~VisionProcessingCoupon2D(void)
{
    delete m_VisionPara;
    CloseDlg();
}

void VisionProcessingCoupon2D::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionProcessingCoupon2D::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();
    ResetDebugInfo();

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgVisionProcessingCoupon2D(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionProcessingCoupon2D::IDD, parent);

    m_pVisionInspDlg->ShowWindow(SW_SHOW);

    return 0;
}

void VisionProcessingCoupon2D::GetOverlayResult(
    VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    UNREFERENCED_PARAMETER(detailSetupMode);
    UNREFERENCED_PARAMETER(overlayResult);
}

void VisionProcessingCoupon2D::AppendTextResult(CString& /*textResult*/)
{
}

BOOL VisionProcessingCoupon2D::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    // Link Parameter
    if (!m_VisionPara->LinkDataBase(bSave, db[_T("{016F9CF9-9B0F-4359-984D-2840D504413B}")]))
        return FALSE;

    return TRUE;
}

std::vector<CString> VisionProcessingCoupon2D::ExportRecipeToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrResult(0);
    if (IsEnabled() == FALSE)
        return vecStrResult;

    const static CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;
    vecStrResult = m_VisionPara->ExportAlgoParaToText(strVisionName, m_strModuleName);

    return vecStrResult;
}