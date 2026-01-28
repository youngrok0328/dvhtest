//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingFiducialAlign.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingFiducialAlign.h"
#include "Result.h"
#include "VisionProcessingFiducialAlignPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
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
VisionProcessingFiducialAlign::VisionProcessingFiducialAlign(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionProcessing(_VISION_INSP_GUID_FIDUCIAL_ALIGN, _T("Fiducial Align"), visionUnit, packageSpec)
    , m_VisionPara(new VisionProcessingFiducialAlignPara(*this))
    , m_result(new Result)
    , m_pVisionInspDlg(nullptr)
{
    m_DebugInfoGroup.Add(_T("Body Center"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Spec PAD ROIs"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("PAD Search ROIs"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Final Align Result ROIs"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Pad Blob Image"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("Apply PAD Align to Body Center"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Apply PAD Align to Body ROI"), enumDebugInfoType::FPI_Rect);
}

void VisionProcessingFiducialAlign::ResetSpecAndPara()
{
    m_VisionPara->Init();
}

void VisionProcessingFiducialAlign::ResetResult()
{
    ResetDebugInfo();

    // Get Algorithm...
    m_pEdgeDetect = getReusableMemory().GetEdgeDetect();
    m_pBlob = getReusableMemory().GetBlob();
    m_psBlobInfo = getReusableMemory().GetBlobInfo();

    m_result->Init();
}

VisionProcessingFiducialAlign::~VisionProcessingFiducialAlign(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    delete m_result;
    delete m_VisionPara;
}

void VisionProcessingFiducialAlign::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionProcessingFiducialAlign::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();
    m_visionUnit.RunInspection(m_visionUnit.GetVisionAlignProcessing(), false, GetCurVisionModule_Status());

    checkJobParameter();
    DoInspection(true); // UI 표시를 위해 현재계산도 그냥 치뤄주자

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new DlgVisionProcessingFiducialAlign(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(DlgVisionProcessingFiducialAlign::IDD, parent);

    m_pVisionInspDlg->ShowWindow(SW_SHOW);
    m_pVisionInspDlg->SetInitParaWindow();
    //	m_pVisionInspDlg->ShowImage();

    return 0;
}

BOOL VisionProcessingFiducialAlign::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
    {
        return FALSE;
    }

    if (!m_VisionPara->LinkDataBase(bSave, db[_T("{6679034A-9672-4962-9B8F-61F246E4B52F}")]))
    {
        return FALSE;
    }

    if (!bSave)
    {
        checkJobParameter();
    }

    return TRUE;
}

void VisionProcessingFiducialAlign::GetOverlayResult(
    VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (detailSetupMode)
    {
        overlayResult->OverlayReset();
        //for (auto& object : m_result->m_debugResult_align_Ellipses)
        //{
        //	overlayResult->AddEllipse(object, RGB(0, 255, 0));
        //}
        //
        //for (auto& object : m_result->m_debugResult_align_Polygons)
        //{
        //	overlayResult->AddPolygon(object, RGB(0, 255, 0));
        //}

        overlayResult->AddCrPoint(m_result->m_applyAlignTobodyCenter, RGB(0, 255, 0));
    }
}

void VisionProcessingFiducialAlign::AppendTextResult(CString& textResult)
{
    textResult.AppendFormat(_T("\r\n< %s Result >\r\n"), (LPCTSTR)m_strModuleName);
    textResult.AppendFormat(_T("  [ Inspection Time : %.2f ]\r\n"), m_fCalcTime);

    if (m_bInvalid)
    {
        textResult.AppendFormat(_T("  Fiducial Align Invalid\r\n"));
        textResult += m_errorLogText;
    }
}

std::vector<CString> VisionProcessingFiducialAlign::ExportRecipeToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrResult(0);
    if (IsEnabled() == FALSE)
        return vecStrResult;

    const static CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;
    vecStrResult = m_VisionPara->ExportAlgoParaToText(strVisionName, m_strModuleName);

    return vecStrResult;
}