//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingFOV.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingFOV.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h" //kircheis_TxtRecipe
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionProcessingFOV::VisionProcessingFOV(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionProcessing(_VISION_INSP_GUID_FOV, _T("F.O.V. Editor"), visionUnit, packageSpec)
{
    m_pVisionInspDlg = NULL;

    m_DebugInfoGroup.Add(_T("Device Rect"), enumDebugInfoType::Rect);
}

void VisionProcessingFOV::ResetSpecAndPara()
{
}

void VisionProcessingFOV::ResetResult()
{
}

VisionProcessingFOV::~VisionProcessingFOV(void)
{
    CloseDlg();
}

void VisionProcessingFOV::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionProcessingFOV::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();
    ResetDebugInfo();

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgVisionProcessingFOV(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionProcessingFOV::IDD, parent);

    m_pVisionInspDlg->ShowWindow(SW_SHOW);

    return 0;
}

void VisionProcessingFOV::GetOverlayResult(VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    UNREFERENCED_PARAMETER(detailSetupMode);
    UNREFERENCED_PARAMETER(overlayResult);
}

void VisionProcessingFOV::AppendTextResult(CString& /*textResult*/)
{
}

BOOL VisionProcessingFOV::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    return TRUE;
}

std::vector<CString> VisionProcessingFOV::ExportRecipeToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrResult(0);
    if (IsEnabled() == FALSE)
        return vecStrResult;

    //VisionProcessingFOV는 Pane Offset을 저장하자

    const static CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;
    const static CString strInspectionModuleName = m_strModuleName;
    CString strHeader;
    strHeader.Format(_T("%s,%s"), (LPCTSTR)strVisionName, (LPCTSTR)strInspectionModuleName);

    vecStrResult.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, _T("Tray Spec"),
        _T("Field of view information"), _T("Pane offset x"), _T(""), getTrayScanSpec().GetHorPaneOffset_um()));
    vecStrResult.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, _T("Tray Spec"),
        _T("Field of view information"), _T("Pane offset y"), _T(""), getTrayScanSpec().GetVerPaneOffset_um()));

    return vecStrResult;
}