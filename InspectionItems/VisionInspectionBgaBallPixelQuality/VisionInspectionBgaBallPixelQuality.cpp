//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionBgaBallPixelQuality.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionBgaBallPixelQuality.h"
#include "VisionInspectionBgaBallPixelQualityPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
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
static LPCTSTR g_szFillRatioGuid[] = {
    _T("{1FB94434-E670-4225-880F-1486ECBE316B}"),
    _T("{3D840B7C-FBD0-4321-8CFC-CC5E0B2DB97E}"),
    _T("{A32A3B04-9B6D-4658-A5D9-0C29A5508018}"),
    _T("{22A175EA-1367-4EF3-91DC-1448D8781BA3}"),
    _T("{BB6DE7F5-CE9A-4896-A75E-F1DDA173F8A0}"),
};

static LPCTSTR g_szContrastGuid[] = {
    _T("{159CF4B0-5F46-46D7-BB64-33016A776AD3}"),
    _T("{5F6891B5-C90D-4110-B7B7-20C1347023F2}"),
    _T("{44D4539C-430D-49B0-9296-E12C8A36F572}"),
    _T("{1DEEE0BA-6F09-4E16-B19A-230A80B8458B}"),
    _T("{60387BE8-91C7-4CD3-81C9-87BAE82ED812}"),
};

VisionInspectionBgaBallPixelQuality::VisionInspectionBgaBallPixelQuality(
    VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionInspection(_VISION_INSP_GUID_BGA_BALL_PQ, _VISION_INSP_NAME_BGA_BALL_PQ, visionUnit, packageSpec)
    , m_VisionPara(new VisionInspectionBgaBallPixelQualityPara(*this))
{
    m_pVisionInspDlg = NULL;

    m_defaultFixedInspectionSpecs.emplace_back(g_szFillRatioGuid[0],
        gl_szStrInspectPixelQualityItem[INSPECT_BALL_PIXEL_QUALITY_ITEM_FILLRATIO_1], _T("PQF1"), _T("Q"), _T("%"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::PERCENT, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(g_szFillRatioGuid[1],
        gl_szStrInspectPixelQualityItem[INSPECT_BALL_PIXEL_QUALITY_ITEM_FILLRATIO_2], _T("PQF2"), _T("Q"), _T("%"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::PERCENT, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(g_szFillRatioGuid[2],
        gl_szStrInspectPixelQualityItem[INSPECT_BALL_PIXEL_QUALITY_ITEM_FILLRATIO_3], _T("PQF3"), _T("Q"), _T("%"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::PERCENT, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(g_szFillRatioGuid[3],
        gl_szStrInspectPixelQualityItem[INSPECT_BALL_PIXEL_QUALITY_ITEM_FILLRATIO_4], _T("PQF4"), _T("Q"), _T("%"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::PERCENT, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(g_szFillRatioGuid[4],
        gl_szStrInspectPixelQualityItem[INSPECT_BALL_PIXEL_QUALITY_ITEM_FILLRATIO_5], _T("PQF5"), _T("Q"), _T("%"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::PERCENT, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(g_szContrastGuid[0],
        gl_szStrInspectPixelQualityItem[INSPECT_BALL_PIXEL_QUALITY_ITEM_CONTRAST_1], _T("PQC1"), _T("Q"), _T("GV"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::GRAY_VALUE, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(g_szContrastGuid[1],
        gl_szStrInspectPixelQualityItem[INSPECT_BALL_PIXEL_QUALITY_ITEM_CONTRAST_2], _T("PQC2"), _T("Q"), _T("GV"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::GRAY_VALUE, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(g_szContrastGuid[2],
        gl_szStrInspectPixelQualityItem[INSPECT_BALL_PIXEL_QUALITY_ITEM_CONTRAST_3], _T("PQC3"), _T("Q"), _T("GV"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::GRAY_VALUE, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(g_szContrastGuid[3],
        gl_szStrInspectPixelQualityItem[INSPECT_BALL_PIXEL_QUALITY_ITEM_CONTRAST_4], _T("PQC4"), _T("Q"), _T("GV"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::GRAY_VALUE, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(g_szContrastGuid[4],
        gl_szStrInspectPixelQualityItem[INSPECT_BALL_PIXEL_QUALITY_ITEM_CONTRAST_5], _T("PQC5"), _T("Q"), _T("GV"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::GRAY_VALUE, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);

    __super::ResetSpec();

    m_DebugInfoGroup.Add(_T("1st Ring Radius"), enumDebugInfoType::EllipseEq_32f);
    m_DebugInfoGroup.Add(_T("2nd Ring Radius"), enumDebugInfoType::EllipseEq_32f);
    m_DebugInfoGroup.Add(_T("3rd Ring Radius"), enumDebugInfoType::EllipseEq_32f);
    m_DebugInfoGroup.Add(_T("4th Ring Radius"), enumDebugInfoType::EllipseEq_32f);
    m_DebugInfoGroup.Add(_T("5th Ring Radius"), enumDebugInfoType::EllipseEq_32f);
    m_DebugInfoGroup.Add(_T("1st Ring BallID, Ring Area, Dark Pixel Area, Dark Ratio(%)"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("2nd Ring BallID, Ring Area, Dark Pixel Area, Dark Ratio(%)"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("3rd Ring BallID, Ring Area, Dark Pixel Area, Dark Ratio(%)"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("4th Ring BallID, Ring Area, Dark Pixel Area, Dark Ratio(%)"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("5th Ring BallID, Ring Area, Dark Pixel Area, Dark Ratio(%)"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("1st Ring Mask"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("1st Ring Bright Pixel"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("1st Ring Dark Pixel"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("2nd Ring Mask"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("2nd Ring Bright Pixel"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("2nd Ring Dark Pixel"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("3rd Ring Mask"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("3rd Ring Bright Pixel"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("3rd Ring Dark Pixel"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("4th Ring Mask"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("4th Ring Bright Pixel"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("4th Ring Dark Pixel"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("5th Ring Mask"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("5th Ring Bright Pixel"), enumDebugInfoType::Image_8u_C1);
    m_DebugInfoGroup.Add(_T("5th Ring Dark Pixel"), enumDebugInfoType::Image_8u_C1);
}

void VisionInspectionBgaBallPixelQuality::ResetSpecAndPara()
{
    __super::ResetSpec();

    m_VisionPara->Init();
}

VisionInspectionBgaBallPixelQuality::~VisionInspectionBgaBallPixelQuality(void)
{
    CloseDlg();

    delete m_VisionPara;
}

void VisionInspectionBgaBallPixelQuality::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionInspectionBgaBallPixelQuality::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    if (m_visionUnit.IsTheUsingVisionProcessingByGuid(_VISION_INSP_GUID_BGA_BALL_2D))
        m_visionUnit.RunInspection(
            m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_BGA_BALL_2D), false, GetCurVisionModule_Status());
    else
    {
        CString strError;
        strError.Format(_T("This module need execute the [%s] in the pre-process."),
            m_visionUnit.GetVisionProcessingModuleNameByGuid(_VISION_INSP_GUID_BGA_BALL_2D));
        SimpleMessage(strError);
        strError.Empty();
    }

    // 티칭에서 사용할거라서 필요함.
    //long nDataNum = 0;
    //void* pData = m_visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_EDGE_ALIGN, _T("EDGE Align Result"), nDataNum);
    //if (pData != nullptr && nDataNum > 0)
    //	*m_sEdgeAlignResult = *(VisionAlignResult *)pData;

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgVisionInspectionBgaBallPixelQuality(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionInspectionBgaBallPixelQuality::IDD, parent);

    //m_pVisionInspDlg->ShowImage();

    m_pVisionInspDlg->ShowWindow(SW_SHOW);

    return 0;
}

BOOL VisionInspectionBgaBallPixelQuality::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    // Link Parameter
    if (!m_VisionPara->LinkDataBase(bSave, db[_T("{32D5BC1A-8571-4174-99C3-CF9941825A3E}")]))
        return FALSE;

    return TRUE;
}

void VisionInspectionBgaBallPixelQuality::GetOverlayResult(
    VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (overlayResult == nullptr)
    {
        return;
    }

    if (detailSetupMode)
        overlayResult->OverlayReset();

    long nInspSpecNum = (long)m_fixedInspectionSpecs.size();
    CString strSpecName;
    for (long nSpec = 0; nSpec < nInspSpecNum; nSpec++)
    {
        if (m_fixedInspectionSpecs[nSpec].m_use)
        {
            strSpecName = m_fixedInspectionSpecs[nSpec].m_specName;
            VisionInspectionResult* pResult = m_resultGroup.GetResultByName(strSpecName);
            if (pResult != NULL)
            {
                if (pResult->vecrtRejectROI.size())
                {
                    overlayResult->AddRectangles(pResult->vecrtRejectROI, RGB(255, 0, 0));

                    if (m_fixedInspectionSpecs[nSpec].m_useMarginal && pResult->vecrtMarginalROI.size())
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
    strSpecName.Empty();
}
void VisionInspectionBgaBallPixelQuality::AppendTextResult(CString& textResult)
{
    textResult.AppendFormat(_T("\r\n< %s Result >\r\n"), (LPCTSTR)m_strModuleName);
    textResult.AppendFormat(_T("  [ Inspection Time : %.2f ]\r\n"), m_fCalcTime);

    long nInspNum = (long)m_fixedInspectionSpecs.size();
    long nResultNum = (long)m_resultGroup.m_vecResult.size();
    if (nInspNum != nResultNum)
    {
        ASSERT(!(_T("Check the inspection item's number")));
        return;
    }

    VisionInspectionResult* pResult = NULL;

    long nTotalResult(NOT_MEASURED);
    // Total Result...
    for (long nInsp = 0; nInsp < nInspNum; nInsp++)
    {
        if (m_fixedInspectionSpecs[nInsp].m_use)
        {
            auto* result = m_resultGroup.GetResultByName(m_fixedInspectionSpecs[nInsp].m_specName);
            if (result->m_totalResult > nTotalResult)
            {
                nTotalResult = result->m_totalResult;
            }
        }
    }

    textResult.AppendFormat(
        _T("    %s Total Result : %s\r\n"), (LPCTSTR)m_strModuleName, (LPCTSTR)Result2String(nTotalResult));

    for (long nInsp = 0; nInsp < nInspNum; nInsp++)
    {
        pResult = m_resultGroup.GetResultByName(m_fixedInspectionSpecs[nInsp].m_specName);
        if (pResult == NULL)
            continue;
        VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
        if (pSpec == NULL || !pSpec->m_use)
            continue;
        //strTemp.Format("Worst Value = %.2f\n", pResult->m_worstErrorValue);
        textResult.AppendFormat(
            _T("      %s = %s\r\n"), (LPCTSTR)pResult->m_resultName, (LPCTSTR)Result2String(pResult->m_totalResult));
    }

    long nObjectTotalNum = (long)m_packageSpec.m_ballMap->m_balls.size(); //Parameters for this module only.

    AppendDetailTextResult(textResult, nObjectTotalNum);
}

void VisionInspectionBgaBallPixelQuality::ResetResult()
{
    __super::ResetResult();

    // Get Algorithm...
    m_pBlob = getReusableMemory().GetBlob();

    m_vecrtBallCircleROI.clear();
}

CString VisionInspectionBgaBallPixelQuality::GetStringCount(long nCnt) //kircheis_BPQ
{
    CString str;
    if (nCnt <= 0)
        return _T("");
    else if (nCnt == 1)
        str.Format(_T("1st"));
    else if (nCnt == 2)
        str.Format(_T("2nd"));
    else if (nCnt == 3)
        str.Format(_T("3rd"));
    else
        str.Format(_T("%dth"), nCnt);

    return str;
}

std::vector<CString> VisionInspectionBgaBallPixelQuality::ExportRecipeToText() //kircheis_TxtRecipe
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
