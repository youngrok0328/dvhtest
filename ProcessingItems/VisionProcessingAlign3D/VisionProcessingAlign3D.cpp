//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingAlign3D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingAlign3D.h"
#include "MergeResult.h"
#include "Result.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"
#include "../../VisionNeedLibrary/VisionEdgeAlign/Inspection.h"
#include "../../VisionNeedLibrary/VisionEdgeAlign/Para.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionProcessingAlign3D::VisionProcessingAlign3D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionProcessing(_VISION_INSP_GUID_ALIGN_3D, _T("Align 3D"), visionUnit, packageSpec)
    , m_3DMergeResult(new S3DMergeResult)
    , m_result(new Result)
{
    m_pVisionInspDlg = NULL;

    m_fCalcTime = 0.f;

    m_DebugInfoGroup.Add(_T("Raw Package Rect"), enumDebugInfoType::FPI_Rect);
    m_DebugInfoGroup.Add(_T("Rough Matching Line"), enumDebugInfoType::LineSeg_32f);
    m_DebugInfoGroup.Add(_T("2D Matching ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("2D Matching Line"), enumDebugInfoType::LineSeg_32f);
    m_DebugInfoGroup.Add(_T("Overlap ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Overlap Matching ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Spec Object ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Search Object ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Object ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Object Center"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Measure ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Cross Line"), enumDebugInfoType::LineSeg_32f);
    m_DebugInfoGroup.Add(_T("Pane ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Device Rect"), enumDebugInfoType::Rect);

    m_edgeAlign_para = new VisionEdgeAlign::Para(*this);
    m_edgeAlign_inspection = new VisionEdgeAlign::Inspection(*this);
}

void VisionProcessingAlign3D::ResetSpecAndPara()
{
    m_edgeAlign_para->Init();
}

void VisionProcessingAlign3D::ResetResult()
{
    m_edgeAlign_inspection->resetResult();
    m_errorLog.Empty();
    ResetDebugInfo();

    const float bodySizeX_px = getScale().convert_umToPixelX(m_packageSpec.m_bodyInfoMaster->fBodySizeX);
    const float bodySizeY_px = getScale().convert_umToPixelY(m_packageSpec.m_bodyInfoMaster->fBodySizeY);

    float inspMargeX = getScale().convert_umToPixelX(10000.f);
    float inspMargeY = getScale().convert_umToPixelY(10000.f);
    long inspSizeX = (long(bodySizeX_px + inspMargeX) / 4) * 4; // 양쪽을 균일하게 만들기 편하게 4의 배수로 만들자
    long inspSizeY = (long(bodySizeY_px + inspMargeY) / 4) * 4; // 양쪽을 균일하게 만들기 편하게 4의 배수로 만들자

    getReusableMemory().SetInspImageSize(inspSizeX, inspSizeY);
}

VisionProcessingAlign3D::~VisionProcessingAlign3D(void)
{
    CloseDlg();

    delete m_3DMergeResult;
    delete m_edgeAlign_inspection;
    delete m_edgeAlign_para;
    delete m_result;
}

void VisionProcessingAlign3D::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionProcessingAlign3D::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();
    ResetResult();

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgVisionProcessingAlign3D(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionProcessingAlign3D::IDD, parent);

    m_pVisionInspDlg->ShowWindow(SW_SHOW);

    return 0;
}

void VisionProcessingAlign3D::GetOverlayResult(VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (overlayResult == NULL || detailSetupMode == FALSE)
        return;

    if (detailSetupMode)
        overlayResult->OverlayReset();

    m_edgeAlign_inspection->getOverlayResult(*m_edgeAlign_para, overlayResult, m_nOverlayMode, detailSetupMode);
}

void VisionProcessingAlign3D::AppendTextResult(CString& textResult)
{
    textResult.AppendFormat(_T("\r\n< %s Result >\r\n"), (LPCTSTR)m_strModuleName);
    textResult.AppendFormat(_T("  [ Inspection Time : %.2f ]\r\n"), m_fCalcTime);

    if (!m_errorLog.IsEmpty())
    {
        textResult.AppendFormat(_T("[ERROR LOG] %s"), LPCTSTR(m_errorLog));
    }

    m_edgeAlign_inspection->appendTextResult(textResult);
}

BOOL VisionProcessingAlign3D::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    if (!m_VisionPara.LinkDataBase(bSave, m_packageSpec.m_bodyInfoMaster->GetBodySizeX(),
            m_packageSpec.m_bodyInfoMaster->GetBodySizeY(), db[_T("{03610B73-0407-4125-8C21-B310F654A894}")]))
        return FALSE;

    if (!m_edgeAlign_para->LinkDataBase(bSave, m_packageSpec.m_bodyInfoMaster->GetBodySizeX(),
            m_packageSpec.m_bodyInfoMaster->GetBodySizeY(), db[_T("{2EAF49EA-878D-4EB4-AE0A-E22272C8E9DB}")]))
        return FALSE;

    return TRUE;
}

std::vector<CString> VisionProcessingAlign3D::ExportRecipeToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrResult(0);
    if (IsEnabled() == FALSE)
        return vecStrResult;

    const static CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;
    vecStrResult = m_VisionPara.ExportAlgoParaToText(strVisionName, m_strModuleName);
    std::vector<CString> vecstrEdgeAlignPara
        = m_edgeAlign_para->ExportAlgoParaToText(strVisionName, m_strModuleName); //만들어야 함.
    vecStrResult.insert(vecStrResult.end(), vecstrEdgeAlignPara.begin(), vecstrEdgeAlignPara.end());

    return vecStrResult;
}