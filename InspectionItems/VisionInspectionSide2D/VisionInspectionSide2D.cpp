//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionSide2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionSide2D.h"
#include "VisionInspectionSide2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
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
VisionInspectionSide2D::VisionInspectionSide2D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionInspection(
          _VISION_INSP_GUID_SIDE_INSPECTION_2D, _VISION_INSP_NAME_SIDE_INSPECTION_2D, visionUnit, packageSpec)
    , m_VisionPara(new VisionInspectionSide2DPara)
    , m_sEdgeAlignResult(new VisionAlignResult)
{
    m_pVisionInspDlg = NULL;

    m_pEdgeDetect = NULL;

    m_defaultFixedInspectionSpecs.emplace_back(_T("{F1E259E4-4FAF-4232-8FA8-5CB7EE1B66FC}"),
        g_szSide2DInspectionName[SIDE_INSPECTION_2D_TOTAL_THICKNESS], _T("STH"), _T("Si"), _T("um"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{FD8F59FC-7D2C-4C2A-B3FB-A2E16A577960}"),
        g_szSide2DInspectionName[SIDE_INSPECTION_2D_GLASS_THICKNESS], _T("SGTH"), _T("Si"), _T("um"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{E46CC16D-1721-4E9D-8511-56BFCFBE49FE}"),
        g_szSide2DInspectionName[SIDE_INSPECTION_2D_TOP_SUBSTRATE_THICKNESS], _T("STTH"), _T("Si"), _T("um"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{795351D3-3DE7-47C6-8A96-60B8935A0E8A}"),
        g_szSide2DInspectionName[SIDE_INSPECTION_2D_BOTTOM_SUBSTRATE_THICKNESS], _T("SBTH"), _T("Si"), _T("um"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{25EFCE33-4C12-43FA-A25B-82045F2A1FDC}"),
        g_szSide2DInspectionName[SIDE_INSPECTION_2D_SIDE_WARPAGE], _T("SW"), _T("Si"), _T("um"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::DEGREE, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);

    __super::ResetSpec();

    //이하는 이 모듈이 직접 뽑는 Data가 아닌 Side Detail Align에서 뽑은 Data를 넣는다. 단 Detail Setup에서만 확인 가능하게 한다. 추가 사유는 직접적으로 위치 정보 확인을 통한 Debug를 위함이다.
    m_DebugInfoGroup.Add(_T("Side-Detail Align Result"),
        enumDebugInfoType::Point_32f_C2); // Top Substrate Top Point + Bottom Substrate Bottom Point + Glass Side Point
    m_DebugInfoGroup.Add(_T("Side-Top Substrate Align Result"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Side-Bottom Substrate Align Result"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Side-Glass Align Result"), enumDebugInfoType::Point_32f_C2);
}

void VisionInspectionSide2D::ResetSpecAndPara()
{
    __super::ResetSpec();

    m_VisionPara->Init();
}

VisionInspectionSide2D::~VisionInspectionSide2D(void)
{
    CloseDlg();

    delete m_sEdgeAlignResult;
    delete m_VisionPara;
}

void VisionInspectionSide2D::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionInspectionSide2D::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    if (m_visionUnit.IsTheUsingVisionProcessingByGuid(_VISION_INSP_GUID_SIDE_DETAIL_ALIGN) == FALSE)
        ::SimpleMessage(_T("This inspection module requires [Side Detail Align]."));

    m_visionUnit.RunInspection(m_visionUnit.GetPreviousVisionProcessing(this), false, GetCurVisionModule_Status());

    // 티칭에서 사용할거라서 필요함.
    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
        *m_sEdgeAlignResult = *(VisionAlignResult*)pData;

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgVisionInspectionSide2D(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionInspectionSide2D::IDD, parent);

    m_pVisionInspDlg->ShowWindow(SW_SHOW);

    return 0;
}

BOOL VisionInspectionSide2D::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (bSave)
        ResetNormalModelInspectionItem();

    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    if (bSave == false)
    {
        m_strModuleName = _VISION_INSP_NAME_SIDE_INSPECTION_2D;
        ResetNormalModelInspectionItem();
    }
    // Link Parameter
    if (!m_VisionPara->LinkDataBase(bSave, db[_T("{F8A40120-A4D7-47E0-9F8B-3AB4771720A3}")]))
        return FALSE;

    return TRUE;
}

void VisionInspectionSide2D::GetOverlayResult(VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
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

void VisionInspectionSide2D::AppendTextResult(CString& textResult)
{
    textResult.AppendFormat(_T("\r\n< %s Result >\r\n"), (LPCTSTR)m_strModuleName);
    textResult.AppendFormat(_T("  [ Inspection Time : %.2f ]\r\n"), m_fCalcTime);

    long nTotalResult(NOT_MEASURED);

    // Total Result...
    for (auto& spec : m_fixedInspectionSpecs)
    {
        if (spec.m_use)
        {
            auto* result = m_resultGroup.GetResultByName(spec.m_specName);
            if (result != nullptr && result->m_totalResult > nTotalResult)
            {
                nTotalResult = result->m_totalResult;
            }
        }
    }

    textResult.AppendFormat(
        _T("    %s Total Result : %s\r\n"), (LPCTSTR)m_strModuleName, (LPCTSTR)Result2String(nTotalResult));

    CString strAssertMsg;

    for (auto& spec : m_fixedInspectionSpecs)
    {
        if (!spec.m_use)
            continue;

        auto* result = m_resultGroup.GetResultByName(spec.m_specName);
        if (result == nullptr)
        {
            strAssertMsg.Format(
                _T("VisionInspectionSide2D::AppendTextResult() - %ls result is nullptr"), (LPCTSTR)spec.m_specName);
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

void VisionInspectionSide2D::ResetResult()
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

std::vector<CString> VisionInspectionSide2D::ExportRecipeToText() //kircheis_TxtRecipe
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