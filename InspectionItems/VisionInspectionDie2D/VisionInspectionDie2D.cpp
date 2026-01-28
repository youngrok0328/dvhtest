//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionDie2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionDie2D.h"
#include "VisionInspectionDie2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
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
VisionInspectionDie2D::VisionInspectionDie2D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionInspection(_VISION_INSP_GUID_DIE_2D, _VISION_INSP_NAME_DIE_2D, visionUnit, packageSpec)
    , m_VisionPara(new VisionInspectionDie2DPara(*this))
    , m_sEdgeAlignResult(new VisionAlignResult)
{
    m_pVisionInspDlg = NULL;

    m_defaultFixedInspectionSpecs.emplace_back(_T("{C51E4924-CD2F-4B72-B299-B08C3B97A101}"),
        g_szDieInspection2DName[DIE_INSPECTION_2D_OFFSET_X], _T("DdX"), _T("D"), _T("um"), HostReportCategory::DIE,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{A86FA8E1-3AA5-42B2-8B16-19A2DC7A46DD}"),
        g_szDieInspection2DName[DIE_INSPECTION_2D_OFFSET_Y], _T("DdY"), _T("D"), _T("um"), HostReportCategory::DIE,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{A241EBFE-84F6-487F-8A4B-4738B1C10C87}"),
        g_szDieInspection2DName[DIE_INSPECTION_2D_OFFSET_THETA], _T("DR"), _T("D"), _T("Deg"), HostReportCategory::DIE,
        II_RESULT_TYPE::DEGREE, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);
    m_defaultFixedInspectionSpecs.emplace_back(_T("{D249601D-E815-4949-93BC-E969973A034C}"),
        g_szDieInspection2DName[DIE_INSPECTION_2D_SIZE_X], _T("DSX"), _T("D"), _T("um"), HostReportCategory::DIE,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE); //kircheis_LidInspOpt
    m_defaultFixedInspectionSpecs.emplace_back(_T("{BA0ECD61-3814-4515-8734-03608487F77F}"),
        g_szDieInspection2DName[DIE_INSPECTION_2D_SIZE_Y], _T("DSY"), _T("D"), _T("um"), HostReportCategory::DIE,
        II_RESULT_TYPE::MEASURED, FALSE, 0.f, 0.f, 0.f, 0.f, FALSE, FALSE, FALSE);

    __super::ResetSpec();

    m_DebugInfoGroup.Add(_T("Die Spec"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Die Edge Search ROI"), enumDebugInfoType::Rect);
    m_DebugInfoGroup.Add(_T("Die Edge Point Left"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Die Edge Point Top"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Die Edge Point Right"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Die Edge Point Bottom"), enumDebugInfoType::Point_32f_C2);
    m_DebugInfoGroup.Add(_T("Detect Die Rect"), enumDebugInfoType::Rect_32f);
    m_DebugInfoGroup.Add(_T("Detect Die ROI"), enumDebugInfoType::PI_Rect);
}

void VisionInspectionDie2D::ResetSpecAndPara()
{
    __super::ResetSpec();
}

VisionInspectionDie2D::~VisionInspectionDie2D(void)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CloseDlg();

    delete m_sEdgeAlignResult;
    delete m_VisionPara;
}

void VisionInspectionDie2D::CloseDlg()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pVisionInspDlg->GetSafeHwnd())
    {
        m_pVisionInspDlg->DestroyWindow();
        delete m_pVisionInspDlg;
        m_pVisionInspDlg = nullptr;
    }
}

long VisionInspectionDie2D::ShowDlg(const ProcessingDlgInfo& procDlgInfo)
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

    m_pVisionInspDlg = new CDlgVisionInspectionDie2D(procDlgInfo, this, parent);
    m_pVisionInspDlg->Create(CDlgVisionInspectionDie2D::IDD, parent);

    Ipvm::Rect32r frtBody = m_pVisionInspDlg->m_pVisionInsp->GetBodyRect();

    m_pVisionInspDlg->ShowWindow(SW_SHOW);
    m_pVisionInspDlg->SetInitParaWindow();

    return 0;
}

BOOL VisionInspectionDie2D::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
        return FALSE;

    //// Link Parameter
    if (!m_VisionPara->LinkDataBase(bSave, db[_T("{DE4EE7C3-F949-49EE-B93B-828F6DA3E763}")]))
        return FALSE;

    return TRUE;
}

void VisionInspectionDie2D::GetOverlayResult(VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode)
{
    if (overlayResult == nullptr)
    {
        return;
    }

    if (detailSetupMode)
        overlayResult->OverlayReset();

    BOOL bOnlyUseAlign = TRUE;

    for (long nInsp = DIE_INSPECTION_2D_START; nInsp < DIE_INSPECTION_2D_END; nInsp++)
    {
        if (m_fixedInspectionSpecs[nInsp].m_use)
        {
            VisionInspectionResult* pResult = m_resultGroup.GetResultByName(g_szDieInspection2DName[nInsp]);
            if (pResult == NULL)
                continue;

            bOnlyUseAlign = FALSE;

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

    if (bOnlyUseAlign)
    {
        std::vector<Ipvm::Rect32s> vrtDie;
        for (auto dieAlignResult : m_vecsDieAlignResult)
        {
            vrtDie.push_back(dieAlignResult.getBodyRect32s());
        }
        overlayResult->AddRectangles(vrtDie, RGB(0, 255, 0));
    }
}

void VisionInspectionDie2D::AppendTextResult(CString& textResult)
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

void VisionInspectionDie2D::ResetResult()
{
    __super::ResetResult();

    m_pEdgeDetect = getReusableMemory().GetEdgeDetect();

    if (m_pVisionInspDlg)
    {
        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
    }
}

std::vector<CString> VisionInspectionDie2D::ExportRecipeToText() //kircheis_TxtRecipe
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