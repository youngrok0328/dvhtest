//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingSideDetailAlign.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingSideDetailAlign.h"
#include "VisionProcessingSideDetailAlignPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
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
VisionProcessingSideDetailAlign::VisionProcessingSideDetailAlign(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionProcessing(_VISION_INSP_GUID_SIDE_DETAIL_ALIGN, _T("Side Detail Align"), visionUnit, packageSpec)
    , m_VisionPara(new VisionProcessingSideDetailAlignPara(*this))
    , m_sEdgeAlignResult(new VisionAlignResult)
{
    m_pVisionInspDlg = NULL;

    m_DebugInfoGroup.Add(DEBUGINFO_SDA_SpecROI,
        enumDebugInfoType::
            Rect); //Add 함수의 첫번째 파라미터는 문자열인데 이 문자열을 정보를 등록할 때도 써야한다.. 이때 오타 신경쓰는게 싫어서 VisionBaseDef.h에 #define 걸어 버림

    m_DebugInfoGroup.Add(DEBUGINFO_SDA_SubstrateRoughSearchROI_TB, enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(DEBUGINFO_SDA_SubstrateRoughEdgePoint_TB, enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(DEBUGINFO_SDA_SubstrateSearchROI_TB, enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(DEBUGINFO_SDA_SubstrateEdgePoint_TB, enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(DEBUGINFO_SDA_SubstrateSearchROI_LR, enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(DEBUGINFO_SDA_SubstrateEdgePoint_LR, enumDebugInfoType::Point_32f_C2);

    m_DebugInfoGroup.Add(DEBUGINFO_SDA_GlassRoughSearchROI_TB, enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(DEBUGINFO_SDA_GlassRoughEdgePoint_TB, enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(DEBUGINFO_SDA_GlassSearchROI_TB, enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(DEBUGINFO_SDA_GlassEdgePoint_TB, enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(DEBUGINFO_SDA_GlassSearchROI_LR, enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(DEBUGINFO_SDA_GlassEdgePoint_LR, enumDebugInfoType::Point_32f_C2);

    //이하는 Surface Mask외에 VisionInspectionSide2D::GetSideAlignInfo()와 엮여 있음.
    m_DebugInfoGroup.Add(DEBUGINFO_SDA_NormalUnitAlignResult, enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(DEBUGINFO_SDA_TopSubstrateAlignResult, enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(DEBUGINFO_SDA_BottomSubstrateAlignResult, enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(DEBUGINFO_SDA_GlassAlignResult, enumDebugInfoType::Point_32f_C2);
}

void VisionProcessingSideDetailAlign::ResetSpecAndPara()
{
}

void VisionProcessingSideDetailAlign::ResetResult()
{
    m_pEdgeDetect = getReusableMemory().GetEdgeDetect();

    if (m_pVisionInspDlg)
    {
        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
    }
}

VisionProcessingSideDetailAlign::~VisionProcessingSideDetailAlign(void)
{
    delete m_VisionPara;
    CloseDlg();

    delete m_sEdgeAlignResult;
}

void VisionProcessingSideDetailAlign::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionProcessingSideDetailAlign::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();
    ResetDebugInfo();

    if (m_visionUnit.IsTheUsingVisionProcessingByGuid(m_visionUnit.GetVisionAlignProcessingModuleGUID()))
        m_visionUnit.RunInspection(m_visionUnit.GetVisionAlignProcessing(), false, GetCurVisionModule_Status());

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgVisionProcessingSideDetailAlign(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionProcessingSideDetailAlign::IDD, parent);

    m_pVisionInspDlg->ShowWindow(SW_SHOW);

    return 0;
}

void VisionProcessingSideDetailAlign::GetOverlayResult(
    VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (detailSetupMode)
    {
        overlayResult->OverlayReset();

        DrawInspectioOverlay(overlayResult, m_vecfptNormalAlignPoint);
        DrawInspectioOverlay(overlayResult, m_vecfptGlassAlignPoint);
        DrawInspectioOverlay(overlayResult, m_vecfptTopSubstrateAlignPoint);
        DrawInspectioOverlay(overlayResult, m_vecfptBottomSubstrateAlignPoint);
    }
}

bool VisionProcessingSideDetailAlign::DrawInspectioOverlay(
    VisionInspectionOverlayResult* overlayResult, const std::vector<Ipvm::Point32r2>& i_vecfptAlignPoint)
{
    const long nPointNum = (long)i_vecfptAlignPoint.size();
    if (nPointNum <= 0)
        return false;

    Ipvm::Polygon32r polygon;
    polygon.SetVertices(&i_vecfptAlignPoint[0], nPointNum);

    overlayResult->AddPolygon(polygon, RGB(0, 255, 0));

    return true;
}

void VisionProcessingSideDetailAlign::AppendTextResult(CString& textResult)
{
    textResult.AppendFormat(_T("\r\n< %s Result >\r\n"), (LPCTSTR)m_strModuleName);
    textResult.AppendFormat(_T("  [ Inspection Time : %.2f ]\r\n"), m_fCalcTime);
}

BOOL VisionProcessingSideDetailAlign::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    // Link Parameter
    if (!m_VisionPara->LinkDataBase(bSave, db[_T("{37BD7593-5FD2-4EE1-82CC-24BEBD2EE66A}")]))
        return FALSE;

    return TRUE;
}

std::vector<CString> VisionProcessingSideDetailAlign::ExportRecipeToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrResult(0);
    if (IsEnabled() == FALSE)
        return vecStrResult;

    const static CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;
    vecStrResult = m_VisionPara->ExportAlgoParaToText(strVisionName, m_strModuleName);

    return vecStrResult;
}