//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingPadAlign2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingPadAlign2D.h"
#include "Result.h"
#include "VisionProcessingPadAlign2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/EdgeDetection.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionProcessingPadAlign2D::VisionProcessingPadAlign2D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionProcessing(_VISION_INSP_GUID_PAD_ALIGN_2D, _T("Pad Align 2D"), visionUnit, packageSpec)
    , m_VisionPara(new VisionProcessingPadAlign2DPara(*this, packageSpec))
    , m_result(new Result)
    , m_pVisionInspDlg(nullptr)
{
    m_edgeDetection = new Ipvm::EdgeDetection;

    m_DebugInfoGroup.Add(_T("Body Center"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Spec PAD ROIs"), enumDebugInfoType::FPI_Rect);
    m_DebugInfoGroup.Add(_T("Edge Source ROIs"), enumDebugInfoType::FPI_Rect);
    m_DebugInfoGroup.Add(_T("Edge Detection Lines"), enumDebugInfoType::LineSeg_32f);
    m_DebugInfoGroup.Add(_T("Edge Points"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Align PAD Blobs"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Align PAD Ellipses"), enumDebugInfoType::EllipseEq_32f);
    m_DebugInfoGroup.Add(_T("Align PAD Polygons"), enumDebugInfoType::Polygon_32f);
    m_DebugInfoGroup.Add(_T("Align PAD Other"), enumDebugInfoType::FPI_Rect);
    m_DebugInfoGroup.Add(_T("Align PAD Center"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Align PAD Image"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("Apply PAD Align to Body Center"), enumDebugInfoType::Point_32f_C2);
}

void VisionProcessingPadAlign2D::ResetSpecAndPara()
{
    m_VisionPara->Init();
}

void VisionProcessingPadAlign2D::ResetResult()
{
    ResetDebugInfo();

    // Get Algorithm...
    m_pEdgeDetect = getReusableMemory().GetEdgeDetect();
    m_pBlob = getReusableMemory().GetBlob();
    m_psBlobInfo = getReusableMemory().GetBlobInfo();

    m_result->Init();
}

VisionProcessingPadAlign2D::~VisionProcessingPadAlign2D(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    delete m_result;
    delete m_VisionPara;
    delete m_edgeDetection;
}

void VisionProcessingPadAlign2D::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionProcessingPadAlign2D::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();
    m_visionUnit.RunInspection(m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_PAD_ALIGN_2D));

    checkJobParameter();
    DoInspection(true); // UI 표시를 위해 현재계산도 그냥 치뤄주자

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new DlgVisionProcessingPadAlign2D(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(DlgVisionProcessingPadAlign2D::IDD, parent);

    m_pVisionInspDlg->ShowWindow(SW_SHOW);
    m_pVisionInspDlg->SetInitParaWindow();
    //	m_pVisionInspDlg->ShowImage();

    return 0;
}

BOOL VisionProcessingPadAlign2D::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
    {
        return FALSE;
    }

    if (!m_VisionPara->LinkDataBase(bSave, db[_T("{167F48D4-E05C-4D5B-AE15-FE3DFD7C36B9}")]))
    {
        return FALSE;
    }

    if (!bSave)
    {
        checkJobParameter();
    }

    return TRUE;
}

void VisionProcessingPadAlign2D::GetOverlayResult(
    VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (detailSetupMode)
    {
        overlayResult->OverlayReset();
        for (auto& object : m_result->m_debugResult_align_Blobs)
        {
            overlayResult->AddRectangle(object, RGB(0, 255, 0));
        }
        for (auto& object : m_result->m_debugResult_align_Ellipses)
        {
            overlayResult->AddEllipse(object, RGB(0, 255, 0));
        }

        for (auto& object : m_result->m_debugResult_align_Polygons)
        {
            overlayResult->AddPolygon(object, RGB(0, 255, 0));
        }

        overlayResult->AddCrPoint(m_result->m_applyAlignTobodyCenter, RGB(0, 255, 0));
    }
}

void VisionProcessingPadAlign2D::AppendTextResult(CString& textResult)
{
    textResult.AppendFormat(_T("\r\n< %s Result >\r\n"), (LPCTSTR)m_strModuleName);
    textResult.AppendFormat(_T("  [ Inspection Time : %.2f ]\r\n"), m_fCalcTime);

    if (m_bInvalid)
    {
        textResult.AppendFormat(_T("  Pad Align Invalid\r\n"));
        textResult += m_errorLogText;
    }
}

std::vector<CString> VisionProcessingPadAlign2D::ExportRecipeToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrResult(0);
    if (IsEnabled() == FALSE)
        return vecStrResult;

    const static CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;
    vecStrResult = m_VisionPara->ExportAlgoParaToText(strVisionName, m_strModuleName);

    return vecStrResult;
}