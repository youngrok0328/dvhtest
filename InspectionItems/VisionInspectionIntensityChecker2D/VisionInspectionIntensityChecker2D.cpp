//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionIntensityChecker2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionIntensityChecker2D.h"
#include "VisionInspectionIntensityChecker2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h" //
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Conversion.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionInspectionIntensityChecker2D::VisionInspectionIntensityChecker2D(
    VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionInspection(
          _VISION_INSP_GUID_INTENSITYCHECKER_2D, _VISION_INSP_NAME_INTENSITYCHECKER_2D, visionUnit, packageSpec)
    , m_VisionPara(new VisionInspectionIntensityChecker2DPara(*this))
    , m_sEdgeAlignResult(new VisionAlignResult)
    , m_bUseBypassMode(false) // Bypass 구현
{
    m_pVisionInspDlg = NULL;

    m_defaultFixedInspectionSpecs.emplace_back(_T("{F5D07F97-3296-493B-A039-CF7209EDAAD1}"),
        g_szIntensityCheckerInspection2DName[INTENSITYCHECKER_INSPECTION_ROI0_1], _T("R1.1I"), _T("L"), _T("GV"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{2D91BD77-D22A-4521-A92E-A9089D55E20F}"),
        g_szIntensityCheckerInspection2DName[INTENSITYCHECKER_INSPECTION_ROI0_2], _T("R1.2I"), _T("L"), _T("GV"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{3CB807F2-B61B-48B2-98C6-A35E5D0D134B}"),
        g_szIntensityCheckerInspection2DName[INTENSITYCHECKER_INSPECTION_ROI0_3], _T("R1.3I"), _T("L"), _T("GV"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{84568396-0B62-472F-AEB1-2F24CD2F0F5F}"),
        g_szIntensityCheckerInspection2DName[INTENSITYCHECKER_INSPECTION_ROI1_1], _T("R2.1I"), _T("L"), _T("GV"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{68120DA7-88B4-4231-9A16-0A067632DED2}"),
        g_szIntensityCheckerInspection2DName[INTENSITYCHECKER_INSPECTION_ROI1_2], _T("R2.2I"), _T("L"), _T("GV"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{6BC4F607-A2A9-4E49-AA39-F57AFFA8BB44}"),
        g_szIntensityCheckerInspection2DName[INTENSITYCHECKER_INSPECTION_ROI1_3], _T("R2.3I"), _T("L"), _T("GV"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{C4F790E9-52A7-46B4-8B74-37353AD09232}"),
        g_szIntensityCheckerInspection2DName[INTENSITYCHECKER_INSPECTION_ROI2_1], _T("R3.1I"), _T("L"), _T("GV"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{92FBBED5-88F7-4965-88DA-D63B91615D1B}"),
        g_szIntensityCheckerInspection2DName[INTENSITYCHECKER_INSPECTION_ROI2_2], _T("R3.2I"), _T("L"), _T("GV"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{A34AFE2A-1AAF-4C5C-AA7D-CAD1DDD5A77D}"),
        g_szIntensityCheckerInspection2DName[INTENSITYCHECKER_INSPECTION_ROI2_3], _T("R3.3I"), _T("L"), _T("GV"),
        HostReportCategory::MEASURE, II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    __super::ResetSpec();

    m_DebugInfoGroup.Add(
        _T("Detect IntensityChecker ROI"), enumDebugInfoType::Float); // 전체 프레임에 해당하는 GV 값 검사

    m_DebugInfoGroup.Add(_T("Intensity Checker Usage"), enumDebugInfoType::Long);
    m_DebugInfoGroup.Add(_T("Intensity AVG"), enumDebugInfoType::Double);
    m_DebugInfoGroup.Add(_T("Intensity Max"), enumDebugInfoType::Long);
    m_DebugInfoGroup.Add(_T("Intensity Min"), enumDebugInfoType::Long);
}

void VisionInspectionIntensityChecker2D::ResetSpecAndPara()
{
    __super::ResetSpec();
}

VisionInspectionIntensityChecker2D::~VisionInspectionIntensityChecker2D(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    delete m_sEdgeAlignResult;
    delete m_VisionPara;
}

void VisionInspectionIntensityChecker2D::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionInspectionIntensityChecker2D::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
{
    // 이 코드가 없으면 Dialog 가 보이지 않음
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    if (m_visionUnit.IsTheUsingVisionProcessingByGuid(_VISION_INSP_GUID_PAD_ALIGN_2D))
        m_visionUnit.RunInspection(
            m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_PAD_ALIGN_2D), false, GetCurVisionModule_Status());
    else if (m_visionUnit.IsTheUsingVisionProcessingByGuid(_VISION_INSP_GUID_ALIGN_2D))
        m_visionUnit.RunInspection(
            m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_ALIGN_2D), false, GetCurVisionModule_Status());
    else if (m_visionUnit.IsTheUsingVisionProcessingByGuid(m_visionUnit.GetVisionAlignProcessingModuleGUID()))
        m_visionUnit.RunInspection(m_visionUnit.GetVisionAlignProcessing(), false, GetCurVisionModule_Status());
    else
    {
        CString strError;
        strError.Format(_T("This module need execute the [%s] in the pre-process."),
            m_visionUnit.GetVisionProcessingModuleNameByGuid(m_visionUnit.GetVisionAlignProcessingModuleGUID()));
        SimpleMessage(strError);

        strError.Empty();
    }

    CWnd* parent = CWnd::FromHandle(procDlgInfo.m_hwndParent);

    m_pVisionInspDlg = new CDlgVisionInspectionIntensityChecker2D(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionInspectionIntensityChecker2D::IDD, parent);

    m_rtPaneRect = Ipvm::Conversion::ToRect32s(m_pVisionInspDlg->m_pVisionInsp->GetBodyRect());

    m_pVisionInspDlg->ShowWindow(SW_SHOW);
    m_pVisionInspDlg->SetInitParaWindow();

    return 0;
}

BOOL VisionInspectionIntensityChecker2D::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    //// Link Parameter
    if (!m_VisionPara->LinkDataBase(bSave, db[_T("{D7F81183-F96C-4310-BE8A-2A42B42F4974}")]))
        return FALSE;

    return TRUE;
}

void VisionInspectionIntensityChecker2D::GetOverlayResult(
    VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (overlayResult == nullptr)
    {
        return;
    }

    if (detailSetupMode)
        overlayResult->OverlayReset();

    for (long nInsp = INTENSITYCHECKER_INSPECTION_2D_START; nInsp < INTENSITYCHECKER_INSPECTION_2D_END; nInsp++)
    {
        if (m_fixedInspectionSpecs[nInsp].m_use)
        {
            VisionInspectionResult* pResult
                = m_resultGroup.GetResultByName(g_szIntensityCheckerInspection2DName[nInsp]);
            if (pResult == NULL)
                continue;

            if (m_nOverlayMode == OverlayType_ShowAll || m_nOverlayMode == OverlayType_Reject)
            {
                //if(pResult->m_totalResult == REJECT)
                {
                    std::vector<Ipvm::Rect32s> vrtPass;
                    std::vector<Ipvm::Rect32s> vrtReject;
                    long nObjNum = (long)pResult->m_objectResults.size();
                    if (nObjNum != pResult->m_objectRects.size())
                        continue;

                    for (long nID = 0; nID < nObjNum; nID++)
                    {
                        auto rtObj = pResult->m_objectRects[nID];
                        if (pResult->m_objectResults[nID] == PASS)
                            vrtPass.push_back(rtObj);
                        else
                            vrtReject.push_back(rtObj);
                    }

                    overlayResult->AddRectangles(vrtReject, RGB(255, 0, 0));
                    if (m_nOverlayMode == OverlayType_ShowAll)
                        overlayResult->AddRectangles(vrtPass, RGB(0, 255, 0));
                }
            }
        }
    }
}

void VisionInspectionIntensityChecker2D::AppendTextResult(CString& textResult)
{
    long nInspNum = (long)m_fixedInspectionSpecs.size();
    long nResultNum = (long)m_resultGroup.m_vecResult.size();
    if (nInspNum != nResultNum)
    {
        ASSERT(!(_T("Check the inspection item's number")));
        return;
    }

    std::vector<VisionInspectionResult*> vecpInspResult(nInspNum);
    std::vector<VisionInspectionSpec*> vecpInspSpec(nInspNum);
    std::vector<BOOL> vecbUseInsp(nInspNum);
    std::vector<CString> vecstrInspShortName(nInspNum);
    long nVisionModuleTotalResult = NOT_MEASURED;

    for (long nID = 0; nID < nInspNum; nID++)
    {
        vecpInspResult[nID] = m_resultGroup.GetResultByName(m_fixedInspectionSpecs[nID].m_specName);
        if (vecpInspResult[nID] == nullptr)
            ASSERT(!(_T("Check the inspection item's number")));
        vecpInspSpec[nID] = GetSpecByName(vecpInspResult[nID]->m_resultName);
        if (vecpInspSpec[nID] == nullptr)
            ASSERT(!(_T("Check the inspection item's number")));

        vecbUseInsp[nID] = -vecpInspSpec[nID]->m_use;
        vecstrInspShortName[nID] = vecpInspSpec[nID]->m_shortName;
        nVisionModuleTotalResult = (long)max(nVisionModuleTotalResult, vecpInspResult[nID]->m_totalResult);
    }

    textResult.AppendFormat(_T("\r\n< %s Result >\r\n"), (LPCTSTR)m_strModuleName);
    textResult.AppendFormat(_T("  [ Inspection Time : %.2f ]\r\n"), m_fCalcTime);
    textResult.AppendFormat(
        _T("    %s Total Result : %s\r\n"), (LPCTSTR)m_strModuleName, (LPCTSTR)Result2String(nVisionModuleTotalResult));

    for (long nID = 0; nID < nInspNum; nID++)
    {
        if (!vecbUseInsp[nID])
            continue;

        auto worstErrorValue = vecpInspResult[nID]->getObjectWorstErrorValue();

        if (worstErrorValue == Ipvm::k_noiseValue32r)
        {
            textResult.AppendFormat(_T("      %s [%s] = %s (Worst : ID [ %s ], Value [ INV ])\r\n"),
                (LPCTSTR)vecpInspResult[nID]->m_resultName, (LPCTSTR)vecstrInspShortName[nID],
                (LPCTSTR)Result2String(vecpInspResult[nID]->m_totalResult),
                (LPCTSTR)vecpInspResult[nID]->getObjectWorstID());
        }
        else
        {
            textResult.AppendFormat(_T("      %s [%s] = %s (Worst : ID [ %s ], Value [ %.2f ])\r\n"),
                (LPCTSTR)vecpInspResult[nID]->m_resultName, (LPCTSTR)vecstrInspShortName[nID],
                (LPCTSTR)Result2String(vecpInspResult[nID]->m_totalResult),
                (LPCTSTR)vecpInspResult[nID]->getObjectWorstID(), worstErrorValue);
        }
    }

    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_2D_MATRIX, _T("2D Matrix Result"), nDataNum);
    if (nDataNum > 0 && pData != nullptr)
    {
        CString strTemp;
        CString* pStr2DID = (CString*)pData;
        for (long nID = 0; nID < nDataNum; nID++)
        {
            textResult.AppendFormat(_T("          %s\r\n"), (LPCTSTR)pStr2DID[nID]);
        }
        strTemp.Empty();
    }

    for (int nIdx = 0; nIdx < vecstrInspShortName.size(); nIdx++)
    {
        vecstrInspShortName[nIdx].Empty();
    }
}

void VisionInspectionIntensityChecker2D::ResetResult()
{
    __super::ResetResult();

    m_pEdgeDetect = getReusableMemory().GetEdgeDetect();

    if (m_pVisionInspDlg)
    {
        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
    }
}

std::vector<CString> VisionInspectionIntensityChecker2D::ExportRecipeToText() //kircheis_TxtRecipe
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