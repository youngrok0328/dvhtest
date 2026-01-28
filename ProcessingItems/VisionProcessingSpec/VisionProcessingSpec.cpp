//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingSpec.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingSpec.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionProcessingSpec::VisionProcessingSpec(
    LPCTSTR moduleGuid, LPCTSTR moduleName, VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionProcessing(moduleGuid, moduleName, visionUnit, packageSpec)
{
    m_pVisionInspDlg = NULL;
}

void VisionProcessingSpec::SetModuleName(CString strName)
{
    m_strModuleName = strName;
}

void VisionProcessingSpec::ResetSpecAndPara()
{
}

void VisionProcessingSpec::ResetResult()
{
}

VisionProcessingSpec::~VisionProcessingSpec(void)
{
    CloseDlg();
}

void VisionProcessingSpec::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionProcessingSpec::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgVisionProcessingSpec(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionProcessingSpec::IDD, parent);

    m_pVisionInspDlg->ShowWindow(SW_SHOW);

    return 0;
}

void VisionProcessingSpec::GetOverlayResult(
    VisionInspectionOverlayResult* overlayResult, const bool /*detailSetupMode*/)
{
    if (overlayResult == NULL)
        return;
}

void VisionProcessingSpec::AppendTextResult(CString& /*textResult*/)
{
}

BOOL VisionProcessingSpec::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    if (!m_VisionPara.LinkDataBase(bSave, db[_T("{CFE7795D-3A61-4BF8-A971-944A2848C340}")]))
        return FALSE;

    return TRUE;
}

std::vector<CString> VisionProcessingSpec::ExportRecipeToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrResult(0);
    if (IsEnabled() == FALSE)
        return vecStrResult;

    //VisionProcessingSpec은 Package Spec의 Size 정보 및 Dead Bug 여부를 저장하자

    const static CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;
    const static CString strInspectionModuleName = m_strModuleName;
    CString strHeader;
    strHeader.Format(_T("%s,%s"), (LPCTSTR)strVisionName, (LPCTSTR)strInspectionModuleName);

    vecStrResult.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, _T("Pacakge Spec"), _T("Device"),
        _T("Body Size X"), _T(""), m_packageSpec.m_bodyInfoMaster->fBodySizeX));
    vecStrResult.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, _T("Pacakge Spec"), _T("Device"),
        _T("Body Size Y"), _T(""), m_packageSpec.m_bodyInfoMaster->fBodySizeY));
    vecStrResult.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, _T("Pacakge Spec"), _T("Device"),
        _T("Body Size Z"), _T(""), m_packageSpec.m_bodyInfoMaster->fBodyThickness));
    vecStrResult.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, _T("Pacakge Spec"), _T("Device"), _T("Dead Bug"), _T(""), (bool)m_packageSpec.m_deadBug));

    //{{//kircheis_SideInsp
    vecStrResult.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, _T("Pacakge Spec"), _T("Device"),
        _T("Glass Core Package"), _T(""), (bool)m_packageSpec.m_bodyInfoMaster->m_bGlassCorePackage));
    vecStrResult.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, _T("Pacakge Spec"), _T("Device"),
        _T("Glass Core Thickness"), _T(""), m_packageSpec.m_bodyInfoMaster->m_fGlassCoreThickness));
    vecStrResult.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, _T("Pacakge Spec"), _T("Device"),
        _T("Glass Core Substrate Size X"), _T(""), m_packageSpec.m_bodyInfoMaster->m_fGlassCoreSubstrateSizeX));
    vecStrResult.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, _T("Pacakge Spec"), _T("Device"),
        _T("Glass Core Substrate Size Y"), _T(""), m_packageSpec.m_bodyInfoMaster->m_fGlassCoreSubstrateSizeY));
    vecStrResult.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, _T("Pacakge Spec"), _T("Device"),
        _T("Glass Core Top Substrate Thickness"), _T(""),
        m_packageSpec.m_bodyInfoMaster->m_fGlassCoreTopSubstrateThickness));
    vecStrResult.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, _T("Pacakge Spec"), _T("Device"),
        _T("Glass Core Bottom Substrate Thickness"), _T(""),
        m_packageSpec.m_bodyInfoMaster->m_fGlassCoreBottomSubstrateThickness));
    //}}
    return vecStrResult;
}