//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionOTITR.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionOTITR.h"
#include "VisionInspectionOTITRPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_EdgeDetect/dPI_EdgeDetectLib.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/SprocketHoleInfo.h"
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
VisionInspectionOTITR::VisionInspectionOTITR(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionInspection(_VISION_INSP_GUID_OTI_TR, _VISION_INSP_NAME_TR_OTI, visionUnit, packageSpec)
    , m_VisionPara(new VisionInspectionOTITRPara(*this))
    , m_sEdgeAlignResult(new VisionAlignResult)
    , m_alignedFillPocket(false)
    , m_receivedFillPocket(false)
    , m_rtPocket(0, 0, 0, 0)
    , m_fCalcTime(0.f)
{
    m_pVisionInspDlg = NULL;

    m_pEdgeDetect = getReusableMemory().GetEdgeDetect();

    m_sprocketHoleInfo = new SprocketHoleInfo;

    m_defaultFixedInspectionSpecs.emplace_back(_INSP_ITEM_GUID_OTI_EMPTY_STATUS,
        g_szOTIInspectionName[OTI_INSPECTION_EMPTY_STATUS], _T("OTEP"), _T("T"), _T(""), HostReportCategory::MEASURE,
        II_RESULT_TYPE::COUNT, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);

    m_defaultFixedInspectionSpecs.emplace_back(_INSP_ITEM_GUID_OTI_COVER_TAPE_SHIFT,
        g_szOTIInspectionName[OTI_INSPECTION_COVER_TAPE_SHIFT], _T("OTCS"), _T("T"), _T("um"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);

    m_defaultFixedInspectionSpecs.emplace_back(_INSP_ITEM_GUID_OTI_SEALING_SHIFT,
        g_szOTIInspectionName[OTI_INSPECTION_SEALING_SHIFT], _T("OTSS"), _T("T"), _T("um"), HostReportCategory::MEASURE,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);

    m_defaultFixedInspectionSpecs.emplace_back(_INSP_ITEM_GUID_OTI_SEALING_WIDTH,
        g_szOTIInspectionName[OTI_INSPECTION_SEALING_WIDTH], _T("OTSW"), _T("T"), _T("um"), HostReportCategory::MEASURE,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);

    m_defaultFixedInspectionSpecs.emplace_back(_INSP_ITEM_GUID_OTI_SEALING_SPAN,
        g_szOTIInspectionName[OTI_INSPECTION_SEALING_SPAN], _T("OPSP"), _T("T"), _T("um"), HostReportCategory::MEASURE,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);

    //m_defaultFixedInspectionSpecs.emplace_back(_INSP_ITEM_GUID_OTI_SEALING_CUTTING,
    //    g_szOTIInspectionName[OTI_INSPECTION_SEALING_CUTTING], _T("OTSC"), _T("T"), _T("um"),
    //    HostReportCategory::MEASURE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);

    m_defaultFixedInspectionSpecs.emplace_back(_INSP_ITEM_GUID_OTI_SEALING_PARALLELISM,
        g_szOTIInspectionName[OIT_INSPECTION_SEALING_PARALLELISM], _T("OTSP"), _T("T"), _T("um"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);


    m_DebugInfoGroup.Add(_DEBUG_INFO_OTI_COVER_TAPE_SEARCH_ROI, enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_DEBUG_INFO_OTI_COVER_TAPE_LEFT_EDGE_POINT, enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_DEBUG_INFO_OTI_COVER_TAPE_RIGHT_EDGE_POINT, enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_DEBUG_INFO_OTI_COVER_TAPE_EDGE_LEFT_LINE, enumDebugInfoType::LineSeg_32f);
    m_DebugInfoGroup.Add(_DEBUG_INFO_OTI_COVER_TAPE_EDGE_RIGHT_LINE, enumDebugInfoType::LineSeg_32f);
    m_DebugInfoGroup.Add(_DEBUG_INFO_OTI_COVER_TAPE_ALIGN_QRECT, enumDebugInfoType::Quadrangle_32f);
    m_DebugInfoGroup.Add(_DEBUG_INFO_OTI_SEALING_SEARCH_ROI, enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_DEBUG_INFO_OTI_LEFT_SEALING_LEFT_EDGE_POINT, enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_DEBUG_INFO_OTI_LEFT_SEALING_RIGHT_EDGE_POINT, enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_DEBUG_INFO_OTI_RIGHT_SEALING_LEFT_EDGE_POINT, enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_DEBUG_INFO_OTI_RIGHT_SEALING_RIGHT_EDGE_POINT, enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_DEBUG_INFO_OTI_SEALING_LINE, enumDebugInfoType::LineSeg_32f);
    m_DebugInfoGroup.Add(_DEBUG_INFO_OTI_SEALING_QRECT, enumDebugInfoType::Quadrangle_32f);

    __super::ResetSpec();
}

void VisionInspectionOTITR::ResetSpecAndPara()
{
    __super::ResetSpec();

    m_VisionPara->Init();
}

VisionInspectionOTITR::~VisionInspectionOTITR(void)
{
    CloseDlg();

    delete m_sprocketHoleInfo;
    delete m_sEdgeAlignResult;
    delete m_VisionPara;
}

void VisionInspectionOTITR::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionInspectionOTITR::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    m_visionUnit.RunInspection(m_visionUnit.GetPreviousVisionProcessing(this), false, GetCurVisionModule_Status());

    Ipvm::Image8u image = GetInspectionFrameImage();
    CollectAlignInfoInDebugInfo(Ipvm::Point32r2((float)image.GetSizeX() * 0.5f, (float)image.GetSizeY() * 0.5f));
    
    // 티칭에서 사용할거라서 필요함.
    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
        *m_sEdgeAlignResult = *(VisionAlignResult*)pData;

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgVisionInspectionOTITR(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionInspectionOTITR::IDD, parent);

    m_pVisionInspDlg->ShowWindow(SW_SHOW);

    return 0;
}

BOOL VisionInspectionOTITR::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    // Link Parameter
    if (!m_VisionPara->LinkDataBase(bSave, db[_T("{54604D40-CFF2-4C73-A790-4446FD941F05}")]))
        return FALSE;

    return TRUE;
}

void VisionInspectionOTITR::GetOverlayResult(VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
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

void VisionInspectionOTITR::AppendTextResult(CString& textResult)
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
                strAssertMsg.Format(
                    _T("VisionInspectionOTITR::AppendTextResult() - %s result is nullptr"), (LPCTSTR)spec.m_specName);
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
                _T("VisionInspectionOTITR::AppendTextResult() - %s result is nullptr"), (LPCTSTR)spec.m_specName);
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

void VisionInspectionOTITR::ResetResult()
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

std::vector<CString> VisionInspectionOTITR::ExportRecipeToText() //kircheis_TxtRecipe
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