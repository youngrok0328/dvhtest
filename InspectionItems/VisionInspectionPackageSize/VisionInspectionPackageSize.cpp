//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionPackageSize.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionPackageSize.h"
#include "VisionInspectionPackageSizePara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
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
VisionInspectionPackageSize::VisionInspectionPackageSize(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionInspection(_VISION_INSP_GUID_PACKAGE_SIZE, _VISION_INSP_NAME_PACKAGE_SIZE, visionUnit, packageSpec)
    , m_VisionPara(new VisionInspectionPackageSizePara)
    , m_sEdgeAlignResult(new VisionAlignResult)
{
    m_pVisionInspDlg = NULL;

    m_pEdgeDetect = NULL;

    m_defaultFixedInspectionSpecs.emplace_back(_T("{CA6541E1-83CC-4E0E-843B-272E46811E14}"),
        g_szPackageSizeInspectionName[PACKAGE_SIZE_INSPECTION_BODYSIZE_X], _T("BODX"), _T("B"), _T("um"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{162EED2D-2C0E-4824-B69E-AB6EDFE700E7}"),
        g_szPackageSizeInspectionName[PACKAGE_SIZE_INSPECTION_BODYSIZE_Y], _T("BODY"), _T("B"), _T("um"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{8BED7E64-0C3C-4CC7-A9AC-8C4340EDC3F3}"),
        g_szPackageSizeInspectionName[PACKAGE_SIZE_INSPECTION_PARALLELISM], _T("bPAR"), _T("B"), _T("um"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{FD4A1D1C-04A6-46A5-96B9-25D5C985E81D}"),
        g_szPackageSizeInspectionName[PACKAGE_SIZE_INSPECTION_ORTHOGONALITY], _T("bOTH"), _T("B"), _T("Deg"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::DEGREE, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);

    __super::ResetSpec();
}

void VisionInspectionPackageSize::ResetSpecAndPara()
{
    __super::ResetSpec();

    m_VisionPara->Init();
}

VisionInspectionPackageSize::~VisionInspectionPackageSize(void)
{
    CloseDlg();

    delete m_sEdgeAlignResult;
    delete m_VisionPara;
}

void VisionInspectionPackageSize::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionInspectionPackageSize::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    m_visionUnit.RunInspection(m_visionUnit.GetPreviousVisionProcessing(this), false, GetCurVisionModule_Status());

    // 티칭에서 사용할거라서 필요함.
    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
        *m_sEdgeAlignResult = *(VisionAlignResult*)pData;

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgVisionInspectionPackageSize(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionInspectionPackageSize::IDD, parent);

    m_pVisionInspDlg->ShowWindow(SW_SHOW);

    return 0;
}

BOOL VisionInspectionPackageSize::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    // Link Parameter
    if (!m_VisionPara->LinkDataBase(bSave, db[_T("{54604D40-CFF2-4C73-A790-4446FD941F05}")]))
        return FALSE;

    return TRUE;
}

void VisionInspectionPackageSize::GetOverlayResult(
    VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (overlayResult == nullptr)
    {
        return;
    }

    if (detailSetupMode)
        overlayResult->OverlayReset();

    for (auto& spec : m_fixedInspectionSpecs)
    {
        if (spec.m_use)
        {
            auto* pResult = m_resultGroup.GetResultByName(spec.m_specName);

            if (m_nOverlayMode == OverlayType_ShowAll || m_nOverlayMode == OverlayType_Reject)
            {
                if (pResult->vecrtRejectROI.size())
                {
                    overlayResult->AddRectangles(pResult->vecrtRejectROI, RGB(255, 0, 0));

                    if (spec.m_useMarginal && pResult->vecrtMarginalROI.size())
                    {
                        overlayResult->AddRectangles(pResult->vecrtMarginalROI, RGB(243, 157, 58));
                    }
                }

                else if (pResult->vecrtMarginalROI.size())
                {
                    overlayResult->AddRectangles(pResult->vecrtMarginalROI, RGB(243, 157, 58));
                }
            }
        }
    }
}

void VisionInspectionPackageSize::AppendTextResult(CString& textResult)
{
    textResult.AppendFormat(_T("\r\n< %s Result >\r\n"), (LPCTSTR)m_strModuleName);
    textResult.AppendFormat(_T("  [ Inspection Time : %.2f ]\r\n"), m_fCalcTime);

    long nTotalResult(NOT_MEASURED);

    CString strAssertMsg;

    // Total Result...
    for (auto& spec : m_fixedInspectionSpecs)
    {
        if (spec.m_use)
        {
            auto* result = m_resultGroup.GetResultByName(spec.m_specName);
            if (result == nullptr)
            {
                strAssertMsg.Format(_T("VisionInspectionPackageSize::AppendTextResult() - %s result is nullptr"),
                    (LPCTSTR)spec.m_specName);
                ASSERT(strAssertMsg);
                continue;
            }
            if (result->m_totalResult > nTotalResult)
            {
                nTotalResult = result->m_totalResult;
            }
        }
    }

    textResult.AppendFormat(
        _T("    %s Total Result : %s\r\n"), (LPCTSTR)m_strModuleName, (LPCTSTR)Result2String(nTotalResult));

    for (auto& spec : m_fixedInspectionSpecs)
    {
        if (!spec.m_use)
            continue;

        auto* result = m_resultGroup.GetResultByName(spec.m_specName);
        if (result == nullptr)
        {
            strAssertMsg.Format(
                _T("VisionInspectionPackageSize::AppendTextResult() - %s result is nullptr"), (LPCTSTR)spec.m_specName);
            ASSERT((LPCTSTR)strAssertMsg);
            continue;
        }

        auto worstErrorValue = result->getObjectWorstErrorValue();

        if (worstErrorValue == Ipvm::k_noiseValue32r)
        {
            textResult.AppendFormat(_T("      %s [%s] = %s (Worst : ID [ %s ], Value [ INV ])\r\n"),
                (LPCTSTR)result->m_resultName, (LPCTSTR)spec.m_shortName, (LPCTSTR)Result2String(result->m_totalResult),
                (LPCTSTR)result->getObjectWorstID());
        }
        else
        {
            textResult.AppendFormat(_T("      %s [%s] = %s (Worst : ID [ %s ], Value [ %.2f ])\r\n"),
                (LPCTSTR)result->m_resultName, (LPCTSTR)spec.m_shortName, (LPCTSTR)Result2String(result->m_totalResult),
                (LPCTSTR)result->getObjectWorstID(), worstErrorValue);
        }
    }

    AppendDetailTextResult(textResult);
}

void VisionInspectionPackageSize::ResetResult()
{
    __super::ResetResult();

    m_bInvalid = FALSE;

    // Get Algorithm...
    m_pEdgeDetect = getReusableMemory().GetEdgeDetect();

    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
    {
        *m_sEdgeAlignResult = *(VisionAlignResult*)pData;
    }
    else
    {
        m_sEdgeAlignResult->bAvailable = FALSE;
    }

    for (auto& result : m_resultGroup.m_vecResult)
    {
        result.Resize(1);
    }
}

std::vector<CString> VisionInspectionPackageSize::ExportRecipeToText() //kircheis_TxtRecipe
{
    std::vector<CString> vecStrResult(0);
    if (IsEnabled() == FALSE)
        return vecStrResult;

    const static CString strVisionName = SystemConfig::GetInstance().m_strVisionInfo;
    vecStrResult = m_VisionPara->ExportAlgoParaToText(strVisionName, m_strModuleName);
    std::vector<CString> vecstrInspectionSpec = ExportInspectionSpecToText(strVisionName);

    vecStrResult.insert(vecStrResult.end(), vecstrInspectionSpec.begin(), vecstrInspectionSpec.end());

    return vecStrResult;
}